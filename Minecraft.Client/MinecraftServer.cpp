#include "stdafx.h"
//#include "Minecraft.h"

#include <ctime>

#include "ConsoleInput.h"
#include "DerivedServerLevel.h"
#include "DispenserBootstrap.h"
#include "EntityTracker.h"
#include "MinecraftServer.h"
#include "Options.h"
#include "PlayerList.h"
#include "ServerChunkCache.h"
#include "ServerConnection.h"
#include "ServerLevel.h"
#include "ServerLevelListener.h"
#include "Settings.h"
#include "../Minecraft.World/Command.h"
#include "../Minecraft.World/AABB.h"
#include "../Minecraft.World/Vec3.h"
#include "../Minecraft.World/net.minecraft.network.h"
#include "../Minecraft.World/net.minecraft.world.level.dimension.h"
#include "../Minecraft.World/net.minecraft.world.level.storage.h"
#include "../Minecraft.World/net.minecraft.world.h"
#include "../Minecraft.World/net.minecraft.world.level.h"
#include "../Minecraft.World/net.minecraft.world.level.tile.h"
#include "../Minecraft.World/Pos.h"
#include "../Minecraft.World/System.h"
#include "../Minecraft.World/StringHelpers.h"
#include "../Minecraft.World/net.minecraft.world.entity.item.h"
#include "../Minecraft.World/net.minecraft.world.item.h"
#include "../Minecraft.World/net.minecraft.world.item.enchantment.h"
#include "../Minecraft.World/net.minecraft.world.damagesource.h"
#ifdef _WINDOWS64
#include "Windows64/Network/WinsockNetLayer.h"
#endif
#include <sstream>
#ifdef SPLIT_SAVES
#include "../Minecraft.World/ConsoleSaveFileSplit.h"
#endif
#include "../Minecraft.World/ConsoleSaveFileOriginal.h"
#include "../Minecraft.World/Socket.h"
#include "../Minecraft.World/net.minecraft.world.entity.h"
#include "ProgressRenderer.h"
#include "ServerPlayer.h"
#include "GameRenderer.h"
#include "../Minecraft.World/ThreadName.h"
#include "../Minecraft.World/IntCache.h"
#include "../Minecraft.World/CompressedTileStorage.h"
#include "../Minecraft.World/SparseLightStorage.h"
#include "../Minecraft.World/SparseDataStorage.h"
#include "../Minecraft.World/compression.h"
#ifdef _XBOX
#include "Common/XUI/XUI_DebugSetCamera.h"
#endif
#include "PS3/PS3Extras/ShutdownManager.h"
#include "ServerCommandDispatcher.h"
#include "../Minecraft.World/BiomeSource.h"
#include "PlayerChunkMap.h"
#include "Common/Telemetry/TelemetryManager.h"
#include "PlayerConnection.h"
#ifdef _XBOX_ONE
#include "Durango/Network/NetworkPlayerDurango.h"
#endif

#define DEBUG_SERVER_DONT_SPAWN_MOBS 0

//4J Added
MinecraftServer *MinecraftServer::server = NULL;
bool MinecraftServer::setTimeAtEndOfTick = false;
int64_t MinecraftServer::setTime = 0;
bool MinecraftServer::setTimeOfDayAtEndOfTick = false;
int64_t MinecraftServer::setTimeOfDay = 0;
bool	MinecraftServer::m_bPrimaryPlayerSignedOut=false;
bool	MinecraftServer::s_bServerHalted=false;
bool	MinecraftServer::s_bSaveOnExitAnswered=false;
#ifdef _ACK_CHUNK_SEND_THROTTLING
bool MinecraftServer::s_hasSentEnoughPackets = false;
int64_t MinecraftServer::s_tickStartTime = 0;
vector<INetworkPlayer *> MinecraftServer::s_sentTo;
#else
int MinecraftServer::s_slowQueuePlayerIndex = 0;
int MinecraftServer::s_slowQueueLastTime = 0;
bool MinecraftServer::s_slowQueuePacketSent = false;
#endif

unordered_map<wstring, int> MinecraftServer::ironTimers;

static bool ShouldUseDedicatedServerProperties()
{
#ifdef _WINDOWS64
	return g_Win64DedicatedServer;
#else
	return false;
#endif
}

static int GetDedicatedServerInt(Settings *settings, const wchar_t *key, int defaultValue)
{
	return (ShouldUseDedicatedServerProperties() && settings != NULL) ? settings->getInt(key, defaultValue) : defaultValue;
}

static bool GetDedicatedServerBool(Settings *settings, const wchar_t *key, bool defaultValue)
{
	return (ShouldUseDedicatedServerProperties() && settings != NULL) ? settings->getBoolean(key, defaultValue) : defaultValue;
}

static wstring GetDedicatedServerString(Settings *settings, const wchar_t *key, const wstring &defaultValue)
{
	return (ShouldUseDedicatedServerProperties() && settings != NULL) ? settings->getString(key, defaultValue) : defaultValue;
}

static void PrintConsoleLine(const wchar_t *prefix, const wstring &message)
{
	wprintf(L"%ls%ls\n", prefix, message.c_str());
	fflush(stdout);
}

static bool TryParseIntValue(const wstring &text, int &value)
{
	std::wistringstream stream(text);
	stream >> value;
	return !stream.fail() && stream.eof();
}

static vector<wstring> SplitConsoleCommand(const wstring &command)
{
	vector<wstring> tokens;
	std::wistringstream stream(command);
	wstring token;
	while (stream >> token)
	{
		tokens.push_back(token);
	}
	return tokens;
}

static wstring JoinConsoleCommandTokens(const vector<wstring> &tokens, size_t startIndex)
{
	wstring joined;
	for (size_t i = startIndex; i < tokens.size(); ++i)
	{
		if (!joined.empty()) joined += L" ";
		joined += tokens[i];
	}
	return joined;
}

static shared_ptr<ServerPlayer> FindPlayerByName(PlayerList *playerList, const wstring &name)
{
	if (playerList == NULL) return nullptr;

	for (size_t i = 0; i < playerList->players.size(); ++i)
	{
		shared_ptr<ServerPlayer> player = playerList->players[i];
		if (player != NULL && equalsIgnoreCase(player->getName(), name))
		{
			return player;
		}
	}

	return nullptr;
}

static void SetAllLevelTimes(MinecraftServer *server, int value)
{
	for (unsigned int i = 0; i < server->levels.length; ++i)
	{
		if (server->levels[i] != NULL)
		{
			server->levels[i]->setDayTime(value);
		}
	}
}

static bool ExecuteConsoleCommand(MinecraftServer *server, const wstring &rawCommand)
{
	if (server == NULL)
		return false;

	wstring command = trimString(rawCommand);
	if (command.empty())
		return true;

	if (command[0] == L'/')
	{
		command = trimString(command.substr(1));
	}

	vector<wstring> tokens = SplitConsoleCommand(command);
	if (tokens.empty())
		return true;

	const wstring action = toLower(tokens[0]);
	PlayerList *playerList = server->getPlayers();

	if (action == L"help" || action == L"?")
	{
		server->info(L"Commands: help, stop, list, say <message>, save-all, time <set day|night|ticks|add ticks>, weather <clear|rain|thunder> [seconds], tp <player> <target>, give <player> <itemId> [amount] [aux], enchant <player> <enchantId> [level], kill <player>");
		return true;
	}

	if (action == L"stop")
	{
		server->info(L"Stopping server...");
		MinecraftServer::HaltServer();
		return true;
	}

	if (action == L"list")
	{
		wstring playerNames = (playerList != NULL) ? playerList->getPlayerNames() : L"";
		if (playerNames.empty()) playerNames = L"(none)";
		server->info(L"Players (" + std::to_wstring((playerList != NULL) ? playerList->getPlayerCount() : 0) + L"): " + playerNames);
		return true;
	}

	if (action == L"say")
	{
		if (tokens.size() < 2)
		{
			server->warn(L"Usage: say <message>");
			return false;
		}

		wstring message = L"[Server] " + JoinConsoleCommandTokens(tokens, 1);
		if (playerList != NULL)
		{
			playerList->broadcastAll(shared_ptr<ChatPacket>(new ChatPacket(message)));
		}
		server->info(message);
		return true;
	}

	if (action == L"save-all")
	{
		if (playerList != NULL)
		{
			playerList->saveAll(NULL, false);
		}
		server->info(L"World saved.");
		return true;
	}

	if (action == L"time")
	{
		if (tokens.size() < 2)
		{
			server->warn(L"Usage: time set <day|night|ticks> | time add <ticks>");
			return false;
		}

		if (toLower(tokens[1]) == L"add")
		{
			if (tokens.size() < 3)
			{
				server->warn(L"Usage: time add <ticks>");
				return false;
			}

			int delta = 0;
			if (!TryParseIntValue(tokens[2], delta))
			{
				server->warn(L"Invalid tick value: " + tokens[2]);
				return false;
			}

			for (unsigned int i = 0; i < server->levels.length; ++i)
			{
				if (server->levels[i] != NULL)
				{
					server->levels[i]->setDayTime(server->levels[i]->getDayTime() + delta);
				}
			}

			server->info(L"Added " + std::to_wstring(delta) + L" ticks.");
			return true;
		}

		wstring timeValue = toLower(tokens[1]);
		if (timeValue == L"set")
		{
			if (tokens.size() < 3)
			{
				server->warn(L"Usage: time set <day|night|ticks>");
				return false;
			}
			timeValue = toLower(tokens[2]);
		}

		int targetTime = 0;
		if (timeValue == L"day")
		{
			targetTime = 0;
		}
		else if (timeValue == L"night")
		{
			targetTime = 12500;
		}
		else if (!TryParseIntValue(timeValue, targetTime))
		{
			server->warn(L"Invalid time value: " + timeValue);
			return false;
		}

		SetAllLevelTimes(server, targetTime);
		server->info(L"Time set to " + std::to_wstring(targetTime) + L".");
		return true;
	}

	if (action == L"weather")
	{
		if (tokens.size() < 2)
		{
			server->warn(L"Usage: weather <clear|rain|thunder> [seconds]");
			return false;
		}

		int durationSeconds = 600;
		if (tokens.size() >= 3 && !TryParseIntValue(tokens[2], durationSeconds))
		{
			server->warn(L"Invalid duration: " + tokens[2]);
			return false;
		}

		if (server->levels[0] == NULL)
		{
			server->warn(L"The overworld is not loaded.");
			return false;
		}

		LevelData *levelData = server->levels[0]->getLevelData();
		int duration = durationSeconds * SharedConstants::TICKS_PER_SECOND;
		levelData->setRainTime(duration);
		levelData->setThunderTime(duration);

		wstring weather = toLower(tokens[1]);
		if (weather == L"clear")
		{
			levelData->setRaining(false);
			levelData->setThundering(false);
		}
		else if (weather == L"rain")
		{
			levelData->setRaining(true);
			levelData->setThundering(false);
		}
		else if (weather == L"thunder")
		{
			levelData->setRaining(true);
			levelData->setThundering(true);
		}
		else
		{
			server->warn(L"Usage: weather <clear|rain|thunder> [seconds]");
			return false;
		}

		server->info(L"Weather set to " + weather + L".");
		return true;
	}

	if (action == L"tp" || action == L"teleport")
	{
		if (tokens.size() < 3)
		{
			server->warn(L"Usage: tp <player> <target>");
			return false;
		}

		shared_ptr<ServerPlayer> subject = FindPlayerByName(playerList, tokens[1]);
		shared_ptr<ServerPlayer> destination = FindPlayerByName(playerList, tokens[2]);
		if (subject == NULL)
		{
			server->warn(L"Unknown player: " + tokens[1]);
			return false;
		}
		if (destination == NULL)
		{
			server->warn(L"Unknown player: " + tokens[2]);
			return false;
		}
		if (subject->level->dimension->id != destination->level->dimension->id || !subject->isAlive())
		{
			server->warn(L"Teleport failed because the players are not in the same dimension or the source player is dead.");
			return false;
		}

		subject->ride(nullptr);
		subject->connection->teleport(destination->x, destination->y, destination->z, destination->yRot, destination->xRot);
		server->info(L"Teleported " + subject->getName() + L" to " + destination->getName() + L".");
		return true;
	}

	if (action == L"give")
	{
		if (tokens.size() < 3)
		{
			server->warn(L"Usage: give <player> <itemId> [amount] [aux]");
			return false;
		}

		shared_ptr<ServerPlayer> player = FindPlayerByName(playerList, tokens[1]);
		if (player == NULL)
		{
			server->warn(L"Unknown player: " + tokens[1]);
			return false;
		}

		int itemId = 0;
		int amount = 1;
		int aux = 0;
		if (!TryParseIntValue(tokens[2], itemId))
		{
			server->warn(L"Invalid item id: " + tokens[2]);
			return false;
		}
		if (tokens.size() >= 4 && !TryParseIntValue(tokens[3], amount))
		{
			server->warn(L"Invalid amount: " + tokens[3]);
			return false;
		}
		if (tokens.size() >= 5 && !TryParseIntValue(tokens[4], aux))
		{
			server->warn(L"Invalid aux value: " + tokens[4]);
			return false;
		}
		if (itemId <= 0 || Item::items[itemId] == NULL)
		{
			server->warn(L"Unknown item id: " + std::to_wstring(itemId));
			return false;
		}
		if (amount <= 0)
		{
			server->warn(L"Amount must be positive.");
			return false;
		}

		shared_ptr<ItemInstance> itemInstance(new ItemInstance(itemId, amount, aux));
		shared_ptr<ItemEntity> drop = player->drop(itemInstance);
		if (drop != NULL)
		{
			drop->throwTime = 0;
		}
		server->info(L"Gave item " + std::to_wstring(itemId) + L" x" + std::to_wstring(amount) + L" to " + player->getName() + L".");
		return true;
	}

	if (action == L"enchant")
	{
		if (tokens.size() < 3)
		{
			server->warn(L"Usage: enchant <player> <enchantId> [level]");
			return false;
		}

		shared_ptr<ServerPlayer> player = FindPlayerByName(playerList, tokens[1]);
		if (player == NULL)
		{
			server->warn(L"Unknown player: " + tokens[1]);
			return false;
		}

		int enchantmentId = 0;
		int enchantmentLevel = 1;
		if (!TryParseIntValue(tokens[2], enchantmentId))
		{
			server->warn(L"Invalid enchantment id: " + tokens[2]);
			return false;
		}
		if (tokens.size() >= 4 && !TryParseIntValue(tokens[3], enchantmentLevel))
		{
			server->warn(L"Invalid enchantment level: " + tokens[3]);
			return false;
		}

		shared_ptr<ItemInstance> selectedItem = player->getSelectedItem();
		if (selectedItem == NULL)
		{
			server->warn(L"The player is not holding an item.");
			return false;
		}

		Enchantment *enchantment = Enchantment::enchantments[enchantmentId];
		if (enchantment == NULL)
		{
			server->warn(L"Unknown enchantment id: " + std::to_wstring(enchantmentId));
			return false;
		}
		if (!enchantment->canEnchant(selectedItem))
		{
			server->warn(L"That enchantment cannot be applied to the selected item.");
			return false;
		}

		if (enchantmentLevel < enchantment->getMinLevel()) enchantmentLevel = enchantment->getMinLevel();
		if (enchantmentLevel > enchantment->getMaxLevel()) enchantmentLevel = enchantment->getMaxLevel();

		if (selectedItem->hasTag())
		{
			ListTag<CompoundTag> *enchantmentTags = selectedItem->getEnchantmentTags();
			if (enchantmentTags != NULL)
			{
				for (int i = 0; i < enchantmentTags->size(); i++)
				{
					int type = enchantmentTags->get(i)->getShort((wchar_t *)ItemInstance::TAG_ENCH_ID);
					if (Enchantment::enchantments[type] != NULL && !Enchantment::enchantments[type]->isCompatibleWith(enchantment))
					{
						server->warn(L"That enchantment conflicts with an existing enchantment on the selected item.");
						return false;
					}
				}
			}
		}

		selectedItem->enchant(enchantment, enchantmentLevel);
		server->info(L"Enchanted " + player->getName() + L"'s held item with " + std::to_wstring(enchantmentId) + L" " + std::to_wstring(enchantmentLevel) + L".");
		return true;
	}

	if (action == L"kill")
	{
		if (tokens.size() < 2)
		{
			server->warn(L"Usage: kill <player>");
			return false;
		}

		shared_ptr<ServerPlayer> player = FindPlayerByName(playerList, tokens[1]);
		if (player == NULL)
		{
			server->warn(L"Unknown player: " + tokens[1]);
			return false;
		}

		player->hurt(DamageSource::outOfWorld, 3.4e38f);
		server->info(L"Killed " + player->getName() + L".");
		return true;
	}

	server->warn(L"Unknown command: " + command);
	return false;
}

MinecraftServer::MinecraftServer()
{
	// 4J - added initialisers
	connection = NULL;
	settings = NULL;
	players = NULL;
	commands = NULL;
	running = true;
	m_bLoaded = false;
	stopped = false;
	tickCount = 0;
	wstring progressStatus;
	progress = 0;
	motd = L"";

	m_isServerPaused = false;
	m_serverPausedEvent = new C4JThread::Event;

	m_saveOnExit = false;
	m_suspending = false;

	m_ugcPlayersVersion = 0;
	m_texturePackId = 0;
	maxBuildHeight = Level::maxBuildHeight;
	playerIdleTimeout = 0;
	m_postUpdateThread = NULL;
	forceGameType = false;

	commandDispatcher = new ServerCommandDispatcher();
	InitializeCriticalSection(&m_consoleInputCS);

	DispenserBootstrap::bootStrap();
}

MinecraftServer::~MinecraftServer()
{
	DeleteCriticalSection(&m_consoleInputCS);
}

bool MinecraftServer::initServer(int64_t seed, NetworkGameInitData *initData, DWORD initSettings, bool findSeed)
{
	// 4J - removed
#if 0
	commands = new ConsoleCommands(this);

	Thread t = new Thread() {
		public void run() {
			BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
			String line = null;
			try {
				while (!stopped && running && (line = br.readLine()) != null) {
					handleConsoleInput(line, MinecraftServer.this);
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	};
	t.setDaemon(true);
	t.start();


	LogConfigurator.initLogger();
	logger.info("Starting minecraft server version " + VERSION);

	if (Runtime.getRuntime().maxMemory() / 1024 / 1024 < 512) {
		logger.warning("**** NOT ENOUGH RAM!");
		logger.warning("To start the server with more ram, launch it as \"java -Xmx1024M -Xms1024M -jar minecraft_server.jar\"");
	}

	logger.info("Loading properties");
#endif
	settings = new Settings(new File(L"server.properties"));

	app.SetGameHostOption(eGameHostOption_Difficulty, GetDedicatedServerInt(settings, L"difficulty", app.GetGameHostOption(eGameHostOption_Difficulty)));
	app.SetGameHostOption(eGameHostOption_GameType, GetDedicatedServerInt(settings, L"gamemode", app.GetGameHostOption(eGameHostOption_GameType)));
	app.SetGameHostOption(eGameHostOption_Structures, GetDedicatedServerBool(settings, L"generate-structures", app.GetGameHostOption(eGameHostOption_Structures) > 0) ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_BonusChest, GetDedicatedServerBool(settings, L"bonus-chest", app.GetGameHostOption(eGameHostOption_BonusChest) > 0) ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_PvP, GetDedicatedServerBool(settings, L"pvp", app.GetGameHostOption(eGameHostOption_PvP) > 0) ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_TrustPlayers, GetDedicatedServerBool(settings, L"trust-players", app.GetGameHostOption(eGameHostOption_TrustPlayers) > 0) ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_FireSpreads, GetDedicatedServerBool(settings, L"fire-spreads", app.GetGameHostOption(eGameHostOption_FireSpreads) > 0) ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_TNT, GetDedicatedServerBool(settings, L"tnt", app.GetGameHostOption(eGameHostOption_TNT) > 0) ? 1 : 0);

	app.DebugPrintf("\n*** SERVER SETTINGS ***\n");
	app.DebugPrintf("ServerSettings: host-friends-only is %s\n",(app.GetGameHostOption(eGameHostOption_FriendsOfFriends)>0)?"on":"off");
	app.DebugPrintf("ServerSettings: game-type is %s\n",(app.GetGameHostOption(eGameHostOption_GameType)==0)?"Survival Mode":"Creative Mode");
	app.DebugPrintf("ServerSettings: pvp is %s\n",(app.GetGameHostOption(eGameHostOption_PvP)>0)?"on":"off");
	app.DebugPrintf("ServerSettings: fire spreads is %s\n",(app.GetGameHostOption(eGameHostOption_FireSpreads)>0)?"on":"off");
	app.DebugPrintf("ServerSettings: tnt explodes is %s\n",(app.GetGameHostOption(eGameHostOption_TNT)>0)?"on":"off");
	app.DebugPrintf("\n");

	// TODO 4J Stu - Init a load of settings based on data passed as params
	//settings->setBooleanAndSave( L"host-friends-only", (app.GetGameHostOption(eGameHostOption_FriendsOfFriends)>0) );

	// 4J - Unused
	//localIp = settings->getString(L"server-ip", L"");
	//onlineMode = settings->getBoolean(L"online-mode", true);
	//motd = settings->getString(L"motd", L"A Minecraft Server");
	//motd.replace('�', '$');

	setAnimals(GetDedicatedServerBool(settings, L"spawn-animals", true));
	setNpcsEnabled(GetDedicatedServerBool(settings, L"spawn-npcs", true));
	setPvpAllowed(app.GetGameHostOption( eGameHostOption_PvP )>0?true:false);

	// 4J Stu - We should never have hacked clients flying when they shouldn't be like the PC version, so enable flying always
	// Fix for #46612 - TU5: Code: Multiplayer: A client can be banned for flying when accidentaly being blown by dynamite
	setFlightAllowed(GetDedicatedServerBool(settings, L"allow-flight", true));

	// 4J Stu - Enabling flight to stop it kicking us when we use it
#ifdef _DEBUG_MENUS_ENABLED
	setFlightAllowed(true);
#endif

#if 1
	connection = new ServerConnection(this);
	Socket::Initialise(connection);	// 4J - added
#else
	// 4J - removed
	InetAddress localAddress = null;
	if (localIp.length() > 0) localAddress = InetAddress.getByName(localIp);
	port = settings.getInt("server-port", DEFAULT_MINECRAFT_PORT);

	logger.info("Starting Minecraft server on " + (localIp.length() == 0 ? "*" : localIp) + ":" + port);
	try {
		connection = new ServerConnection(this, localAddress, port);
	} catch (IOException e) {
		logger.warning("**** FAILED TO BIND TO PORT!");
		logger.log(Level.WARNING, "The exception was: " + e.toString());
		logger.warning("Perhaps a server is already running on that port?");
		return false;
	}

	if (!onlineMode) {
		logger.warning("**** SERVER IS RUNNING IN OFFLINE/INSECURE MODE!");
		logger.warning("The server will make no attempt to authenticate usernames. Beware.");
		logger.warning("While this makes the game possible to play without internet access, it also opens up the ability for hackers to connect with any username they choose.");
		logger.warning("To change this, set \"online-mode\" to \"true\" in the server.settings file.");
	}
#endif
	setPlayers(new PlayerList(this));
#ifdef _WINDOWS64
	{
		int maxP = getPlayerList()->getMaxPlayers();
		WinsockNetLayer::UpdateAdvertiseMaxPlayers((BYTE)(maxP > 255 ? 255 : maxP));
	}
#endif

	// 4J-JEV: Need to wait for levelGenerationOptions to load.
	while ( app.getLevelGenerationOptions() != NULL && !app.getLevelGenerationOptions()->hasLoadedData() )
		Sleep(1);

	if ( app.getLevelGenerationOptions() != NULL && !app.getLevelGenerationOptions()->ready() )
	{
		// TODO: Stop loading, add error message.
	}

	int64_t levelNanoTime = System::nanoTime();

        wstring levelName = (initData && !initData->levelName.empty()) ? initData->levelName : GetDedicatedServerString(settings, L"level-name", L"world");
		wstring levelTypeString;

	bool gameRuleUseFlatWorld = false;
	if(app.getLevelGenerationOptions() != NULL)
	{
		gameRuleUseFlatWorld = app.getLevelGenerationOptions()->getuseFlatWorld();
	}
	if(gameRuleUseFlatWorld || app.GetGameHostOption(eGameHostOption_LevelType)>0)
	{
		levelTypeString = GetDedicatedServerString(settings, L"level-type",  L"flat");
	}
	else
	{
		levelTypeString = GetDedicatedServerString(settings, L"level-type",L"default");
	}

	LevelType *pLevelType = LevelType::getLevelType(levelTypeString);
	if (pLevelType == NULL)
	{
		pLevelType = LevelType::lvl_normal;
	}

	ProgressRenderer *mcprogress = Minecraft::GetInstance()->progressRenderer;
	mcprogress->progressStart(IDS_PROGRESS_INITIALISING_SERVER);

	if( findSeed )
	{
#ifdef __PSVITA__
		seed = BiomeSource::findSeed(pLevelType, &running);
#else
		seed = BiomeSource::findSeed(pLevelType);
#endif
	}

	setMaxBuildHeight(GetDedicatedServerInt(settings, L"max-build-height", Level::maxBuildHeight));
	setMaxBuildHeight(((getMaxBuildHeight() + 8) / 16) * 16);
	setMaxBuildHeight(Mth::clamp(getMaxBuildHeight(), 64, Level::maxBuildHeight));
	//settings->setProperty(L"max-build-height", maxBuildHeight);

#if 0
	wstring levelSeedString = settings->getString(L"level-seed", L"");
	int64_t levelSeed = (new Random())->nextLong();
	if (levelSeedString.length() > 0)
	{
		long newSeed = _fromString<int64_t>(levelSeedString);
		if (newSeed != 0) {
			levelSeed = newSeed;
		}
	}
#endif
	//        logger.info("Preparing level \"" + levelName + "\"");
	m_bLoaded = loadLevel(new McRegionLevelStorageSource(File(L".")), levelName, seed, pLevelType, initData);
	//        logger.info("Done (" + (System.nanoTime() - levelNanoTime) + "ns)! For help, type \"help\" or \"?\"");

	// 4J delete passed in save data now - this is only required for the tutorial which is loaded by passing data directly in rather than using the storage manager
	if( initData->saveData )
	{
		delete initData->saveData->data;
		initData->saveData->data = 0;
		initData->saveData->fileSize = 0;
	}

	g_NetworkManager.ServerReady();	// 4J added
	return m_bLoaded;

}

// 4J - added - extra thread to post processing on separate thread during level creation
int MinecraftServer::runPostUpdate(void* lpParam)
{
	ShutdownManager::HasStarted(ShutdownManager::ePostProcessThread);

	MinecraftServer *server = (MinecraftServer *)lpParam;
	Entity::useSmallIds();		// This thread can end up spawning entities as resources
	IntCache::CreateNewThreadStorage();
	AABB::CreateNewThreadStorage();
	Vec3::CreateNewThreadStorage();
	Compression::UseDefaultThreadStorage();
	Level::enableLightingCache();
	Tile::CreateNewThreadStorage();

	// Update lights for both levels until we are signalled to terminate
	do
	{
		EnterCriticalSection(&server->m_postProcessCS);
		if( server->m_postProcessRequests.size() )
		{
			MinecraftServer::postProcessRequest request = server->m_postProcessRequests.back();
			server->m_postProcessRequests.pop_back();
			LeaveCriticalSection(&server->m_postProcessCS);
			static int count = 0;
			PIXBeginNamedEvent(0,"Post processing %d ", (count++)%8);
			request.chunkSource->postProcess(request.chunkSource, request.x, request.z );
			PIXEndNamedEvent();
		}
		else
		{
			LeaveCriticalSection(&server->m_postProcessCS);
		}
		Sleep(1);
	} while (!server->m_postUpdateTerminate && ShutdownManager::ShouldRun(ShutdownManager::ePostProcessThread));
	//#ifndef __PS3__
	// One final pass through updates to make sure we're done
	EnterCriticalSection(&server->m_postProcessCS);
	int maxRequests = server->m_postProcessRequests.size();
	while(server->m_postProcessRequests.size() && ShutdownManager::ShouldRun(ShutdownManager::ePostProcessThread) )
	{
		MinecraftServer::postProcessRequest request = server->m_postProcessRequests.back();
		server->m_postProcessRequests.pop_back();
		LeaveCriticalSection(&server->m_postProcessCS);
		request.chunkSource->postProcess(request.chunkSource, request.x, request.z );
#ifdef __PS3__
#ifndef _CONTENT_PACKAGE
		if((server->m_postProcessRequests.size() % 10) == 0)
			printf("processing request %00d\n", server->m_postProcessRequests.size());
#endif
		Sleep(1);
#endif
		EnterCriticalSection(&server->m_postProcessCS);
	}
	LeaveCriticalSection(&server->m_postProcessCS);
	//#endif //__PS3__
	Tile::ReleaseThreadStorage();
	IntCache::ReleaseThreadStorage();
	AABB::ReleaseThreadStorage();
	Vec3::ReleaseThreadStorage();
	Level::destroyLightingCache();

	ShutdownManager::HasFinished(ShutdownManager::ePostProcessThread);

	return 0;
}

void	MinecraftServer::addPostProcessRequest(ChunkSource *chunkSource, int x, int z)
{
	EnterCriticalSection(&m_postProcessCS);
	m_postProcessRequests.push_back(MinecraftServer::postProcessRequest(x,z,chunkSource));
	LeaveCriticalSection(&m_postProcessCS);
}

void MinecraftServer::postProcessTerminate(ProgressRenderer *mcprogress)
{
	DWORD status = 0;

	EnterCriticalSection(&server->m_postProcessCS);
	size_t postProcessItemCount = server->m_postProcessRequests.size();
	LeaveCriticalSection(&server->m_postProcessCS);

	do
	{
		status = m_postUpdateThread->WaitForCompletion(50);
		if( status == WAIT_TIMEOUT )
		{
			EnterCriticalSection(&server->m_postProcessCS);
			size_t postProcessItemRemaining = server->m_postProcessRequests.size();
			LeaveCriticalSection(&server->m_postProcessCS);

			if( postProcessItemCount )
			{
				mcprogress->progressStagePercentage((postProcessItemCount - postProcessItemRemaining) * 100 / postProcessItemCount);
			}
			CompressedTileStorage::tick();
			SparseLightStorage::tick();
			SparseDataStorage::tick();
		}
	} while ( status == WAIT_TIMEOUT );
	delete m_postUpdateThread;
	m_postUpdateThread = NULL;
	DeleteCriticalSection(&m_postProcessCS);
}

bool MinecraftServer::loadLevel(LevelStorageSource *storageSource, const wstring& name, int64_t levelSeed, LevelType *pLevelType, NetworkGameInitData *initData)
{
	//	4J - TODO - do with new save stuff
	//    if (storageSource->requiresConversion(name))
	//	{
	//		assert(false);
	//    }
	ProgressRenderer *mcprogress = Minecraft::GetInstance()->progressRenderer;

	// 4J TODO - free levels here if there are already some?
	levels = ServerLevelArray(3);

	int gameTypeId = GetDedicatedServerInt(settings, L"gamemode", app.GetGameHostOption(eGameHostOption_GameType));//LevelSettings::GAMETYPE_SURVIVAL);
	GameType *gameType = LevelSettings::validateGameType(gameTypeId);
	app.DebugPrintf("Default game type: %d\n" , gameTypeId);

	LevelSettings *levelSettings = new LevelSettings(levelSeed, gameType, app.GetGameHostOption(eGameHostOption_Structures)>0?true:false, isHardcore(), true, pLevelType, initData->xzSize, initData->hellScale);
	if( app.GetGameHostOption(eGameHostOption_BonusChest ) ) levelSettings->enableStartingBonusItems();

	// 4J - temp - load existing level
	shared_ptr<McRegionLevelStorage> storage = nullptr;
	bool levelChunksNeedConverted = false;
	if( initData->saveData != NULL )
	{
		// We are loading a file from disk with the data passed in

#ifdef SPLIT_SAVES
		ConsoleSaveFileOriginal oldFormatSave( initData->saveData->saveName, initData->saveData->data, initData->saveData->fileSize, false, initData->savePlatform );
		ConsoleSaveFile* pSave = new ConsoleSaveFileSplit( &oldFormatSave );

		//ConsoleSaveFile* pSave = new ConsoleSaveFileSplit( initData->saveData->saveName, initData->saveData->data, initData->saveData->fileSize, false, initData->savePlatform );
#else
		ConsoleSaveFile* pSave = new ConsoleSaveFileOriginal( initData->saveData->saveName, initData->saveData->data, initData->saveData->fileSize, false, initData->savePlatform );
#endif
		if(pSave->isSaveEndianDifferent())
			levelChunksNeedConverted = true;
		pSave->ConvertToLocalPlatform(); // check if we need to convert this file from PS3->PS4

		storage = shared_ptr<McRegionLevelStorage>(new McRegionLevelStorage(pSave, File(L"."), name, true));
	}
	else
	{
		// We are loading a save from the storage manager
#ifdef SPLIT_SAVES
		bool bLevelGenBaseSave = false;
		LevelGenerationOptions *levelGen = app.getLevelGenerationOptions();
		if( levelGen != NULL && levelGen->requiresBaseSave())
		{
			DWORD fileSize = 0;
			LPVOID pvSaveData = levelGen->getBaseSaveData(fileSize);
			if(pvSaveData && fileSize != 0) bLevelGenBaseSave = true;
		}
		ConsoleSaveFileSplit *newFormatSave = NULL;
		if(bLevelGenBaseSave)
		{
			ConsoleSaveFileOriginal oldFormatSave( L"" );
			newFormatSave = new ConsoleSaveFileSplit( &oldFormatSave );
		}
		else
		{
			newFormatSave = new ConsoleSaveFileSplit( L"" );
		}

		storage = shared_ptr<McRegionLevelStorage>(new McRegionLevelStorage(newFormatSave, File(L"."), name, true));
#else
		storage = shared_ptr<McRegionLevelStorage>(new McRegionLevelStorage(new ConsoleSaveFileOriginal( L"" ), File(L"."), name, true));
#endif
	}

	//	McRegionLevelStorage *storage = new McRegionLevelStorage(new ConsoleSaveFile( L"" ), L"", L"", 0); // original
	//    McRegionLevelStorage *storage = new McRegionLevelStorage(File(L"."), name, true); // TODO
	for (unsigned int i = 0; i < levels.length; i++)
	{
		if( s_bServerHalted || !g_NetworkManager.IsInSession() )
		{
			return false;
		}

		//            String levelName = name;
		//            if (i == 1) levelName += "_nether";
		int dimension = 0;
		if (i == 1) dimension = -1;
		if (i == 2) dimension = 1;
		if (i == 0)
		{
			levels[i] = new ServerLevel(this, storage, name, dimension, levelSettings);
			if(app.getLevelGenerationOptions() != NULL)
			{
				LevelGenerationOptions *mapOptions = app.getLevelGenerationOptions();
				Pos *spawnPos = mapOptions->getSpawnPos();
				if( spawnPos != NULL )
				{
					levels[i]->setSpawnPos( spawnPos );
				}

				levels[i]->getLevelData()->setHasBeenInCreative(mapOptions->isFromDLC());
			}
		}
		else levels[i] = new DerivedServerLevel(this, storage, name, dimension, levelSettings, levels[0]);
		//        levels[i]->addListener(new ServerLevelListener(this, levels[i]));		// 4J - have moved this to the ServerLevel ctor so that it is set up in time for the first chunk to load, which might actually happen there

		// 4J Stu - We set the levels difficulty based on the minecraft options
		//levels[i]->difficulty = settings->getBoolean(L"spawn-monsters", true) ? Difficulty::EASY : Difficulty::PEACEFUL;
		Minecraft *pMinecraft = Minecraft::GetInstance();
		//		m_lastSentDifficulty = pMinecraft->options->difficulty;
		levels[i]->difficulty = app.GetGameHostOption(eGameHostOption_Difficulty); //pMinecraft->options->difficulty;
		app.DebugPrintf("MinecraftServer::loadLevel - Difficulty = %d\n",levels[i]->difficulty);

#if DEBUG_SERVER_DONT_SPAWN_MOBS
		levels[i]->setSpawnSettings(false, false);
#else
		levels[i]->setSpawnSettings(GetDedicatedServerBool(settings, L"spawn-monsters", true), animals);
#endif
		levels[i]->getLevelData()->setGameType(gameType);

		if(app.getLevelGenerationOptions() != NULL)
		{
			LevelGenerationOptions *mapOptions = app.getLevelGenerationOptions();
			levels[i]->getLevelData()->setHasBeenInCreative(mapOptions->getLevelHasBeenInCreative() );
		}

		players->setLevel(levels);
	}

	if( levels[0]->isNew )
	{
		mcprogress->progressStage(IDS_PROGRESS_GENERATING_SPAWN_AREA);
	}
	else
	{
		mcprogress->progressStage(IDS_PROGRESS_LOADING_SPAWN_AREA);
	}
	app.SetGameHostOption( eGameHostOption_HasBeenInCreative, gameType == GameType::CREATIVE || levels[0]->getHasBeenInCreative() );
	app.SetGameHostOption( eGameHostOption_Structures, levels[0]->isGenerateMapFeatures() );

	if( s_bServerHalted || !g_NetworkManager.IsInSession() ) return false;

	// 4J - Make a new thread to do post processing
	InitializeCriticalSection(&m_postProcessCS);

	// 4J-PB - fix for 108310 - TCR #001 BAS Game Stability: TU12: Code: Compliance: Crash after creating world on "journey" seed.
	// Stack gets very deep with some sand tower falling, so increased the stacj to 256K from 128k on other platforms (was already set to that on PS3 and Orbis)

	m_postUpdateThread = new C4JThread(runPostUpdate, this, "Post processing", 256*1024);

	m_postUpdateTerminate = false;
	m_postUpdateThread->SetProcessor(CPU_CORE_POST_PROCESSING);
	m_postUpdateThread->SetPriority(THREAD_PRIORITY_ABOVE_NORMAL);
	m_postUpdateThread->Run();

	int64_t startTime = System::currentTimeMillis();

	// 4J Stu - Added this to temporarily make starting games on vita faster
#ifdef __PSVITA__
	int r = 48;
#else
	int r = 196;
#endif

	//  4J JEV: load gameRules.
	ConsoleSavePath filepath(GAME_RULE_SAVENAME);
	ConsoleSaveFile *csf = getLevel(0)->getLevelStorage()->getSaveFile();
	if( csf->doesFileExist(filepath) )
	{
		DWORD numberOfBytesRead;
		byteArray ba_gameRules;

		FileEntry *fe = csf->createFile(filepath);

		ba_gameRules.length = fe->getFileSize();
		ba_gameRules.data = new BYTE[ ba_gameRules.length ];

		csf->setFilePointer(fe,0,NULL,FILE_BEGIN);
		csf->readFile(fe, ba_gameRules.data, ba_gameRules.length, &numberOfBytesRead);
		assert(numberOfBytesRead == ba_gameRules.length);

		app.m_gameRules.loadGameRules(ba_gameRules.data, ba_gameRules.length);
		csf->closeHandle(fe);
	}

	int64_t lastTime = System::currentTimeMillis();
#ifdef _LARGE_WORLDS
	if(app.GetGameNewWorldSize() > levels[0]->getLevelData()->getXZSizeOld())
	{
		if(!app.GetGameNewWorldSizeUseMoat()) // check the moat settings to see if we should be overwriting the edge tiles
		{
			overwriteBordersForNewWorldSize(levels[0]);
		}
		// we're always overwriting hell edges
		int oldHellSize = levels[0]->getLevelData()->getXZHellSizeOld();
		overwriteHellBordersForNewWorldSize(levels[1], oldHellSize);
	}
#endif

	// 4J Stu - This loop is changed in 1.0.1 to only process the first level (ie the overworld), but I think we still want to do them all
	int i = 0;
	for (int i = 0; i < levels.length ; i++)
	{
		//        logger.info("Preparing start region for level " + i);
		if (i == 0 || GetDedicatedServerBool(settings, L"allow-nether", true))
		{
			ServerLevel *level = levels[i];
			if(levelChunksNeedConverted)
			{
				// 				storage->getSaveFile()->convertLevelChunks(level)
			}

#if 0
			int64_t lastStorageTickTime = System::currentTimeMillis();

			// Test code to enable full creation of levels at start up
			int halfsidelen = ( i == 0 ) ? 27 : 9;
			for( int x = -halfsidelen; x < halfsidelen; x++ )
			{
				for( int z = -halfsidelen; z < halfsidelen; z++ )
				{
					int total = halfsidelen * halfsidelen * 4;
					int pos = z + halfsidelen + ( ( x + halfsidelen ) * 2 * halfsidelen );
					mcprogress->progressStagePercentage((pos) * 100 / total);
					level->cache->create(x,z, true);	// 4J - added parameter to disable postprocessing here

					if( System::currentTimeMillis() - lastStorageTickTime > 50 )
					{
						CompressedTileStorage::tick();
						SparseLightStorage::tick();
						SparseDataStorage::tick();
						lastStorageTickTime = System::currentTimeMillis();
					}
				}
			}
#else
			int64_t lastStorageTickTime = System::currentTimeMillis();
			Pos *spawnPos = level->getSharedSpawnPos();

			int twoRPlusOne = r*2 + 1;
			int total = twoRPlusOne * twoRPlusOne;
			for (int x = -r; x <= r && running; x += 16)
			{
				for (int z = -r; z <= r && running; z += 16)
				{
					if( s_bServerHalted || !g_NetworkManager.IsInSession() )
					{
						delete spawnPos;
						m_postUpdateTerminate = true;
						postProcessTerminate(mcprogress);
						return false;
					}
					//					printf(">>>%d %d %d\n",i,x,z);
					//                    int64_t now = System::currentTimeMillis();
					//                    if (now < lastTime) lastTime = now;
					//                    if (now > lastTime + 1000)
					{
						int pos = (x + r) * twoRPlusOne + (z + 1);
						//                        setProgress(L"Preparing spawn area", (pos) * 100 / total);
						mcprogress->progressStagePercentage((pos+r) * 100 / total);
						//                        lastTime = now;
					}
					static int count = 0;
					PIXBeginNamedEvent(0,"Creating %d ", (count++)%8);
					level->cache->create((spawnPos->x + x) >> 4, (spawnPos->z + z) >> 4, true);	// 4J - added parameter to disable postprocessing here
					PIXEndNamedEvent();
					//                    while (level->updateLights() && running)
					//                        ;
					if( System::currentTimeMillis() - lastStorageTickTime > 50 )
					{
						CompressedTileStorage::tick();
						SparseLightStorage::tick();
						SparseDataStorage::tick();
						lastStorageTickTime = System::currentTimeMillis();
					}
				}
			}

			// 4J - removed this as now doing the recheckGaps call when each chunk is post-processed, so can happen on things outside of the spawn area too
#if 0
			// 4J - added this code to propagate lighting properly in the spawn area before we go sharing it with the local client or across the network
			for (int x = -r; x <= r && running; x += 16)
			{
				for (int z = -r; z <= r && running; z += 16)
				{
					PIXBeginNamedEvent(0,"Lighting gaps for %d %d",x,z);
					level->getChunkAt(spawnPos->x + x, spawnPos->z + z)->recheckGaps(true);
					PIXEndNamedEvent();
				}
			}
#endif

			delete spawnPos;
#endif
		}
	}
	//	printf("Main thread complete at %dms\n",System::currentTimeMillis() - startTime);

	// Wait for post processing, then lighting threads, to end (post-processing may make more lighting changes)
	m_postUpdateTerminate = true;

	postProcessTerminate(mcprogress);


	// stronghold position?
	if(levels[0]->dimension->id==0)
	{

		app.DebugPrintf("===================================\n");

		if(!levels[0]->getLevelData()->getHasStronghold())
		{
			int x,z;
			if(app.GetTerrainFeaturePosition(eTerrainFeature_Stronghold,&x,&z))
			{
				levels[0]->getLevelData()->setXStronghold(x);
				levels[0]->getLevelData()->setZStronghold(z);
				levels[0]->getLevelData()->setHasStronghold();

				app.DebugPrintf("=== FOUND stronghold in terrain features list\n");

			}
			else
			{
				// can't find the stronghold position in the terrain feature list. Do we have to run a post-process?
				app.DebugPrintf("=== Can't find stronghold in terrain features list\n");
			}
		}
		else
		{
			app.DebugPrintf("=== Leveldata has stronghold position\n");
		}
		app.DebugPrintf("===================================\n");
	}

	//	printf("Post processing complete at %dms\n",System::currentTimeMillis() - startTime);

	//	printf("Lighting complete at %dms\n",System::currentTimeMillis() - startTime);

	if( s_bServerHalted || !g_NetworkManager.IsInSession() ) return false;

	if( levels[1]->isNew )
	{
		levels[1]->save(true, mcprogress);
	}

	if( s_bServerHalted || !g_NetworkManager.IsInSession() ) return false;

	if( levels[2]->isNew )
	{
		levels[2]->save(true, mcprogress);
	}

	if( s_bServerHalted || !g_NetworkManager.IsInSession() ) return false;

	// 4J - added - immediately save newly created level, like single player game
	// 4J Stu - We also want to immediately save the tutorial
	if ( levels[0]->isNew )
		saveGameRules();

	if( levels[0]->isNew )
	{
		levels[0]->save(true, mcprogress);
	}

	if( s_bServerHalted || !g_NetworkManager.IsInSession() ) return false;

	if( levels[0]->isNew || levels[1]->isNew || levels[2]->isNew )
	{
#ifndef _WINDOWS64
		// On Windows64 we skip the automatic initial save so that choosing
		// "Exit without saving" on a new world does not leave an orphaned save folder.
		levels[0]->saveToDisc(mcprogress, false);
#endif
	}

	if( s_bServerHalted || !g_NetworkManager.IsInSession() ) return false;

	/*
	* int r = 24; for (int x = -r; x <= r; x++) {
	* setProgress("Preparing spawn area", (x + r) * 100 / (r + r + 1)); for (int z
	* = -r; z <= r; z++) { if (!running) return; level.cache.create((level.xSpawn
	* >> 4) + x, (level.zSpawn >> 4) + z); while (running && level.updateLights())
	* ; } }
	*/
	endProgress();

	return true;
}

#ifdef _LARGE_WORLDS
void MinecraftServer::overwriteBordersForNewWorldSize(ServerLevel* level)
{
	// recreate the chunks round the border (2 chunks or 32 blocks deep), deleting any player data from them
	app.DebugPrintf("Expanding level size\n");
	int oldSize = level->getLevelData()->getXZSizeOld();
	// top
	int minVal = -oldSize/2;
	int maxVal = (oldSize/2)-1;
	for(int xVal = minVal; xVal <= maxVal; xVal++)
	{
		int zVal = minVal;
		level->cache->overwriteLevelChunkFromSource(xVal, zVal);
		level->cache->overwriteLevelChunkFromSource(xVal, zVal+1);
	}
	// bottom
	for(int xVal = minVal; xVal <= maxVal; xVal++)
	{
		int zVal = maxVal;
		level->cache->overwriteLevelChunkFromSource(xVal, zVal);
		level->cache->overwriteLevelChunkFromSource(xVal, zVal-1);
	}
	// left
	for(int zVal = minVal; zVal <= maxVal; zVal++)
	{
		int xVal = minVal;
		level->cache->overwriteLevelChunkFromSource(xVal, zVal);
		level->cache->overwriteLevelChunkFromSource(xVal+1, zVal);
	}
	// right
	for(int zVal = minVal; zVal <= maxVal; zVal++)
	{
		int xVal = maxVal;
		level->cache->overwriteLevelChunkFromSource(xVal, zVal);
		level->cache->overwriteLevelChunkFromSource(xVal-1, zVal);
	}
}

void MinecraftServer::overwriteHellBordersForNewWorldSize(ServerLevel* level, int oldHellSize)
{
	// recreate the chunks round the border (1 chunk or 16 blocks deep), deleting any player data from them
	app.DebugPrintf("Expanding level size\n");
	// top
	int minVal = -oldHellSize/2;
	int maxVal = (oldHellSize/2)-1;
	for(int xVal = minVal; xVal <= maxVal; xVal++)
	{
		int zVal = minVal;
		level->cache->overwriteHellLevelChunkFromSource(xVal, zVal, minVal, maxVal);
	}
	// bottom
	for(int xVal = minVal; xVal <= maxVal; xVal++)
	{
		int zVal = maxVal;
		level->cache->overwriteHellLevelChunkFromSource(xVal, zVal, minVal, maxVal);
	}
	// left
	for(int zVal = minVal; zVal <= maxVal; zVal++)
	{
		int xVal = minVal;
		level->cache->overwriteHellLevelChunkFromSource(xVal, zVal, minVal, maxVal);
	}
	// right
	for(int zVal = minVal; zVal <= maxVal; zVal++)
	{
		int xVal = maxVal;
		level->cache->overwriteHellLevelChunkFromSource(xVal, zVal, minVal, maxVal);
	}
}

#endif

void MinecraftServer::setProgress(const wstring& status, int progress)
{
	progressStatus = status;
	this->progress = progress;
	//    logger.info(status + ": " + progress + "%");
}

void MinecraftServer::endProgress()
{
	progressStatus = L"";
	this->progress = 0;
}

void MinecraftServer::saveAllChunks()
{
	//    logger.info("Saving chunks");
	for (unsigned int i = 0; i < levels.length; i++)
	{
		// 4J Stu - Due to the way save mounting is handled on XboxOne, we can actually save after the player has signed out.
#ifndef _XBOX_ONE
		if( m_bPrimaryPlayerSignedOut ) break;
#endif
		// 4J Stu - Save the levels in reverse order so we don't overwrite the level.dat
		// with the data from the nethers leveldata.
		// Fix for #7418 - Functional: Gameplay: Saving after sleeping in a bed will place player at nighttime when restarting.
		ServerLevel *level = levels[levels.length - 1 - i];
		if( level )	// 4J - added check as level can be NULL if we end up in stopServer really early on due to network failure
		{
			level->save(true, Minecraft::GetInstance()->progressRenderer);

			// Only close the level storage when we have saved the last level, otherwise we need to recreate the region files
			// when saving the next levels
			if( i == (levels.length - 1))
			{
				level->closeLevelStorage();
			}
		}
	}
}

// 4J-JEV: Added
void MinecraftServer::saveGameRules()
{
#ifndef _CONTENT_PACKAGE
	if(app.DebugSettingsOn() && app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad())&(1L<<eDebugSetting_DistributableSave))
	{
		// Do nothing
	}
	else
#endif
	{
		byteArray ba;
		ba.data = NULL;
		app.m_gameRules.saveGameRules( &ba.data, &ba.length );

		if (ba.data != NULL)
		{
			ConsoleSaveFile *csf = getLevel(0)->getLevelStorage()->getSaveFile();
			FileEntry *fe = csf->createFile(ConsoleSavePath(GAME_RULE_SAVENAME));
			csf->setFilePointer(fe, 0, NULL, FILE_BEGIN);
			DWORD length;
			csf->writeFile(fe, ba.data, ba.length, &length );

			delete [] ba.data;

			csf->closeHandle(fe);
		}
	}
}

void MinecraftServer::Suspend()
{
	PIXBeginNamedEvent(0,"Suspending server");
	m_suspending = true;
	// Get the frequency of the timer
	LARGE_INTEGER qwTicksPerSec, qwTime, qwNewTime, qwDeltaTime;
	float fElapsedTime = 0.0f;
	QueryPerformanceFrequency( &qwTicksPerSec );
	float fSecsPerTick = 1.0f / (float)qwTicksPerSec.QuadPart;
	// Save the start time
	QueryPerformanceCounter( &qwTime );
	if(m_bLoaded && ProfileManager.IsFullVersion() && (!StorageManager.GetSaveDisabled()))
	{
		if (players != NULL)
		{
			players->saveAll(NULL);
		}
		for (unsigned int j = 0; j < levels.length; j++)
		{
			if( s_bServerHalted ) break;
			// 4J Stu - Save the levels in reverse order so we don't overwrite the level.dat
			// with the data from the nethers leveldata.
			// Fix for #7418 - Functional: Gameplay: Saving after sleeping in a bed will place player at nighttime when restarting.
			ServerLevel *level = levels[levels.length - 1 - j];
			level->Suspend();
		}
		if( !s_bServerHalted )
		{
			saveGameRules();
			levels[0]->saveToDisc(NULL, true);
		}
	}
	QueryPerformanceCounter( &qwNewTime );

	qwDeltaTime.QuadPart = qwNewTime.QuadPart - qwTime.QuadPart;
	fElapsedTime = fSecsPerTick * ((FLOAT)(qwDeltaTime.QuadPart));

	// 4J-JEV: Flush stats and call PlayerSessionExit.
	for (int iPad = 0; iPad < XUSER_MAX_COUNT; iPad++)
	{
		if (ProfileManager.IsSignedIn(iPad))
		{
			TelemetryManager->RecordPlayerSessionExit(iPad, DisconnectPacket::eDisconnect_Quitting);
		}
	}

	m_suspending = false;
	app.DebugPrintf("Suspend server: Elapsed time %f\n", fElapsedTime);
	PIXEndNamedEvent();
}

bool MinecraftServer::IsSuspending()
{
	return m_suspending;
}

void MinecraftServer::stopServer(bool didInit)
{

	// 4J-PB - need to halt the rendering of the data, since we're about to remove it
#ifdef __PS3__
	if( ShutdownManager::ShouldRun(ShutdownManager::eServerThread )	)		// This thread will take itself out if we are shutting down
#endif
	{
		Minecraft::GetInstance()->gameRenderer->DisableUpdateThread();
	}

	connection->stop();

	app.DebugPrintf("Stopping server\n");
	//    logger.info("Stopping server");
	// 4J-PB - If the primary player has signed out, then don't attempt to save anything

	// also need to check for a profile switch here - primary player signs out, and another player signs in before dismissing the dash
#ifdef _DURANGO
	// On Durango check if the primary user is signed in OR mid-sign-out
	if(ProfileManager.GetUser(0, true) != nullptr)
#else
	if((m_bPrimaryPlayerSignedOut==false) && ProfileManager.IsSignedIn(ProfileManager.GetPrimaryPad()))
#endif
	{
#if defined(_XBOX_ONE) || defined(__ORBIS__)
		// Always save on exit! Except if saves are disabled.
		if(!saveOnExitAnswered()) m_saveOnExit = true;
#endif
		// if trial version or saving is disabled, then don't save anything. Also don't save anything if we didn't actually get through the server initialisation.
		if(m_saveOnExit && ProfileManager.IsFullVersion() && (!StorageManager.GetSaveDisabled()) && didInit)
		{
			if (players != NULL)
			{
				players->saveAll(Minecraft::GetInstance()->progressRenderer, true);
			}
			// 4J Stu - Save the levels in reverse order so we don't overwrite the level.dat
			// with the data from the nethers leveldata.
			// Fix for #7418 - Functional: Gameplay: Saving after sleeping in a bed will place player at nighttime when restarting.
			//for (unsigned int i = levels.length - 1; i >= 0; i--)
			//{
			//	ServerLevel *level = levels[i];
			//	if (level != NULL)
			//	{
			saveAllChunks();
			//	}
			//}

			saveGameRules();
			app.m_gameRules.unloadCurrentGameRules();
			if( levels[0] != NULL )		// This can be null if stopServer happens very quickly due to network error
			{
				levels[0]->saveToDisc(Minecraft::GetInstance()->progressRenderer, false);
			}
		}
	}
	// reset the primary player signout flag
	m_bPrimaryPlayerSignedOut=false;
	s_bServerHalted = false;

	// On Durango/Orbis, we need to wait for all the asynchronous saving processes to complete before destroying the levels, as that will ultimately delete
	// the directory level storage & therefore the ConsoleSaveSplit instance, which needs to be around until all the sub files have completed saving.
#if defined(_DURANGO) || defined(__ORBIS__) || defined(__PSVITA__)
	while(StorageManager.GetSaveState() != C4JStorage::ESaveGame_Idle )
	{
		Sleep(10);
	}
#endif

	// 4J-PB remove the server levels
	unsigned int iServerLevelC=levels.length;
	for (unsigned int i = 0; i < iServerLevelC; i++)
	{
		if(levels[i]!=NULL)
		{
			delete levels[i];
			levels[i] = NULL;
		}
	}

#if defined(__PS3__) || defined(__ORBIS__)
	// Clear the update flags as it's possible they could be out of sync, causing a crash when starting a new world after the first new level ticks
	// Fix for PS3 #1538 - [IN GAME] If the user 'Exit without saving' from inside the Nether or The End, the title can hang when loading back into the save.
#endif

	delete connection;
	connection = NULL;
	delete players;
	players = NULL;
	delete settings;
	settings = NULL;

	g_NetworkManager.ServerStopped();
}

void MinecraftServer::halt()
{
	running = false;
}

void MinecraftServer::setMaxBuildHeight(int maxBuildHeight)
{
	this->maxBuildHeight = maxBuildHeight;
}

int MinecraftServer::getMaxBuildHeight()
{
	return maxBuildHeight;
}

PlayerList *MinecraftServer::getPlayers()
{
	return players;
}

void MinecraftServer::setPlayers(PlayerList *players)
{
	this->players = players;
}

ServerConnection *MinecraftServer::getConnection()
{
	return connection;
}

bool MinecraftServer::isAnimals()
{
	return animals;
}

void MinecraftServer::setAnimals(bool animals)
{
	this->animals = animals;
}

bool MinecraftServer::isNpcsEnabled()
{
	return npcs;
}

void MinecraftServer::setNpcsEnabled(bool npcs)
{
	this->npcs = npcs;
}

bool MinecraftServer::isPvpAllowed()
{
	return pvp;
}

void MinecraftServer::setPvpAllowed(bool pvp)
{
	this->pvp = pvp;
}

bool MinecraftServer::isFlightAllowed()
{
	return allowFlight;
}

void MinecraftServer::setFlightAllowed(bool allowFlight)
{
	this->allowFlight = allowFlight;
}

bool MinecraftServer::isCommandBlockEnabled()
{
	return false; //settings.getBoolean("enable-command-block", false);
}

bool MinecraftServer::isNetherEnabled()
{
	return true; //settings.getBoolean("allow-nether", true);
}

bool MinecraftServer::isHardcore()
{
	return false;
}

int MinecraftServer::getOperatorUserPermissionLevel()
{
	return Command::LEVEL_OWNERS; //settings.getInt("op-permission-level", Command.LEVEL_OWNERS);
}

CommandDispatcher *MinecraftServer::getCommandDispatcher()
{
	return commandDispatcher;
}

Pos *MinecraftServer::getCommandSenderWorldPosition()
{
	return new Pos(0, 0, 0);
}

Level *MinecraftServer::getCommandSenderWorld()
{
	return levels[0];
}

int MinecraftServer::getSpawnProtectionRadius()
{
	return 16;
}

bool MinecraftServer::isUnderSpawnProtection(Level *level, int x, int y, int z, shared_ptr<Player> player)
{
	if (level->dimension->id != 0) return false;
	//if (getPlayers()->getOps()->empty()) return false;
	if (getPlayers()->isOp(player->getName())) return false;
	if (getSpawnProtectionRadius() <= 0) return false;

	Pos *spawnPos = level->getSharedSpawnPos();
	int xd = Mth::abs(x - spawnPos->x);
	int zd = Mth::abs(z - spawnPos->z);
	int dist = max(xd, zd);

	return dist <= getSpawnProtectionRadius();
}

void MinecraftServer::setForceGameType(bool forceGameType)
{
	this->forceGameType = forceGameType;
}

bool MinecraftServer::getForceGameType()
{
	return forceGameType;
}

int64_t MinecraftServer::getCurrentTimeMillis()
{
	return System::currentTimeMillis();
}

int MinecraftServer::getPlayerIdleTimeout()
{
	return playerIdleTimeout;
}

void MinecraftServer::setPlayerIdleTimeout(int playerIdleTimeout)
{
	this->playerIdleTimeout = playerIdleTimeout;
}

extern int c0a, c0b, c1a, c1b, c1c, c2a, c2b;
void MinecraftServer::run(int64_t seed, void *lpParameter)
{
	NetworkGameInitData *initData = NULL;
	DWORD initSettings = 0;
	bool findSeed = false;
	if(lpParameter != NULL)
	{
		initData = (NetworkGameInitData *)lpParameter;
		initSettings = app.GetGameHostOption(eGameHostOption_All);
		findSeed = initData->findSeed;
		m_texturePackId = initData->texturePackId;
	}
	//    try {		// 4J - removed try/catch/finally
	bool didInit = false;
	if (initServer(seed, initData, initSettings,findSeed))
	{
		didInit = true;
		ServerLevel *levelNormalDimension = levels[0];
		// 4J-PB - Set the Stronghold position in the leveldata if there isn't one in there
		Minecraft *pMinecraft = Minecraft::GetInstance();
		LevelData *pLevelData=levelNormalDimension->getLevelData();

		if(pLevelData && pLevelData->getHasStronghold()==false)
		{
			int x,z;
			if(app.GetTerrainFeaturePosition(eTerrainFeature_Stronghold,&x,&z))
			{
				pLevelData->setXStronghold(x);
				pLevelData->setZStronghold(z);
				pLevelData->setHasStronghold();
			}
		}

		int64_t lastTime = getCurrentTimeMillis();
		int64_t unprocessedTime = 0;
		while (running && !s_bServerHalted)
		{
			int64_t now = getCurrentTimeMillis();

			// 4J Stu - When we pause the server, we don't want to count that as time passed
			// 4J Stu - TU-1 hotifx - Remove this line. We want to make sure that we tick connections at the proper rate when paused
			//Fix for #13191 - The host of a game can get a message informing them that the connection to the server has been lost
			//if(m_isServerPaused) lastTime = now;

			int64_t passedTime = now - lastTime;
			if (passedTime > MS_PER_TICK * 40)
			{
				//                logger.warning("Can't keep up! Did the system time change, or is the server overloaded?");
				passedTime = MS_PER_TICK * 40;
			}
			if (passedTime < 0)
			{
				//                logger.warning("Time ran backwards! Did the system time change?");
				passedTime = 0;
			}
			unprocessedTime += passedTime;
			lastTime = now;

			// 4J Added ability to pause the server
			if( !m_isServerPaused )
			{
				bool didTick = false;
				if (levels[0]->allPlayersAreSleeping())
				{
					tick();
					unprocessedTime = 0;
				}
				else
				{
					//					int tickcount = 0;
					//					int64_t beforeall = System::currentTimeMillis();
					while (unprocessedTime > MS_PER_TICK)
					{
						unprocessedTime -= MS_PER_TICK;
						chunkPacketManagement_PreTick();
//						int64_t before = System::currentTimeMillis();
						tick();
//						int64_t after = System::currentTimeMillis();
//						PIXReportCounter(L"Server time",(float)(after-before));

						chunkPacketManagement_PostTick();
					}
//					int64_t afterall = System::currentTimeMillis();
//					PIXReportCounter(L"Server time all",(float)(afterall-beforeall));
//					PIXReportCounter(L"Server ticks",(float)tickcount);
				}
			}
			else
			{
				// 4J Stu - TU1-hotfix
				//Fix for #13191 - The host of a game can get a message informing them that the connection to the server has been lost
				// The connections should tick at the same frequency even when paused
				while (unprocessedTime > MS_PER_TICK)
				{
					unprocessedTime -= MS_PER_TICK;
					// Keep ticking the connections to stop them timing out
					connection->tick();
				}
			}
			if(MinecraftServer::setTimeAtEndOfTick)
			{
				MinecraftServer::setTimeAtEndOfTick = false;
				for (unsigned int i = 0; i < levels.length; i++)
				{
					//					if (i == 0 || settings->getBoolean(L"allow-nether", true))		// 4J removed - we always have nether
					{
						ServerLevel *level = levels[i];
						level->setGameTime( MinecraftServer::setTime );
					}
				}
			}
			if(MinecraftServer::setTimeOfDayAtEndOfTick)
			{
				MinecraftServer::setTimeOfDayAtEndOfTick = false;
				for (unsigned int i = 0; i < levels.length; i++)
				{
					if (i == 0 || GetDedicatedServerBool(settings, L"allow-nether", true))
					{
						ServerLevel *level = levels[i];
						level->setDayTime( MinecraftServer::setTimeOfDay );
					}
				}
			}

			// Process delayed actions
			eXuiServerAction eAction;
			LPVOID param;
			for(int i=0;i<XUSER_MAX_COUNT;i++)
			{
				eAction = app.GetXuiServerAction(i);
				param = app.GetXuiServerActionParam(i);

				switch(eAction)
				{
				case eXuiServerAction_AutoSaveGame:
#if defined(_XBOX_ONE) || defined(__ORBIS__)
					{
						PIXBeginNamedEvent(0,"Autosave");

						// Get the frequency of the timer
						LARGE_INTEGER qwTicksPerSec, qwTime, qwNewTime, qwDeltaTime;
						float fElapsedTime = 0.0f;
						QueryPerformanceFrequency( &qwTicksPerSec );
						float fSecsPerTick = 1.0f / (float)qwTicksPerSec.QuadPart;

						// Save the start time
						QueryPerformanceCounter( &qwTime );

						if (players != NULL)
						{
							players->saveAll(NULL);
						}

						for (unsigned int j = 0; j < levels.length; j++)
						{
							if( s_bServerHalted ) break;
							// 4J Stu - Save the levels in reverse order so we don't overwrite the level.dat
							// with the data from the nethers leveldata.
							// Fix for #7418 - Functional: Gameplay: Saving after sleeping in a bed will place player at nighttime when restarting.
							ServerLevel *level = levels[levels.length - 1 - j];
							PIXBeginNamedEvent(0, "Saving level %d",levels.length - 1 - j);
							level->save(false, NULL, true);
							PIXEndNamedEvent();
						}
						if( !s_bServerHalted )
						{
							PIXBeginNamedEvent(0,"Saving game rules");
							saveGameRules();
							PIXEndNamedEvent();

							PIXBeginNamedEvent(0,"Save to disc");
							levels[0]->saveToDisc(Minecraft::GetInstance()->progressRenderer, true);
							PIXEndNamedEvent();
						}
						PIXEndNamedEvent();

						QueryPerformanceCounter( &qwNewTime );
						qwDeltaTime.QuadPart = qwNewTime.QuadPart - qwTime.QuadPart;
						fElapsedTime = fSecsPerTick * ((FLOAT)(qwDeltaTime.QuadPart));
						app.DebugPrintf("Autosave: Elapsed time %f\n", fElapsedTime);
					}
					break;
#endif
				case eXuiServerAction_SaveGame:
					app.EnterSaveNotificationSection();
					if (players != NULL)
					{
						players->saveAll(Minecraft::GetInstance()->progressRenderer);
					}

					players->broadcastAll( shared_ptr<UpdateProgressPacket>( new UpdateProgressPacket(20) ) );

					for (unsigned int j = 0; j < levels.length; j++)
					{
						if( s_bServerHalted ) break;
						// 4J Stu - Save the levels in reverse order so we don't overwrite the level.dat
						// with the data from the nethers leveldata.
						// Fix for #7418 - Functional: Gameplay: Saving after sleeping in a bed will place player at nighttime when restarting.
						ServerLevel *level = levels[levels.length - 1 - j];
						level->save(true, Minecraft::GetInstance()->progressRenderer, (eAction==eXuiServerAction_AutoSaveGame));

						players->broadcastAll( shared_ptr<UpdateProgressPacket>( new UpdateProgressPacket(33 + (j*33) ) ) );
					}
					if( !s_bServerHalted )
					{
						saveGameRules();

						levels[0]->saveToDisc(Minecraft::GetInstance()->progressRenderer, (eAction==eXuiServerAction_AutoSaveGame));
					}
					app.LeaveSaveNotificationSection();
					break;
				case eXuiServerAction_DropItem:
					// Find the player, and drop the id at their feet
					{
						shared_ptr<ServerPlayer> player = players->players.at(0);
						size_t id = (size_t) param;
						player->drop( shared_ptr<ItemInstance>( new ItemInstance(id, 1, 0 ) ) );
					}
					break;
				case eXuiServerAction_SpawnMob:
					{
						shared_ptr<ServerPlayer> player = players->players.at(0);
						eINSTANCEOF factory = (eINSTANCEOF)((size_t)param);
						shared_ptr<Mob> mob = dynamic_pointer_cast<Mob>(EntityIO::newByEnumType(factory,player->level ));
						mob->moveTo(player->x+1, player->y, player->z+1, player->level->random->nextFloat() * 360, 0);
						mob->setDespawnProtected();		// 4J added, default to being protected against despawning (has to be done after initial position is set)
						player->level->addEntity(mob);
					}
					break;
				case eXuiServerAction_PauseServer:
					m_isServerPaused = ( (size_t) param == TRUE );
					if( m_isServerPaused )
					{
						m_serverPausedEvent->Set();
					}
					break;
				case eXuiServerAction_ToggleRain:
					{
						bool isRaining = levels[0]->getLevelData()->isRaining();
						levels[0]->getLevelData()->setRaining(!isRaining);
						levels[0]->getLevelData()->setRainTime(levels[0]->random->nextInt(Level::TICKS_PER_DAY * 7) + Level::TICKS_PER_DAY / 2);
					}
					break;
				case eXuiServerAction_ToggleThunder:
					{
						bool isThundering = levels[0]->getLevelData()->isThundering();
						levels[0]->getLevelData()->setThundering(!isThundering);
						levels[0]->getLevelData()->setThunderTime(levels[0]->random->nextInt(Level::TICKS_PER_DAY * 7) + Level::TICKS_PER_DAY / 2);
					}
					break;
				case eXuiServerAction_ServerSettingChanged_Gamertags:
					players->broadcastAll( shared_ptr<ServerSettingsChangedPacket>( new ServerSettingsChangedPacket( ServerSettingsChangedPacket::HOST_OPTIONS, app.GetGameHostOption(eGameHostOption_Gamertags)) ) );
					break;
				case eXuiServerAction_ServerSettingChanged_BedrockFog:
					players->broadcastAll( shared_ptr<ServerSettingsChangedPacket>( new ServerSettingsChangedPacket( ServerSettingsChangedPacket::HOST_IN_GAME_SETTINGS, app.GetGameHostOption(eGameHostOption_All)) ) );
					break;

				case eXuiServerAction_ServerSettingChanged_Difficulty:
					players->broadcastAll( shared_ptr<ServerSettingsChangedPacket>( new ServerSettingsChangedPacket( ServerSettingsChangedPacket::HOST_DIFFICULTY, Minecraft::GetInstance()->options->difficulty) ) );
					break;
				case eXuiServerAction_ExportSchematic:
#ifndef _CONTENT_PACKAGE
					app.EnterSaveNotificationSection();

					//players->broadcastAll( shared_ptr<UpdateProgressPacket>( new UpdateProgressPacket(20) ) );

					if( !s_bServerHalted )
					{
						ConsoleSchematicFile::XboxSchematicInitParam *initData = (ConsoleSchematicFile::XboxSchematicInitParam *)param;
#ifdef _XBOX
						File targetFileDir(File::pathRoot + File::pathSeparator + L"Schematics");
#else
						File targetFileDir(L"Schematics");
#endif
						if(!targetFileDir.exists())	targetFileDir.mkdir();

						wchar_t filename[128];
						swprintf(filename,128,L"%ls%dx%dx%d.sch",initData->name,(initData->endX - initData->startX + 1), (initData->endY - initData->startY + 1), (initData->endZ - initData->startZ + 1));

						File dataFile = File( targetFileDir, wstring(filename) );
						if(dataFile.exists()) dataFile._delete();
						FileOutputStream fos = FileOutputStream(dataFile);
						DataOutputStream dos = DataOutputStream(&fos);
						ConsoleSchematicFile::generateSchematicFile(&dos, levels[0], initData->startX, initData->startY, initData->startZ, initData->endX, initData->endY, initData->endZ, initData->bSaveMobs, initData->compressionType);
						dos.close();

						delete initData;
					}
					app.LeaveSaveNotificationSection();
#endif
					break;
				case eXuiServerAction_SetCameraLocation:
#ifndef _CONTENT_PACKAGE
					{
						DebugSetCameraPosition *pos = (DebugSetCameraPosition *)param;

						app.DebugPrintf(	"DEBUG: Player=%i\n", pos->player );
						app.DebugPrintf(	"DEBUG: Teleporting to pos=(%f.2, %f.2, %f.2), looking at=(%f.2,%f.2)\n",
							pos->m_camX, pos->m_camY, pos->m_camZ,
							pos->m_yRot, pos->m_elev
							);

						shared_ptr<ServerPlayer> player = players->players.at(pos->player);
						player->debug_setPosition(	pos->m_camX, pos->m_camY, pos->m_camZ,
							pos->m_yRot, pos->m_elev	);

						// Doesn't work
						//player->setYHeadRot(pos->m_yRot);
						//player->absMoveTo(pos->m_camX, pos->m_camY, pos->m_camZ, pos->m_yRot, pos->m_elev);
					}
#endif
					break;
				}

				app.SetXuiServerAction(i,eXuiServerAction_Idle);
			}

			Sleep(1);
		}
	}
	//else
	//{
	//     while (running)
	//	{
	//        handleConsoleInputs();
	//		Sleep(10);
	//    }
	//}
#if 0
} catch (Throwable t) {
	t.printStackTrace();
	logger.log(Level.SEVERE, "Unexpected exception", t);
	while (running) {
		handleConsoleInputs();
		try {
			Thread.sleep(10);
		} catch (InterruptedException e1) {
			e1.printStackTrace();
		}
	}
} finally {
	try {
		stopServer();
		stopped = true;
	} catch (Throwable t) {
		t.printStackTrace();
	} finally {
		System::exit(0);
	}
}
#endif

	// 4J Stu - Stop the server when the loops complete, as the finally would do
	stopServer(didInit);
	stopped = true;
}

void MinecraftServer::broadcastStartSavingPacket()
{
	players->broadcastAll( shared_ptr<GameEventPacket>( new GameEventPacket(GameEventPacket::START_SAVING, 0) ) );;
}

void MinecraftServer::broadcastStopSavingPacket()
{
	if( !s_bServerHalted )
	{
		players->broadcastAll( shared_ptr<GameEventPacket>( new GameEventPacket(GameEventPacket::STOP_SAVING, 0) ) );;
	}
}

void MinecraftServer::tick()
{
	vector<wstring> toRemove;
    for ( auto& it : ironTimers )
    {
		int t = it.second;
		if (t > 0)
		{
			ironTimers[it.first] = t - 1;
		}
		else
		{
			toRemove.push_back(it.first);
		}
	}
	for (const auto& i : toRemove)
	{
		ironTimers.erase(i);
	}

	AABB::resetPool();
	Vec3::resetPool();

	tickCount++;

	// 4J We need to update client difficulty levels based on the servers
	Minecraft *pMinecraft = Minecraft::GetInstance();
	// 4J-PB - sending this on the host changing the difficulty in the menus
	/*	if(m_lastSentDifficulty != pMinecraft->options->difficulty)
	{
	m_lastSentDifficulty = pMinecraft->options->difficulty;
	players->broadcastAll( shared_ptr<ServerSettingsChangedPacket>( new ServerSettingsChangedPacket( ServerSettingsChangedPacket::HOST_DIFFICULTY, pMinecraft->options->difficulty) ) );
	}*/

	for (unsigned int i = 0; i < levels.length; i++)
	{
		//        if (i == 0 || settings->getBoolean(L"allow-nether", true))		// 4J removed - we always have nether
		{
			ServerLevel *level = levels[i];

			// 4J Stu - We set the levels difficulty based on the minecraft options
			level->difficulty = app.GetGameHostOption(eGameHostOption_Difficulty); //pMinecraft->options->difficulty;

#if DEBUG_SERVER_DONT_SPAWN_MOBS
			level->setSpawnSettings(false, false);
#else
			level->setSpawnSettings(level->difficulty > 0 && !Minecraft::GetInstance()->isTutorial(), animals);
#endif

			if (tickCount % 20 == 0)
			{
				players->broadcastAll( shared_ptr<SetTimePacket>( new SetTimePacket(level->getGameTime(), level->getDayTime(), level->getGameRules()->getBoolean(GameRules::RULE_DAYLIGHT) ) ), level->dimension->id);
			}
			// #ifndef __PS3__
			static int64_t stc = 0;
			int64_t st0 = System::currentTimeMillis();
			PIXBeginNamedEvent(0,"Level tick %d",i);
			((Level *)level)->tick();
			int64_t st1 = System::currentTimeMillis();
			PIXEndNamedEvent();
			PIXBeginNamedEvent(0,"Update lights %d",i);

			int64_t st2 = System::currentTimeMillis();
			PIXEndNamedEvent();
			PIXBeginNamedEvent(0,"Entity tick %d",i);
			// 4J added to stop ticking entities in levels when players are not in those levels.
			// Note: now changed so that we also tick if there are entities to be removed, as this also happens as a result of calling tickEntities. If we don't do this, then the
			// entities get removed at the first point that there is a player count in the level - this has been causing a problem when going from normal dimension -> nether -> normal,
			// as the player is getting flagged as to be removed (from the normal dimension) when going to the nether, but Actually gets removed only when it returns
			if( ( players->getPlayerCount(level) > 0) || ( level->hasEntitiesToRemove() ) )
			{
#ifdef __PSVITA__
				// AP - the PlayerList->viewDistance initially starts out at 3 to make starting a level speedy
				// the problem with this is that spawned monsters are always generated on the edge of the known map
				// which means they wont process (unless they are surrounded by 2 visible chunks). This means
				// they wont checkDespawn so they are NEVER removed which results in monsters not spawning.
				// This bit of hack will modify the view distance once the level is up and running.
				int newViewDistance = 5;
				level->getServer()->getPlayers()->setViewDistance(newViewDistance);
				level->getTracker()->updateMaxRange();
				level->getChunkMap()->setRadius(level->getServer()->getPlayers()->getViewDistance());
#endif
				level->tickEntities();
			}
			PIXEndNamedEvent();

			PIXBeginNamedEvent(0,"Entity tracker tick");
			level->getTracker()->tick();
			PIXEndNamedEvent();

			int64_t st3 = System::currentTimeMillis();
			//			printf(">>>>>>>>>>>>>>>>>>>>>> Tick %d %d %d : %d\n", st1 - st0, st2 - st1, st3 - st2, st0 - stc );
			stc = st0;
			// #endif// __PS3__
		}
	}
	Entity::tickExtraWandering();	// 4J added

	PIXBeginNamedEvent(0,"Connection tick");
	connection->tick();
	PIXEndNamedEvent();
	PIXBeginNamedEvent(0,"Players tick");
	players->tick();
	PIXEndNamedEvent();

	// 4J - removed
#if 0
	for (int i = 0; i < tickables.size(); i++) {
		tickables.get(i)-tick();
	}
#endif

	//    try {		// 4J - removed try/catch
	handleConsoleInputs();
	//    } catch (Exception e) {
	//        logger.log(Level.WARNING, "Unexpected exception while parsing console command", e);
	//    }
}

void MinecraftServer::handleConsoleInput(const wstring& msg, ConsoleInputSource *source)
{
	EnterCriticalSection(&m_consoleInputCS);
	consoleInput.push_back(new ConsoleInput(msg, source));
	LeaveCriticalSection(&m_consoleInputCS);
}

void MinecraftServer::handleConsoleInputs()
{
	vector<ConsoleInput *> pendingInputs;
	EnterCriticalSection(&m_consoleInputCS);
	pendingInputs.swap(consoleInput);
	LeaveCriticalSection(&m_consoleInputCS);

	for (size_t i = 0; i < pendingInputs.size(); ++i)
	{
		ConsoleInput *input = pendingInputs[i];
		ExecuteConsoleCommand(this, input->msg);
		delete input;
	}
}

void MinecraftServer::main(int64_t seed, void *lpParameter)
{
#if __PS3__
	ShutdownManager::HasStarted(ShutdownManager::eServerThread );
#endif
	server = new MinecraftServer();
	server->run(seed, lpParameter);
	delete server;
	server = NULL;
	ShutdownManager::HasFinished(ShutdownManager::eServerThread );
}

void MinecraftServer::HaltServer(bool bPrimaryPlayerSignedOut)
{
	s_bServerHalted = true;
	if( server != NULL )
	{
		m_bPrimaryPlayerSignedOut=bPrimaryPlayerSignedOut;
		server->halt();
	}
}

File *MinecraftServer::getFile(const wstring& name)
{
	return new File(name);
}

void MinecraftServer::info(const wstring& string)
{
	PrintConsoleLine(L"[INFO] ", string);
}

void MinecraftServer::warn(const wstring& string)
{
	PrintConsoleLine(L"[WARN] ", string);
}

wstring MinecraftServer::getConsoleName()
{
	return L"CONSOLE";
}

ServerLevel *MinecraftServer::getLevel(int dimension)
{
	if (dimension == -1) return levels[1];
	else if (dimension == 1) return levels[2];
	else return levels[0];
}

// 4J added
void MinecraftServer::setLevel(int dimension, ServerLevel *level)
{
	if (dimension == -1) levels[1] = level;
	else if (dimension == 1) levels[2] = level;
	else levels[0] = level;
}

#if defined _ACK_CHUNK_SEND_THROTTLING
bool MinecraftServer::chunkPacketManagement_CanSendTo(INetworkPlayer *player)
{
	if( s_hasSentEnoughPackets ) return false;
	if( player == NULL ) return false;

	for( int i = 0; i < s_sentTo.size(); i++ )
	{
		if( s_sentTo[i]->IsSameSystem(player) )
		{
			return false;
		}
	}

#if defined(__PS3__) || defined(__ORBIS__) || defined(__PSVITA__)
	return ( player->GetOutstandingAckCount() < 3 );
#else
	return ( player->GetOutstandingAckCount() < 2 );
#endif
}

void MinecraftServer::chunkPacketManagement_DidSendTo(INetworkPlayer *player)
{
	int64_t currentTime = System::currentTimeMillis();

	if( ( currentTime - s_tickStartTime ) >= MAX_TICK_TIME_FOR_PACKET_SENDS )
	{
		s_hasSentEnoughPackets = true;
//		app.DebugPrintf("Sending, setting enough packet flag: %dms\n",currentTime - s_tickStartTime);
	}
	else
	{
//		app.DebugPrintf("Sending, more time: %dms\n",currentTime - s_tickStartTime);
	}

	player->SentChunkPacket();

	s_sentTo.push_back(player);
}

void MinecraftServer::chunkPacketManagement_PreTick()
{
//	app.DebugPrintf("*************************************************************************************************************************************************************************\n");
	s_hasSentEnoughPackets = false;
	s_tickStartTime = System::currentTimeMillis();
	s_sentTo.clear();

	vector< shared_ptr<PlayerConnection> > *players = connection->getPlayers();

	if( players->size() )
	{
		vector< shared_ptr<PlayerConnection> > playersOrig = *players;
		players->clear();

		do
		{
			int longestTime = 0;
			auto playerConnectionBest = playersOrig.begin();
			for( auto it = playersOrig.begin(); it != playersOrig.end(); it++)
			{
				int thisTime = 0;
				INetworkPlayer *np = (*it)->getNetworkPlayer();
				if( np )
				{
					thisTime = np->GetTimeSinceLastChunkPacket_ms();
				}

				if( thisTime > longestTime )
				{
					playerConnectionBest = it;
					longestTime = thisTime;
				}
			}
			players->push_back(*playerConnectionBest);
			playersOrig.erase(playerConnectionBest);
		} while ( playersOrig.size() > 0 );
	}
}

void MinecraftServer::chunkPacketManagement_PostTick()
{
}

#else
// 4J Added - round-robin chunk sends by player index. Compare vs the player at the current queue index,
// not GetSessionIndex() (smallId), so reused smallIds after many connect/disconnects still get chunk sends.
bool MinecraftServer::chunkPacketManagement_CanSendTo(INetworkPlayer *player)
{
	if( player == NULL ) return false;

	int time = GetTickCount();
	DWORD currentPlayerCount = g_NetworkManager.GetPlayerCount();
	if( currentPlayerCount == 0 ) return false;
	int index = s_slowQueuePlayerIndex % (int)currentPlayerCount;
	INetworkPlayer *queuePlayer = g_NetworkManager.GetPlayerByIndex( index );
	if( queuePlayer != NULL && (player == queuePlayer || player->IsSameSystem(queuePlayer)) && (time - s_slowQueueLastTime) > MINECRAFT_SERVER_SLOW_QUEUE_DELAY )
	{
		return true;
	}

	return false;
}

void MinecraftServer::chunkPacketManagement_DidSendTo(INetworkPlayer *player)
{
	s_slowQueuePacketSent = true;
}

void MinecraftServer::chunkPacketManagement_PreTick()
{
}

void MinecraftServer::chunkPacketManagement_PostTick()
{
	// 4J Ensure that the slow queue owner keeps cycling if it's not been used in a while
	int time = GetTickCount();
	if( ( s_slowQueuePacketSent ) || (  (time - s_slowQueueLastTime) > ( 2 * MINECRAFT_SERVER_SLOW_QUEUE_DELAY ) ) )
	{
//		app.DebugPrintf("Considering cycling: (%d) %d - %d -> %d > %d\n",s_slowQueuePacketSent, time, s_slowQueueLastTime, (time - s_slowQueueLastTime), (2*MINECRAFT_SERVER_SLOW_QUEUE_DELAY));
		MinecraftServer::cycleSlowQueueIndex();
		s_slowQueuePacketSent = false;
		s_slowQueueLastTime = time;
	}
//	else
//	{
//		app.DebugPrintf("Not considering cycling: %d - %d -> %d > %d\n",time, s_slowQueueLastTime, (time - s_slowQueueLastTime), (2*MINECRAFT_SERVER_SLOW_QUEUE_DELAY));
//	}
}

void MinecraftServer::cycleSlowQueueIndex()
{
	if( !g_NetworkManager.IsInSession() ) return;

	int startingIndex = s_slowQueuePlayerIndex;
	INetworkPlayer *currentPlayer = NULL;
	DWORD currentPlayerCount = 0;
	do
	{
		currentPlayerCount = g_NetworkManager.GetPlayerCount();
		if( startingIndex >= currentPlayerCount ) startingIndex = 0;
		++s_slowQueuePlayerIndex;

		if( currentPlayerCount > 0 )
		{
			s_slowQueuePlayerIndex %= currentPlayerCount;
			// Fix for #9530 - NETWORKING: Attempting to fill a multiplayer game beyond capacity results in a softlock for the last players to join.
			// The QNet session might be ending while we do this, so do a few more checks that the player is real
			currentPlayer = g_NetworkManager.GetPlayerByIndex( s_slowQueuePlayerIndex );
		}
		else
		{
			s_slowQueuePlayerIndex = 0;
		}
	} while ( g_NetworkManager.IsInSession() &&
		currentPlayerCount > 0 &&
		s_slowQueuePlayerIndex != startingIndex &&
		currentPlayer != NULL &&
		currentPlayer->IsLocal()
		);
//	app.DebugPrintf("Cycled slow queue index to %d\n", s_slowQueuePlayerIndex);
}
#endif

// 4J added - sets up a vector of flags to indicate which entities (with small Ids) have been removed from the level, but are still haven't constructed a network packet
// to tell a remote client about it. These small Ids shouldn't be re-used. Most of the time this method shouldn't actually do anything, in which case it will return false
// and nothing is set up.
bool MinecraftServer::flagEntitiesToBeRemoved(unsigned int *flags)
{
	bool removedFound = false;
	for( unsigned int i = 0; i < levels.length; i++ )
	{
		ServerLevel *level = levels[i];
		if( level )
		{
			level->flagEntitiesToBeRemoved( flags, &removedFound );
		}
	}
	return removedFound;
}
