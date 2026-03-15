#include "stdafx.h"

#include "Common/App_Defines.h"
#include "Common/Network/GameNetworkManager.h"
#include "Input.h"
#include "Minecraft.h"
#include "MinecraftServer.h"
#include "..\Access\Access.h"
#include "..\Common\StringUtils.h"
#include "..\ServerLogger.h"
#include "..\ServerLogManager.h"
#include "..\ServerProperties.h"
#include "..\ServerShutdown.h"
#include "..\WorldManager.h"
#include "..\Console\ServerCli.h"
#include "Tesselator.h"
#include "Windows64/4JLibs/inc/4J_Render.h"
#include "Windows64/GameConfig/Minecraft.spa.h"
#include "Windows64/KeyboardMouseInput.h"
#include "Windows64/Network/WinsockNetLayer.h"
#include "Windows64/Windows64_UIController.h"
#include "Common/UI/UI.h"

#include "../../Minecraft.World/AABB.h"
#include "../../Minecraft.World/Vec3.h"
#include "../../Minecraft.World/IntCache.h"
#include "../../Minecraft.World/ChunkSource.h"
#include "../../Minecraft.World/TilePos.h"
#include "../../Minecraft.World/compression.h"
#include "../../Minecraft.World/OldChunkStorage.h"
#include "../../Minecraft.World/net.minecraft.world.level.tile.h"
#include "../../Minecraft.World/Random.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <atomic>

extern ATOM MyRegisterClass(HINSTANCE hInstance);
extern BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);
extern HRESULT InitDevice();
extern void CleanupDevice();
extern void DefineActions(void);

extern HWND g_hWnd;
extern int g_iScreenWidth;
extern int g_iScreenHeight;
extern char g_Win64Username[17];
extern wchar_t g_Win64UsernameW[17];
extern ID3D11Device* g_pd3dDevice;
extern ID3D11DeviceContext* g_pImmediateContext;
extern IDXGISwapChain* g_pSwapChain;
extern ID3D11RenderTargetView* g_pRenderTargetView;
extern ID3D11DepthStencilView* g_pDepthStencilView;
extern DWORD dwProfileSettingsA[];

static const int kProfileValueCount = 5;
static const int kProfileSettingCount = 4;

struct DedicatedServerConfig
{
	int port;
	char bindIP[256];
	char name[17];
	int maxPlayers;
	int worldSize;
	int worldSizeChunks;
	int worldHellScale;
	__int64 seed;
	ServerRuntime::EServerLogLevel logLevel;
	bool hasSeed;
	bool showHelp;
};

static std::atomic<bool> g_shutdownRequested(false);
static const DWORD kDefaultAutosaveIntervalMs = 60 * 1000;
static const int kServerActionPad = 0;

static bool IsShutdownRequested()
{
	return g_shutdownRequested.load();
}

namespace ServerRuntime
{
	void RequestDedicatedServerShutdown()
	{
		g_shutdownRequested.store(true);
	}
}

/**
 * Calls Access::Shutdown automatically once dedicated access control was initialized successfully
 * アクセス制御初期化後のShutdownを自動化する
 */
class AccessShutdownGuard
{
public:
	AccessShutdownGuard()
		: m_active(false)
	{
	}

	void Activate()
	{
		m_active = true;
	}

	~AccessShutdownGuard()
	{
		if (m_active)
		{
			ServerRuntime::Access::Shutdown();
		}
	}

private:
	bool m_active;
};
static BOOL WINAPI ConsoleCtrlHandlerProc(DWORD ctrlType)
{
	switch (ctrlType)
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		ServerRuntime::RequestDedicatedServerShutdown();
		return TRUE;
	default:
		return FALSE;
	}
}

/**
 * **Wait For Server Stopped Signal**
 *
 * Thread entry used during shutdown to wait until the network layer reports server stop completion
 * 停止通知待機用の終了スレッド処理
 */
static int WaitForServerStoppedThreadProc(void *)
{
	if (g_NetworkManager.ServerStoppedValid())
	{
		g_NetworkManager.ServerStoppedWait();
	}
	return 0;
}

static void PrintUsage()
{
	ServerRuntime::LogInfo("usage", "Minecraft.Server.exe [options]");
	ServerRuntime::LogInfo("usage", "  -port <1-65535>       Listen TCP port (default: server.properties:server-port)");
	ServerRuntime::LogInfo("usage", "  -ip <addr>            Bind address (default: server.properties:server-ip)");
	ServerRuntime::LogInfo("usage", "  -bind <addr>          Alias of -ip");
	ServerRuntime::LogInfo("usage", "  -name <name>          Host display name (max 16 chars, default: server.properties:server-name)");
	ServerRuntime::LogInfo("usage", "  -maxplayers <1-8>     Public slots (default: server.properties:max-players)");
	ServerRuntime::LogInfo("usage", "  -seed <int64>         World seed (overrides server.properties:level-seed)");
	ServerRuntime::LogInfo("usage", "  -loglevel <level>     debug|info|warn|error (default: server.properties:log-level)");
	ServerRuntime::LogInfo("usage", "  -help                 Show this help");
}

using ServerRuntime::LoadServerPropertiesConfig;
using ServerRuntime::LogError;
using ServerRuntime::LogErrorf;
using ServerRuntime::LogInfof;
using ServerRuntime::LogDebugf;
using ServerRuntime::LogStartupStep;
using ServerRuntime::LogWarn;
using ServerRuntime::LogWorldIO;
using ServerRuntime::SaveServerPropertiesConfig;
using ServerRuntime::SetServerLogLevel;
using ServerRuntime::ServerPropertiesConfig;
using ServerRuntime::TryParseServerLogLevel;
using ServerRuntime::StringUtils::WideToUtf8;
using ServerRuntime::BootstrapWorldForServer;
using ServerRuntime::eWorldBootstrap_CreatedNew;
using ServerRuntime::eWorldBootstrap_Failed;
using ServerRuntime::eWorldBootstrap_Loaded;
using ServerRuntime::WaitForWorldActionIdle;
using ServerRuntime::WorldBootstrapResult;

static bool ParseIntArg(const char *value, int *outValue)
{
	if (value == NULL || *value == 0)
		return false;

	char *end = NULL;
	long parsed = strtol(value, &end, 10);
	if (end == value || *end != 0)
		return false;

	*outValue = (int)parsed;
	return true;
}

static bool ParseInt64Arg(const char *value, __int64 *outValue)
{
	if (value == NULL || *value == 0)
		return false;

	char *end = NULL;
	__int64 parsed = _strtoi64(value, &end, 10);
	if (end == value || *end != 0)
		return false;

	*outValue = parsed;
	return true;
}

static bool ParseCommandLine(int argc, char **argv, DedicatedServerConfig *config)
{
	for (int i = 1; i < argc; ++i)
	{
		const char *arg = argv[i];
		if (_stricmp(arg, "-help") == 0 || _stricmp(arg, "--help") == 0 || _stricmp(arg, "-h") == 0)
		{
			config->showHelp = true;
			return true;
		}
		else if ((_stricmp(arg, "-port") == 0) && (i + 1 < argc))
		{
			int port = 0;
			if (!ParseIntArg(argv[++i], &port) || port <= 0 || port > 65535)
			{
				LogError("startup", "Invalid -port value.");
				return false;
			}
			config->port = port;
		}
		else if ((_stricmp(arg, "-ip") == 0 || _stricmp(arg, "-bind") == 0) && (i + 1 < argc))
		{
			strncpy_s(config->bindIP, sizeof(config->bindIP), argv[++i], _TRUNCATE);
		}
		else if ((_stricmp(arg, "-name") == 0) && (i + 1 < argc))
		{
			strncpy_s(config->name, sizeof(config->name), argv[++i], _TRUNCATE);
		}
		else if ((_stricmp(arg, "-maxplayers") == 0) && (i + 1 < argc))
		{
			int maxPlayers = 0;
			if (!ParseIntArg(argv[++i], &maxPlayers) || maxPlayers <= 0 || maxPlayers > MINECRAFT_NET_MAX_PLAYERS)
			{
				LogError("startup", "Invalid -maxplayers value.");
				return false;
			}
			config->maxPlayers = maxPlayers;
		}
		else if ((_stricmp(arg, "-seed") == 0) && (i + 1 < argc))
		{
			if (!ParseInt64Arg(argv[++i], &config->seed))
			{
				LogError("startup", "Invalid -seed value.");
				return false;
			}
			config->hasSeed = true;
		}
		else if ((_stricmp(arg, "-loglevel") == 0) && (i + 1 < argc))
		{
			if (!TryParseServerLogLevel(argv[++i], &config->logLevel))
			{
				LogError("startup", "Invalid -loglevel value. Use debug/info/warn/error.");
				return false;
			}
		}
		else
		{
			LogErrorf("startup", "Unknown or incomplete argument: %s", arg);
			return false;
		}
	}

	return true;
}

static void SetExeWorkingDirectory()
{
	char exePath[MAX_PATH] = {};
	GetModuleFileNameA(NULL, exePath, MAX_PATH);
	char *slash = strrchr(exePath, '\\');
	if (slash != NULL)
	{
		*(slash + 1) = 0;
		SetCurrentDirectoryA(exePath);
	}
}

static void ApplyServerPropertiesToDedicatedConfig(const ServerPropertiesConfig &serverProperties, DedicatedServerConfig *config)
{
	if (config == NULL)
	{
		return;
	}

	config->port = serverProperties.serverPort;
	strncpy_s(
		config->bindIP,
		sizeof(config->bindIP),
		serverProperties.serverIp.empty() ? "0.0.0.0" : serverProperties.serverIp.c_str(),
		_TRUNCATE);
	strncpy_s(
		config->name,
		sizeof(config->name),
		serverProperties.serverName.empty() ? "DedicatedServer" : serverProperties.serverName.c_str(),
		_TRUNCATE);
	config->maxPlayers = serverProperties.maxPlayers;
	config->worldSize = serverProperties.worldSize;
	config->worldSizeChunks = serverProperties.worldSizeChunks;
	config->worldHellScale = serverProperties.worldHellScale;
	config->logLevel = serverProperties.logLevel;
	config->hasSeed = serverProperties.hasSeed;
	config->seed = serverProperties.seed;
}

/**
 * **Tick Core Async Subsystems**
 *
 * Advances core subsystems for one frame to keep async processing alive
 * Call continuously even inside wait loops to avoid stalling storage/profile/network work
 * 非同期進行維持のためのティック処理
 */
static void TickCoreSystems()
{
	g_NetworkManager.DoWork();
	ProfileManager.Tick();
	StorageManager.Tick();
}

/**
 * **Handle Queued XUI Server Action Once**
 *
 * Processes queued XUI/server action once
 * XUIアクションの単発処理
 */
static void HandleXuiActions()
{
	app.HandleXuiActions();
}

/**
 * Dedicated Server Entory Point
 *
 * 主な責務:
 * - プロセス/描画/ネットワークの初期化
 * - `WorldManager` によるワールドロードまたは新規作成
 * - メインループと定期オートセーブ実行
 * - 終了時の最終保存と各サブシステムの安全停止
 */
int main(int argc, char **argv)
{
	DedicatedServerConfig config;
	config.port = WIN64_NET_DEFAULT_PORT;
	strncpy_s(config.bindIP, sizeof(config.bindIP), "0.0.0.0", _TRUNCATE);
	strncpy_s(config.name, sizeof(config.name), "DedicatedServer", _TRUNCATE);
	config.maxPlayers = MINECRAFT_NET_MAX_PLAYERS;
	config.worldSize = e_worldSize_Classic;
	config.worldSizeChunks = LEVEL_WIDTH_CLASSIC;
	config.worldHellScale = HELL_LEVEL_SCALE_CLASSIC;
	config.seed = 0;
	config.logLevel = ServerRuntime::eServerLogLevel_Info;
	config.hasSeed = false;
	config.showHelp = false;

	SetConsoleCtrlHandler(ConsoleCtrlHandlerProc, TRUE);
	SetExeWorkingDirectory();

	// Load base settings from server.properties, then override with CLI values when provided
	ServerPropertiesConfig serverProperties = LoadServerPropertiesConfig();
	ApplyServerPropertiesToDedicatedConfig(serverProperties, &config);

	if (!ParseCommandLine(argc, argv, &config))
	{
		PrintUsage();
		return 1;
	}
	if (config.showHelp)
	{
		PrintUsage();
		return 0;
	}

	SetServerLogLevel(config.logLevel);
	LogStartupStep("initializing process state");
	AccessShutdownGuard accessShutdownGuard;

	g_iScreenWidth = 1280;
	g_iScreenHeight = 720;

	strncpy_s(g_Win64Username, sizeof(g_Win64Username), config.name, _TRUNCATE);
	MultiByteToWideChar(CP_ACP, 0, g_Win64Username, -1, g_Win64UsernameW, 17);

	g_Win64MultiplayerHost = true;
	g_Win64MultiplayerJoin = false;
	g_Win64MultiplayerPort = config.port;
	strncpy_s(g_Win64MultiplayerIP, sizeof(g_Win64MultiplayerIP), config.bindIP, _TRUNCATE);
	g_Win64DedicatedServer = true;
	g_Win64DedicatedServerPort = config.port;
	strncpy_s(g_Win64DedicatedServerBindIP, sizeof(g_Win64DedicatedServerBindIP), config.bindIP, _TRUNCATE);
	g_Win64DedicatedServerLanAdvertise = serverProperties.lanAdvertise;
	LogStartupStep("initializing server log manager");
	ServerRuntime::ServerLogManager::Initialize();
	LogStartupStep("initializing dedicated access control");
	if (!ServerRuntime::Access::Initialize(".", serverProperties.whiteListEnabled))
	{
		LogError("startup", "Failed to initialize dedicated server access control.");
		return 2;
	}
	accessShutdownGuard.Activate();
	LogInfof("startup", "LAN advertise: %s", serverProperties.lanAdvertise ? "enabled" : "disabled");
	LogInfof("startup", "Whitelist: %s", serverProperties.whiteListEnabled ? "enabled" : "disabled");
	LogInfof("startup", "Spawn protection radius: %d", serverProperties.spawnProtectionRadius);
#ifdef _LARGE_WORLDS
	LogInfof(
		"startup",
		"World size preset: %d (xz=%d, hell-scale=%d)",
		config.worldSize,
		config.worldSizeChunks,
		config.worldHellScale);
#endif

	LogStartupStep("registering hidden window class");
	HINSTANCE hInstance = GetModuleHandle(NULL);
	MyRegisterClass(hInstance);

	LogStartupStep("creating hidden window");
	if (!InitInstance(hInstance, SW_HIDE))
	{
		LogError("startup", "Failed to create window instance.");
		
		return 2;
	}
	ShowWindow(g_hWnd, SW_HIDE);

	LogStartupStep("initializing graphics device wrappers");
	if (FAILED(InitDevice()))
	{
		LogError("startup", "Failed to initialize D3D device.");
		CleanupDevice();
		
		return 2;
	}

	LogStartupStep("loading media/string tables");
	app.loadMediaArchive();
	RenderManager.Initialise(g_pd3dDevice, g_pSwapChain);
	app.loadStringTable();
	ui.init(g_pd3dDevice, g_pImmediateContext, g_pRenderTargetView, g_pDepthStencilView, g_iScreenWidth, g_iScreenHeight);

	InputManager.Initialise(1, 3, MINECRAFT_ACTION_MAX, ACTION_MAX_MENU);
	g_KBMInput.Init();
	DefineActions();
	InputManager.SetJoypadMapVal(0, 0);
	InputManager.SetKeyRepeatRate(0.3f, 0.2f);

	ProfileManager.Initialise(
		TITLEID_MINECRAFT,
		app.m_dwOfferID,
		PROFILE_VERSION_10,
		kProfileValueCount,
		kProfileSettingCount,
		dwProfileSettingsA,
		app.GAME_DEFINED_PROFILE_DATA_BYTES * XUSER_MAX_COUNT,
		&app.uiGameDefinedDataChangedBitmask);
	ProfileManager.SetDefaultOptionsCallback(&CConsoleMinecraftApp::DefaultOptionsCallback, (LPVOID)&app);
	ProfileManager.SetDebugFullOverride(true);

	LogStartupStep("initializing network manager");
	g_NetworkManager.Initialise();

	for (int i = 0; i < MINECRAFT_NET_MAX_PLAYERS; ++i)
	{
		IQNet::m_player[i].m_smallId = (BYTE)i;
		IQNet::m_player[i].m_isRemote = false;
		IQNet::m_player[i].m_isHostPlayer = (i == 0);
		swprintf_s(IQNet::m_player[i].m_gamertag, 32, L"Player%d", i);
	}
	wcscpy_s(IQNet::m_player[0].m_gamertag, 32, g_Win64UsernameW);
	WinsockNetLayer::Initialize();

	Tesselator::CreateNewThreadStorage(1024 * 1024);
	AABB::CreateNewThreadStorage();
	Vec3::CreateNewThreadStorage();
	IntCache::CreateNewThreadStorage();
	Compression::CreateNewThreadStorage();
	OldChunkStorage::CreateNewThreadStorage();
	Level::enableLightingCache();
	Tile::CreateNewThreadStorage();

	LogStartupStep("creating Minecraft singleton");
	Minecraft::main();
	Minecraft *minecraft = Minecraft::GetInstance();
	if (minecraft == NULL)
	{
		LogError("startup", "Minecraft initialization failed.");
		CleanupDevice();
		
		return 3;
	}

	app.InitGameSettings();

	MinecraftServer::resetFlags();
	app.SetTutorialMode(false);
	app.SetCorruptSaveDeleted(false);
	app.SetGameHostOption(eGameHostOption_Difficulty, serverProperties.difficulty);
	app.SetGameHostOption(eGameHostOption_FriendsOfFriends, serverProperties.friendsOfFriends ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_Gamertags, serverProperties.gamertags ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_BedrockFog, serverProperties.bedrockFog ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_GameType, serverProperties.gameMode);
	app.SetGameHostOption(eGameHostOption_LevelType, serverProperties.levelTypeFlat ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_Structures, serverProperties.generateStructures ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_BonusChest, serverProperties.bonusChest ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_PvP, serverProperties.pvp ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_TrustPlayers, serverProperties.trustPlayers ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_FireSpreads, serverProperties.fireSpreads ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_TNT, serverProperties.tnt ? 1 : 0);
	app.SetGameHostOption(
		eGameHostOption_CheatsEnabled,
		(serverProperties.hostCanFly || serverProperties.hostCanChangeHunger || serverProperties.hostCanBeInvisible) ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_HostCanFly, serverProperties.hostCanFly ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_HostCanChangeHunger, serverProperties.hostCanChangeHunger ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_HostCanBeInvisible, serverProperties.hostCanBeInvisible ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_DisableSaving, serverProperties.disableSaving ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_MobGriefing, serverProperties.mobGriefing ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_KeepInventory, serverProperties.keepInventory ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_DoMobSpawning, serverProperties.doMobSpawning ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_DoMobLoot, serverProperties.doMobLoot ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_DoTileDrops, serverProperties.doTileDrops ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_NaturalRegeneration, serverProperties.naturalRegeneration ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_DoDaylightCycle, serverProperties.doDaylightCycle ? 1 : 0);
#ifdef _LARGE_WORLDS
	app.SetGameHostOption(eGameHostOption_WorldSize, serverProperties.worldSize);
	// Apply desired target size for loading existing worlds.
	// Expansion happens only when target size is larger than current level size.
	app.SetGameNewWorldSize(serverProperties.worldSizeChunks, true);
	app.SetGameNewHellScale(serverProperties.worldHellScale);
#endif

	StorageManager.SetSaveDisabled(serverProperties.disableSaving);
	// Read world name and fixed save-id from server.properties
	// Delegate load-vs-create decision to WorldManager
	std::wstring targetWorldName = serverProperties.worldName;
	if (targetWorldName.empty())
	{
		targetWorldName = L"world"; // Default world name
	}
	WorldBootstrapResult worldBootstrap = BootstrapWorldForServer(serverProperties, kServerActionPad, &TickCoreSystems);
	if (worldBootstrap.status == eWorldBootstrap_Loaded)
	{
		const std::string &loadedSaveFilename = worldBootstrap.resolvedSaveId;
		if (!loadedSaveFilename.empty() && _stricmp(loadedSaveFilename.c_str(), serverProperties.worldSaveId.c_str()) != 0)
		{
			// Persist the actually loaded save-id back to config
			// Keep lookup keys aligned for next startup
			LogWorldIO("updating level-id to loaded save filename");
			serverProperties.worldSaveId = loadedSaveFilename;
			if (!SaveServerPropertiesConfig(serverProperties))
			{
				LogWorldIO("failed to persist updated level-id");
			}
		}
	}
	else if (worldBootstrap.status == eWorldBootstrap_Failed)
	{
		LogErrorf("world-io", "Failed to load configured world \"%s\".", WideToUtf8(targetWorldName).c_str());
		WinsockNetLayer::Shutdown();
		g_NetworkManager.Terminate();
		CleanupDevice();
		
		return 4;
	}

	NetworkGameInitData *param = new NetworkGameInitData();
	if (config.hasSeed)
	{
		param->seed = config.seed;
	}
	else
	{
		param->seed = (new Random())->nextLong();
	}
#ifdef _LARGE_WORLDS
	param->xzSize = (unsigned int)config.worldSizeChunks;
	param->hellScale = (unsigned char)config.worldHellScale;
#endif
	param->saveData = worldBootstrap.saveData;
	param->settings = app.GetGameHostOption(eGameHostOption_All);
	param->dedicatedNoLocalHostPlayer = true;

	LogStartupStep("starting hosted network game thread");
	g_NetworkManager.HostGame(0, true, false, (unsigned char)config.maxPlayers, 0);
	g_NetworkManager.FakeLocalPlayerJoined();

	C4JThread *startThread = new C4JThread(&CGameNetworkManager::RunNetworkGameThreadProc, (LPVOID)param, "RunNetworkGame");
	startThread->Run();

	while (startThread->isRunning() && !IsShutdownRequested())
	{
		TickCoreSystems();
		Sleep(10);
	}

	startThread->WaitForCompletion(INFINITE);
	int startupResult = startThread->GetExitCode();
	delete startThread;

	if (startupResult != 0)
	{
		LogErrorf("startup", "Failed to start dedicated server (code %d).", startupResult);
		WinsockNetLayer::Shutdown();
		g_NetworkManager.Terminate();
		CleanupDevice();
		
		return 4;
	}

	LogStartupStep("server startup complete");
	LogInfof("startup", "Dedicated server listening on %s:%d", g_Win64MultiplayerIP, g_Win64MultiplayerPort);
	if (worldBootstrap.status == eWorldBootstrap_CreatedNew && !IsShutdownRequested() && !app.m_bShutdown)
	{
		// Windows64 suppresses saveToDisc right after new world creation
		// Dedicated Server explicitly runs the initial save here
		LogWorldIO("requesting initial save for newly created world");
		WaitForWorldActionIdle(kServerActionPad, 5000, &TickCoreSystems, &HandleXuiActions);
		app.SetXuiServerAction(kServerActionPad, eXuiServerAction_AutoSaveGame);
		if (!WaitForWorldActionIdle(kServerActionPad, 30000, &TickCoreSystems, &HandleXuiActions))
		{
			LogWorldIO("initial save timed out");
			LogWarn("world-io", "Timed out waiting for initial save action to finish.");
		}
		else
		{
			LogWorldIO("initial save completed");
		}
	}
	DWORD autosaveIntervalMs = kDefaultAutosaveIntervalMs;
	if (serverProperties.autosaveIntervalSeconds > 0)
	{
		autosaveIntervalMs = (DWORD)(serverProperties.autosaveIntervalSeconds * 1000);
	}
	DWORD nextAutosaveTick = GetTickCount() + autosaveIntervalMs;
	bool autosaveRequested = false;
	ServerRuntime::ServerCli serverCli;
	serverCli.Start();

	while (!IsShutdownRequested() && !app.m_bShutdown)
	{
		TickCoreSystems();
		HandleXuiActions();
		serverCli.Poll();

		if (IsShutdownRequested() || app.m_bShutdown)
		{
			break;
		}

		if (autosaveRequested && app.GetXuiServerAction(kServerActionPad) == eXuiServerAction_Idle)
		{
			LogWorldIO("autosave completed");
			autosaveRequested = false;
		}

		if (MinecraftServer::serverHalted())
		{
			break;
		}

		DWORD now = GetTickCount();
		if ((LONG)(now - nextAutosaveTick) >= 0)
		{
			if (app.GetXuiServerAction(kServerActionPad) == eXuiServerAction_Idle)
			{
				LogWorldIO("requesting autosave");
				app.SetXuiServerAction(kServerActionPad, eXuiServerAction_AutoSaveGame);
				autosaveRequested = true;
			}
			nextAutosaveTick = now + autosaveIntervalMs;
		}

		Sleep(10);
	}
	serverCli.Stop();
	app.m_bShutdown = true;

	LogInfof("shutdown", "Dedicated server stopped");
	MinecraftServer *server = MinecraftServer::getInstance();
	if (server != NULL)
	{
		server->setSaveOnExit(true);
	}
	if (server != NULL)
	{
		LogWorldIO("requesting save before shutdown");
		LogWorldIO("using saveOnExit for shutdown");
	}

	MinecraftServer::HaltServer();

	if (g_NetworkManager.ServerStoppedValid())
	{
		C4JThread waitThread(&WaitForServerStoppedThreadProc, NULL, "WaitServerStopped");
		waitThread.Run();
		waitThread.WaitForCompletion(INFINITE);
	}

	LogInfof("shutdown", "Cleaning up and exiting.");
	WinsockNetLayer::Shutdown();
	LogDebugf("shutdown", "Network layer shutdown complete.");
	g_NetworkManager.Terminate();
	LogDebugf("shutdown", "Network manager terminated.");
	ServerRuntime::ServerLogManager::Shutdown();
	CleanupDevice();
	

	return 0;
}

