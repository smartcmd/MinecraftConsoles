// Minecraft.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include <assert.h>
#include <iostream>
#include <ShellScalingApi.h>
#include <shellapi.h>
#include "GameConfig\Minecraft.spa.h"
#include "..\MinecraftServer.h"
#include "..\LocalPlayer.h"
#include "..\..\Minecraft.World\ItemInstance.h"
#include "..\..\Minecraft.World\MapItem.h"
#include "..\..\Minecraft.World\Recipes.h"
#include "..\..\Minecraft.World\Recipy.h"
#include "..\..\Minecraft.World\Language.h"
#include "..\..\Minecraft.World\StringHelpers.h"
#include "..\..\Minecraft.World\AABB.h"
#include "..\..\Minecraft.World\Vec3.h"
#include "..\..\Minecraft.World\Level.h"
#include "..\..\Minecraft.World\net.minecraft.world.level.tile.h"

#include "..\ClientConnection.h"
#include "..\User.h"
#include "..\..\Minecraft.World\Socket.h"
#include "..\..\Minecraft.World\ThreadName.h"
#include "..\..\Minecraft.Client\StatsCounter.h"
#include "..\ConnectScreen.h"
//#include "Social\SocialManager.h"
//#include "Leaderboards\LeaderboardManager.h"
//#include "XUI\XUI_Scene_Container.h"
//#include "NetworkManager.h"
#include "..\..\Minecraft.Client\Tesselator.h"
#include "..\..\Minecraft.Client\Options.h"
#include "Sentient\SentientManager.h"
#include "..\..\Minecraft.World\IntCache.h"
#include "..\Textures.h"
#include "..\Settings.h"
#include "Resource.h"
#include "..\..\Minecraft.World\compression.h"
#include "..\..\Minecraft.World\OldChunkStorage.h"
#include "Network\WinsockNetLayer.h"

#include "Xbox/resource.h"

#ifdef _MSC_VER
#pragma comment(lib, "legacy_stdio_definitions.lib")
#endif

HINSTANCE hMyInst;
LRESULT CALLBACK DlgProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
char chGlobalText[256];
uint16_t ui16GlobalText[256];

#define THEME_NAME		"584111F70AAAAAAA"
#define THEME_FILESIZE	2797568

//#define THREE_MB 3145728 // minimum save size (checking for this on a selected device)
//#define FIVE_MB 5242880 // minimum save size (checking for this on a selected device)
//#define FIFTY_TWO_MB (1024*1024*52) // Maximum TCR space required for a save (checking for this on a selected device)
#define FIFTY_ONE_MB (1000000*51) // Maximum TCR space required for a save is 52MB (checking for this on a selected device)

//#define PROFILE_VERSION 3 // new version for the interim bug fix 166 TU
#define NUM_PROFILE_VALUES	5
#define NUM_PROFILE_SETTINGS 4
DWORD dwProfileSettingsA[NUM_PROFILE_VALUES]=
{
#ifdef _XBOX
	XPROFILE_OPTION_CONTROLLER_VIBRATION,
	XPROFILE_GAMER_YAXIS_INVERSION,
	XPROFILE_GAMER_CONTROL_SENSITIVITY,
	XPROFILE_GAMER_ACTION_MOVEMENT_CONTROL,
	XPROFILE_TITLE_SPECIFIC1,
#else
	0,0,0,0,0
#endif
};

//-------------------------------------------------------------------------------------
// Time             Since fAppTime is a float, we need to keep the quadword app time
//                  as a LARGE_INTEGER so that we don't lose precision after running
//                  for a long time.
//-------------------------------------------------------------------------------------

BOOL g_bWidescreen = TRUE;

int g_iScreenWidth = 1920;
int g_iScreenHeight = 1080;

float g_iAspectRatio = static_cast<float>(g_iScreenWidth) / g_iScreenHeight;

char g_Win64Username[17] = { 0 };
wchar_t g_Win64UsernameW[17] = { 0 };

// Fullscreen toggle state
static bool g_isFullscreen = false;
static WINDOWPLACEMENT g_wpPrev = { sizeof(g_wpPrev) };

//--------------------------------------------------------------------------------------
// Update the Aspect Ratio to support Any Aspect Ratio
//--------------------------------------------------------------------------------------
void UpdateAspectRatio(int width, int height)
{
	g_iAspectRatio = static_cast<float>(width) / height;
}

struct Win64LaunchOptions
{
	int screenMode;
	bool serverMode;
};

static void CopyWideArgToAnsi(LPCWSTR source, char* dest, size_t destSize)
{
	if (destSize == 0)
		return;

	dest[0] = 0;
	if (source == NULL)
		return;

	WideCharToMultiByte(CP_ACP, 0, source, -1, dest, (int)destSize, NULL, NULL);
	dest[destSize - 1] = 0;
}

// ---------- Persistent options (options.txt next to exe) ----------
static void GetOptionsFilePath(char *out, size_t outSize)
{
	GetModuleFileNameA(NULL, out, (DWORD)outSize);
	char *p = strrchr(out, '\\');
	if (p) *(p + 1) = '\0';
	strncat_s(out, outSize, "options.txt", _TRUNCATE);
}

static void SaveFullscreenOption(bool fullscreen)
{
	char path[MAX_PATH];
	GetOptionsFilePath(path, sizeof(path));
	FILE *f = nullptr;
	if (fopen_s(&f, path, "w") == 0 && f)
	{
		fprintf(f, "fullscreen=%d\n", fullscreen ? 1 : 0);
		fclose(f);
	}
}

static bool LoadFullscreenOption()
{
	char path[MAX_PATH];
	GetOptionsFilePath(path, sizeof(path));
	FILE *f = nullptr;
	if (fopen_s(&f, path, "r") == 0 && f)
	{
		char line[256];
		while (fgets(line, sizeof(line), f))
		{
			int val = 0;
			if (sscanf_s(line, "fullscreen=%d", &val) == 1)
			{
				fclose(f);
				return val != 0;
			}
		}
		fclose(f);
	}
	return false;
}
// ------------------------------------------------------------------

static void ApplyScreenMode(int screenMode)
{
	switch (screenMode)
	{
	case 1:
		g_iScreenWidth = 1280;
		g_iScreenHeight = 720;
		break;
	case 2:
		g_iScreenWidth = 640;
		g_iScreenHeight = 480;
		break;
	case 3:
		g_iScreenWidth = 720;
		g_iScreenHeight = 408;
		break;
	default:
		break;
	}
}

static Win64LaunchOptions ParseLaunchOptions()
{
	Win64LaunchOptions options = {};
	options.screenMode = 0;
	options.serverMode = false;

	g_Win64MultiplayerJoin = false;
	g_Win64MultiplayerPort = WIN64_NET_DEFAULT_PORT;
	g_Win64DedicatedServer = false;
	g_Win64DedicatedServerPort = WIN64_NET_DEFAULT_PORT;
	g_Win64DedicatedServerBindIP[0] = 0;

	int argc = 0;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	if (argv == NULL)
		return options;

	if (argc > 1 && lstrlenW(argv[1]) == 1)
	{
		if (argv[1][0] >= L'1' && argv[1][0] <= L'3')
			options.screenMode = argv[1][0] - L'0';
	}

	for (int i = 1; i < argc; ++i)
	{
		if (_wcsicmp(argv[i], L"-server") == 0)
		{
			options.serverMode = true;
			break;
		}
	}

	g_Win64DedicatedServer = options.serverMode;

	for (int i = 1; i < argc; ++i)
	{
		if (_wcsicmp(argv[i], L"-name") == 0 && (i + 1) < argc)
		{
			CopyWideArgToAnsi(argv[++i], g_Win64Username, sizeof(g_Win64Username));
		}
		else if (_wcsicmp(argv[i], L"-ip") == 0 && (i + 1) < argc)
		{
			char ipBuf[256];
			CopyWideArgToAnsi(argv[++i], ipBuf, sizeof(ipBuf));
			if (options.serverMode)
			{
				strncpy_s(g_Win64DedicatedServerBindIP, sizeof(g_Win64DedicatedServerBindIP), ipBuf, _TRUNCATE);
			}
			else
			{
				strncpy_s(g_Win64MultiplayerIP, sizeof(g_Win64MultiplayerIP), ipBuf, _TRUNCATE);
				g_Win64MultiplayerJoin = true;
			}
		}
		else if (_wcsicmp(argv[i], L"-port") == 0 && (i + 1) < argc)
		{
			wchar_t* endPtr = NULL;
			long port = wcstol(argv[++i], &endPtr, 10);
			if (endPtr != argv[i] && *endPtr == 0 && port > 0 && port <= 65535)
			{
				if (options.serverMode)
					g_Win64DedicatedServerPort = (int)port;
				else
					g_Win64MultiplayerPort = (int)port;
			}
		}
	}

	LocalFree(argv);
	return options;
}

static BOOL WINAPI HeadlessServerCtrlHandler(DWORD ctrlType)
{
	switch (ctrlType)
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		app.m_bShutdown = true;
		MinecraftServer::HaltServer();
		return TRUE;
	default:
		return FALSE;
	}
}

static void SetupHeadlessServerConsole()
{
	if (AllocConsole())
	{
		FILE* stream = NULL;
		freopen_s(&stream, "CONIN$", "r", stdin);
		freopen_s(&stream, "CONOUT$", "w", stdout);
		freopen_s(&stream, "CONOUT$", "w", stderr);
		SetConsoleTitleA("Minecraft Server");
	}

	SetConsoleCtrlHandler(HeadlessServerCtrlHandler, TRUE);
}

void DefineActions(void)
{
	// The app needs to define the actions required, and the possible mappings for these

	// Split into Menu actions, and in-game actions

	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_A,							_360_JOY_BUTTON_A);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_B,							_360_JOY_BUTTON_B);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_X,							_360_JOY_BUTTON_X);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_Y,							_360_JOY_BUTTON_Y);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_OK,							_360_JOY_BUTTON_A);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_CANCEL,						_360_JOY_BUTTON_B);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_UP,							_360_JOY_BUTTON_DPAD_UP | _360_JOY_BUTTON_LSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_DOWN,						_360_JOY_BUTTON_DPAD_DOWN | _360_JOY_BUTTON_LSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_LEFT,						_360_JOY_BUTTON_DPAD_LEFT | _360_JOY_BUTTON_LSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_RIGHT,						_360_JOY_BUTTON_DPAD_RIGHT | _360_JOY_BUTTON_LSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_PAGEUP,						_360_JOY_BUTTON_LT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_PAGEDOWN,					_360_JOY_BUTTON_RT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_RIGHT_SCROLL,				_360_JOY_BUTTON_RB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_LEFT_SCROLL,					_360_JOY_BUTTON_LB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_PAUSEMENU,					_360_JOY_BUTTON_START);

	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_STICK_PRESS,					_360_JOY_BUTTON_LTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_OTHER_STICK_PRESS,			_360_JOY_BUTTON_RTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_OTHER_STICK_UP,				_360_JOY_BUTTON_RSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_OTHER_STICK_DOWN,			_360_JOY_BUTTON_RSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_OTHER_STICK_LEFT,			_360_JOY_BUTTON_RSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_OTHER_STICK_RIGHT,			_360_JOY_BUTTON_RSTICK_RIGHT);

	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_JUMP,					_360_JOY_BUTTON_A);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_FORWARD,				_360_JOY_BUTTON_LSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_BACKWARD,				_360_JOY_BUTTON_LSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_LEFT,					_360_JOY_BUTTON_LSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_RIGHT,					_360_JOY_BUTTON_LSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_LOOK_LEFT,				_360_JOY_BUTTON_RSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_LOOK_RIGHT,				_360_JOY_BUTTON_RSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_LOOK_UP,				_360_JOY_BUTTON_RSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_LOOK_DOWN,				_360_JOY_BUTTON_RSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_USE,					_360_JOY_BUTTON_LT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_ACTION,					_360_JOY_BUTTON_RT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_RIGHT_SCROLL,			_360_JOY_BUTTON_RB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_LEFT_SCROLL,			_360_JOY_BUTTON_LB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_INVENTORY,				_360_JOY_BUTTON_Y);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_PAUSEMENU,				_360_JOY_BUTTON_START);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_DROP,					_360_JOY_BUTTON_B);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_SNEAK_TOGGLE,			_360_JOY_BUTTON_RTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_CRAFTING,				_360_JOY_BUTTON_X);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_RENDER_THIRD_PERSON,	_360_JOY_BUTTON_LTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_GAME_INFO,				_360_JOY_BUTTON_BACK);

	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_DPAD_LEFT,				_360_JOY_BUTTON_DPAD_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_DPAD_RIGHT,				_360_JOY_BUTTON_DPAD_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_DPAD_UP,				_360_JOY_BUTTON_DPAD_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_DPAD_DOWN,				_360_JOY_BUTTON_DPAD_DOWN);

	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_A,							_360_JOY_BUTTON_A);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_B,							_360_JOY_BUTTON_B);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_X,							_360_JOY_BUTTON_X);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_Y,							_360_JOY_BUTTON_Y);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_OK,							_360_JOY_BUTTON_A);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_CANCEL,						_360_JOY_BUTTON_B);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_UP,							_360_JOY_BUTTON_DPAD_UP | _360_JOY_BUTTON_LSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_DOWN,						_360_JOY_BUTTON_DPAD_DOWN | _360_JOY_BUTTON_LSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_LEFT,						_360_JOY_BUTTON_DPAD_LEFT | _360_JOY_BUTTON_LSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_RIGHT,						_360_JOY_BUTTON_DPAD_RIGHT | _360_JOY_BUTTON_LSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_PAGEUP,						_360_JOY_BUTTON_LB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_PAGEDOWN,					_360_JOY_BUTTON_RT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_RIGHT_SCROLL,				_360_JOY_BUTTON_RB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_LEFT_SCROLL,					_360_JOY_BUTTON_LB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_PAUSEMENU,					_360_JOY_BUTTON_START);

	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_STICK_PRESS,					_360_JOY_BUTTON_LTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_OTHER_STICK_PRESS,			_360_JOY_BUTTON_RTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_OTHER_STICK_UP,				_360_JOY_BUTTON_RSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_OTHER_STICK_DOWN,			_360_JOY_BUTTON_RSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_OTHER_STICK_LEFT,			_360_JOY_BUTTON_RSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_OTHER_STICK_RIGHT,			_360_JOY_BUTTON_RSTICK_RIGHT);

	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_JUMP,					_360_JOY_BUTTON_RB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_FORWARD,				_360_JOY_BUTTON_LSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_BACKWARD,				_360_JOY_BUTTON_LSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_LEFT,					_360_JOY_BUTTON_LSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_RIGHT,					_360_JOY_BUTTON_LSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_LOOK_LEFT,				_360_JOY_BUTTON_RSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_LOOK_RIGHT,				_360_JOY_BUTTON_RSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_LOOK_UP,				_360_JOY_BUTTON_RSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_LOOK_DOWN,				_360_JOY_BUTTON_RSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_USE,					_360_JOY_BUTTON_RT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_ACTION,					_360_JOY_BUTTON_LT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_RIGHT_SCROLL,			_360_JOY_BUTTON_DPAD_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_LEFT_SCROLL,			_360_JOY_BUTTON_DPAD_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_INVENTORY,				_360_JOY_BUTTON_Y);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_PAUSEMENU,				_360_JOY_BUTTON_START);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_DROP,					_360_JOY_BUTTON_B);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_SNEAK_TOGGLE,			_360_JOY_BUTTON_LTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_CRAFTING,				_360_JOY_BUTTON_X);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_RENDER_THIRD_PERSON,	_360_JOY_BUTTON_RTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_GAME_INFO,				_360_JOY_BUTTON_BACK);

	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_DPAD_LEFT,				_360_JOY_BUTTON_DPAD_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_DPAD_RIGHT,				_360_JOY_BUTTON_DPAD_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_DPAD_UP,				_360_JOY_BUTTON_DPAD_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_DPAD_DOWN,				_360_JOY_BUTTON_DPAD_DOWN);

	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_A,							_360_JOY_BUTTON_A);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_B,							_360_JOY_BUTTON_B);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_X,							_360_JOY_BUTTON_X);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_Y,							_360_JOY_BUTTON_Y);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OK,							_360_JOY_BUTTON_A);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_CANCEL,						_360_JOY_BUTTON_B);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_UP,							_360_JOY_BUTTON_DPAD_UP | _360_JOY_BUTTON_LSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_DOWN,						_360_JOY_BUTTON_DPAD_DOWN | _360_JOY_BUTTON_LSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_LEFT,						_360_JOY_BUTTON_DPAD_LEFT | _360_JOY_BUTTON_LSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_RIGHT,						_360_JOY_BUTTON_DPAD_RIGHT | _360_JOY_BUTTON_LSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_PAGEUP,						_360_JOY_BUTTON_DPAD_UP | _360_JOY_BUTTON_LB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_PAGEDOWN,					_360_JOY_BUTTON_RT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_RIGHT_SCROLL,				_360_JOY_BUTTON_RB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_LEFT_SCROLL,					_360_JOY_BUTTON_LB);

	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_JUMP,					_360_JOY_BUTTON_LT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_FORWARD,				_360_JOY_BUTTON_LSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_BACKWARD,				_360_JOY_BUTTON_LSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_LEFT,					_360_JOY_BUTTON_LSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_RIGHT,					_360_JOY_BUTTON_LSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_LOOK_LEFT,				_360_JOY_BUTTON_RSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_LOOK_RIGHT,				_360_JOY_BUTTON_RSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_LOOK_UP,				_360_JOY_BUTTON_RSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_LOOK_DOWN,				_360_JOY_BUTTON_RSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_USE,					_360_JOY_BUTTON_RT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_ACTION,					_360_JOY_BUTTON_A);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_RIGHT_SCROLL,			_360_JOY_BUTTON_DPAD_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_LEFT_SCROLL,			_360_JOY_BUTTON_DPAD_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_INVENTORY,				_360_JOY_BUTTON_Y);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_PAUSEMENU,				_360_JOY_BUTTON_START);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_DROP,					_360_JOY_BUTTON_B);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_SNEAK_TOGGLE,			_360_JOY_BUTTON_LB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_CRAFTING,				_360_JOY_BUTTON_X);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_RENDER_THIRD_PERSON,	_360_JOY_BUTTON_LTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_GAME_INFO,				_360_JOY_BUTTON_BACK);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_PAUSEMENU,					_360_JOY_BUTTON_START);

	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_STICK_PRESS,					_360_JOY_BUTTON_LTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OTHER_STICK_PRESS,			_360_JOY_BUTTON_RTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OTHER_STICK_UP,				_360_JOY_BUTTON_RSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OTHER_STICK_DOWN,			_360_JOY_BUTTON_RSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OTHER_STICK_LEFT,			_360_JOY_BUTTON_RSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OTHER_STICK_RIGHT,			_360_JOY_BUTTON_RSTICK_RIGHT);

	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_DPAD_LEFT,				_360_JOY_BUTTON_DPAD_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_DPAD_RIGHT,				_360_JOY_BUTTON_DPAD_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_DPAD_UP,				_360_JOY_BUTTON_DPAD_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_DPAD_DOWN,				_360_JOY_BUTTON_DPAD_DOWN);
}

#if 0
HRESULT InitD3D( IDirect3DDevice9 **ppDevice,
				D3DPRESENT_PARAMETERS *pd3dPP )
{
	IDirect3D9 *pD3D;

	pD3D = Direct3DCreate9( D3D_SDK_VERSION );

	// Set up the structure used to create the D3DDevice
	// Using a permanent 1280x720 backbuffer now no matter what the actual video resolution.right Have also disabled letterboxing,
	// which would letterbox a 1280x720 output if it detected a 4:3 video source - we're doing an anamorphic squash in this
	// mode so don't need this functionality.

	ZeroMemory( pd3dPP, sizeof(D3DPRESENT_PARAMETERS) );
	XVIDEO_MODE VideoMode;
	XGetVideoMode( &VideoMode );
	g_bWidescreen = VideoMode.fIsWideScreen;
	pd3dPP->BackBufferWidth        = 1280;
	pd3dPP->BackBufferHeight       = 720;
	pd3dPP->BackBufferFormat       = D3DFMT_A8R8G8B8;
	pd3dPP->BackBufferCount        = 1;
	pd3dPP->EnableAutoDepthStencil = TRUE;
	pd3dPP->AutoDepthStencilFormat = D3DFMT_D24S8;
	pd3dPP->SwapEffect             = D3DSWAPEFFECT_DISCARD;
	pd3dPP->PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;
	//pd3dPP->Flags				   = D3DPRESENTFLAG_NO_LETTERBOX;
	//ERR[D3D]: Can't set D3DPRESENTFLAG_NO_LETTERBOX when wide-screen is enabled
	//	in the launcher/dashboard.
	if(g_bWidescreen)
		pd3dPP->Flags=0;
	else
		pd3dPP->Flags				   = D3DPRESENTFLAG_NO_LETTERBOX;

	// Create the device.
	return pD3D->CreateDevice(
		0,
		D3DDEVTYPE_HAL,
		NULL,
		D3DCREATE_HARDWARE_VERTEXPROCESSING|D3DCREATE_BUFFER_2_FRAMES,
		pd3dPP,
		ppDevice );
}
#endif
//#define MEMORY_TRACKING

#ifdef MEMORY_TRACKING
void ResetMem();
void DumpMem();
void MemPixStuff();
#else
void MemSect(int sect)
{
}
#endif

HINSTANCE               g_hInst = NULL;
HWND                    g_hWnd = NULL;
D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*           g_pd3dDevice = NULL;
ID3D11DeviceContext*    g_pImmediateContext = NULL;
IDXGISwapChain*         g_pSwapChain = NULL;
ID3D11RenderTargetView* g_pRenderTargetView = NULL;
ID3D11DepthStencilView* g_pDepthStencilView = NULL;
ID3D11Texture2D*		g_pDepthStencilBuffer = NULL;

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//  WM_SIZE		- handle resizing logic to support Any Aspect Ratio
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_KILLFOCUS:
		g_KBMInput.ClearAllState();
		g_KBMInput.SetWindowFocused(false);
		if (g_KBMInput.IsMouseGrabbed())
			g_KBMInput.SetMouseGrabbed(false);
		break;

	case WM_SETFOCUS:
		g_KBMInput.SetWindowFocused(true);
		break;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	{
		int vk = (int)wParam;
		if (lParam & 0x40000000) break; // ignore auto-repeat
		if (vk == VK_SHIFT)
			vk = (MapVirtualKey((lParam >> 16) & 0xFF, MAPVK_VSC_TO_VK_EX) == VK_RSHIFT) ? VK_RSHIFT : VK_LSHIFT;
		else if (vk == VK_CONTROL)
			vk = (lParam & (1 << 24)) ? VK_RCONTROL : VK_LCONTROL;
		else if (vk == VK_MENU)
			vk = (lParam & (1 << 24)) ? VK_RMENU : VK_LMENU;
		g_KBMInput.OnKeyDown(vk);
		break;
	}
	case WM_KEYUP:
	case WM_SYSKEYUP:
	{
		int vk = (int)wParam;
		if (vk == VK_SHIFT)
			vk = (MapVirtualKey((lParam >> 16) & 0xFF, MAPVK_VSC_TO_VK_EX) == VK_RSHIFT) ? VK_RSHIFT : VK_LSHIFT;
		else if (vk == VK_CONTROL)
			vk = (lParam & (1 << 24)) ? VK_RCONTROL : VK_LCONTROL;
		else if (vk == VK_MENU)
			vk = (lParam & (1 << 24)) ? VK_RMENU : VK_LMENU;
		g_KBMInput.OnKeyUp(vk);
		break;
	}

	case WM_LBUTTONDOWN:
		g_KBMInput.OnMouseButtonDown(KeyboardMouseInput::MOUSE_LEFT);
		break;
	case WM_LBUTTONUP:
		g_KBMInput.OnMouseButtonUp(KeyboardMouseInput::MOUSE_LEFT);
		break;
	case WM_RBUTTONDOWN:
		g_KBMInput.OnMouseButtonDown(KeyboardMouseInput::MOUSE_RIGHT);
		break;
	case WM_RBUTTONUP:
		g_KBMInput.OnMouseButtonUp(KeyboardMouseInput::MOUSE_RIGHT);
		break;
	case WM_MBUTTONDOWN:
		g_KBMInput.OnMouseButtonDown(KeyboardMouseInput::MOUSE_MIDDLE);
		break;
	case WM_MBUTTONUP:
		g_KBMInput.OnMouseButtonUp(KeyboardMouseInput::MOUSE_MIDDLE);
		break;

	case WM_MOUSEMOVE:
		g_KBMInput.OnMouseMove(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_MOUSEWHEEL:
		g_KBMInput.OnMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam));
		break;

	case WM_INPUT:
		{
			UINT dwSize = 0;
			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
			if (dwSize > 0 && dwSize <= 256)
			{
				BYTE rawBuffer[256];
				if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, rawBuffer, &dwSize, sizeof(RAWINPUTHEADER)) == dwSize)
				{
					RAWINPUT* raw = (RAWINPUT*)rawBuffer;
					if (raw->header.dwType == RIM_TYPEMOUSE)
					{
						g_KBMInput.OnRawMouseDelta(raw->data.mouse.lLastX, raw->data.mouse.lLastY);
					}
				}
			}
		}
		break;
	case WM_SIZE:
		{
			UpdateAspectRatio(LOWORD(lParam), HIWORD(lParam));
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, "Minecraft");
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= "Minecraft";
	wcex.lpszClassName	= "MinecraftClass";
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_MINECRAFTWINDOWS));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	g_hInst = hInstance; // Store instance handle in our global variable

	RECT wr = {0, 0, g_iScreenWidth, g_iScreenHeight};    // set the size, but not the position
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);    // adjust the size

	g_hWnd = CreateWindow(	"MinecraftClass",
		"Minecraft",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		0,
		wr.right - wr.left,    // width of the window
		wr.bottom - wr.top,    // height of the window
		NULL,
		NULL,
		hInstance,
		NULL);

	if (!g_hWnd)
	{
		return FALSE;
	}

	ShowWindow(g_hWnd, (nCmdShow != SW_HIDE) ? SW_SHOWMAXIMIZED : nCmdShow);
	UpdateWindow(g_hWnd);

	return TRUE;
}

// 4J Stu - These functions are referenced from the Windows Input library
void ClearGlobalText()
{
	// clear the global text
	memset(chGlobalText,0,256);
	memset(ui16GlobalText,0,512);
}

uint16_t *GetGlobalText()
{
	//copy the ch text to ui16
	char * pchBuffer=(char *)ui16GlobalText;
		for(int i=0;i<256;i++)
		{
			pchBuffer[i*2]=chGlobalText[i];
		}
	return ui16GlobalText;
}
void SeedEditBox()
{
	DialogBox(hMyInst, MAKEINTRESOURCE(IDD_SEED),
		g_hWnd, reinterpret_cast<DLGPROC>(DlgProc));
}


//---------------------------------------------------------------------------
LRESULT CALLBACK DlgProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch(Msg)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		switch(wParam)
		{
		case IDOK:
			// Set the text
			GetDlgItemText(hWndDlg,IDC_EDIT,chGlobalText,256);
			EndDialog(hWndDlg, 0);
			return TRUE;
		}
		break;
	}

	return FALSE;
}

//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect( g_hWnd, &rc );
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;
//app.DebugPrintf("width: %d, height: %d\n", width, height);
	width = g_iScreenWidth;
	height = g_iScreenHeight;
//app.DebugPrintf("width: %d, height: %d\n", width, height);

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE( driverTypes );

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE( featureLevels );

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory( &sd, sizeof( sd ) );
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = g_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
	{
		g_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain( NULL, g_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext );
		if( HRESULT_SUCCEEDED( hr ) )
			break;
	}
	if( FAILED( hr ) )
		return hr;

	// Create a render target view
	ID3D11Texture2D* pBackBuffer = NULL;
	hr = g_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
	if( FAILED( hr ) )
		return hr;

	// Create a depth stencil buffer
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));

	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = g_pd3dDevice->CreateTexture2D(&descDepth, NULL, &g_pDepthStencilBuffer);

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSView;
	ZeroMemory(&descDSView, sizeof(descDSView));
	descDSView.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDSView.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSView.Texture2D.MipSlice = 0;

	hr = g_pd3dDevice->CreateDepthStencilView(g_pDepthStencilBuffer, &descDSView, &g_pDepthStencilView);

	hr = g_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &g_pRenderTargetView );
	pBackBuffer->Release();
	if( FAILED( hr ) )
		return hr;

	g_pImmediateContext->OMSetRenderTargets( 1, &g_pRenderTargetView, g_pDepthStencilView );

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pImmediateContext->RSSetViewports( 1, &vp );

	RenderManager.Initialise(g_pd3dDevice, g_pSwapChain);

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Render the frame
//--------------------------------------------------------------------------------------
void Render()
{
	// Just clear the backbuffer
	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; //red,green,blue,alpha

	g_pImmediateContext->ClearRenderTargetView( g_pRenderTargetView, ClearColor );
	g_pSwapChain->Present( 0, 0 );
}

//--------------------------------------------------------------------------------------
// Toggle borderless fullscreen
//--------------------------------------------------------------------------------------
void ToggleFullscreen()
{
	DWORD dwStyle = GetWindowLong(g_hWnd, GWL_STYLE);
	if (!g_isFullscreen)
	{
		MONITORINFO mi = { sizeof(mi) };
		if (GetWindowPlacement(g_hWnd, &g_wpPrev) &&
			GetMonitorInfo(MonitorFromWindow(g_hWnd, MONITOR_DEFAULTTOPRIMARY), &mi))
		{
			SetWindowLong(g_hWnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
			SetWindowPos(g_hWnd, HWND_TOP,
				mi.rcMonitor.left, mi.rcMonitor.top,
				mi.rcMonitor.right - mi.rcMonitor.left,
				mi.rcMonitor.bottom - mi.rcMonitor.top,
				SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
	}
	else
	{
		SetWindowLong(g_hWnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(g_hWnd, &g_wpPrev);
		SetWindowPos(g_hWnd, NULL, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}
	g_isFullscreen = !g_isFullscreen;
	SaveFullscreenOption(g_isFullscreen);

	if (g_KBMInput.IsWindowFocused())
		g_KBMInput.SetWindowFocused(true);
}

//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
	if( g_pImmediateContext ) g_pImmediateContext->ClearState();

	if( g_pRenderTargetView ) g_pRenderTargetView->Release();
	if( g_pSwapChain ) g_pSwapChain->Release();
	if( g_pImmediateContext ) g_pImmediateContext->Release();
	if( g_pd3dDevice ) g_pd3dDevice->Release();
}

static Minecraft* InitialiseMinecraftRuntime()
{
	app.loadMediaArchive();

	RenderManager.Initialise(g_pd3dDevice, g_pSwapChain);

	app.loadStringTable();
	ui.init(g_pd3dDevice, g_pImmediateContext, g_pRenderTargetView, g_pDepthStencilView, g_iScreenWidth, g_iScreenHeight);

	InputManager.Initialise(1, 3, MINECRAFT_ACTION_MAX, ACTION_MAX_MENU);
	g_KBMInput.Init();
	DefineActions();
	InputManager.SetJoypadMapVal(0, 0);
	InputManager.SetKeyRepeatRate(0.3f, 0.2f);

	ProfileManager.Initialise(TITLEID_MINECRAFT,
		app.m_dwOfferID,
		PROFILE_VERSION_10,
		NUM_PROFILE_VALUES,
		NUM_PROFILE_SETTINGS,
		dwProfileSettingsA,
		app.GAME_DEFINED_PROFILE_DATA_BYTES * XUSER_MAX_COUNT,
		&app.uiGameDefinedDataChangedBitmask
	);
	ProfileManager.SetDefaultOptionsCallback(&CConsoleMinecraftApp::DefaultOptionsCallback, (LPVOID)&app);

	g_NetworkManager.Initialise();

	for (int i = 0; i < MINECRAFT_NET_MAX_PLAYERS; i++)
	{
		IQNet::m_player[i].m_smallId = (BYTE)i;
		IQNet::m_player[i].m_isRemote = false;
		IQNet::m_player[i].m_isHostPlayer = (i == 0);
		swprintf_s(IQNet::m_player[i].m_gamertag, 32, L"Player%d", i);
	}
	wcscpy_s(IQNet::m_player[0].m_gamertag, 32, g_Win64UsernameW);

	WinsockNetLayer::Initialize();

	ProfileManager.SetDebugFullOverride(true);

	Tesselator::CreateNewThreadStorage(1024 * 1024);
	AABB::CreateNewThreadStorage();
	Vec3::CreateNewThreadStorage();
	IntCache::CreateNewThreadStorage();
	Compression::CreateNewThreadStorage();
	OldChunkStorage::CreateNewThreadStorage();
	Level::enableLightingCache();
	Tile::CreateNewThreadStorage();

	Minecraft::main();
	Minecraft* pMinecraft = Minecraft::GetInstance();
	if (pMinecraft == NULL)
		return NULL;

	app.InitGameSettings();
	app.InitialiseTips();

	pMinecraft->options->set(Options::Option::MUSIC, 1.0f);
	pMinecraft->options->set(Options::Option::SOUND, 1.0f);

	return pMinecraft;
}

static int HeadlessServerConsoleThreadProc(void* lpParameter)
{
	UNREFERENCED_PARAMETER(lpParameter);

	std::string line;
	while (!app.m_bShutdown)
	{
		if (!std::getline(std::cin, line))
		{
			if (std::cin.eof())
			{
				break;
			}

			std::cin.clear();
			Sleep(50);
			continue;
		}

		wstring command = trimString(convStringToWstring(line));
		if (command.empty())
			continue;

		MinecraftServer* server = MinecraftServer::getInstance();
		if (server != NULL)
		{
			server->handleConsoleInput(command, server);
		}
	}

	return 0;
}

static int RunHeadlessServer()
{
	SetupHeadlessServerConsole();

	Settings serverSettings(new File(L"server.properties"));
	wstring configuredBindIp = serverSettings.getString(L"server-ip", L"");

	const char* bindIp = "*";
	if (g_Win64DedicatedServerBindIP[0] != 0)
	{
		bindIp = g_Win64DedicatedServerBindIP;
	}
	else if (!configuredBindIp.empty())
	{
		bindIp = wstringtochararray(configuredBindIp);
	}

	const int port = g_Win64DedicatedServerPort > 0 ? g_Win64DedicatedServerPort : serverSettings.getInt(L"server-port", WIN64_NET_DEFAULT_PORT);

	printf("Starting headless server on %s:%d\n", bindIp, port);
	fflush(stdout);

	Minecraft* pMinecraft = InitialiseMinecraftRuntime();
	if (pMinecraft == NULL)
	{
		fprintf(stderr, "Failed to initialise the Minecraft runtime.\n");
		return 1;
	}

	app.SetGameHostOption(eGameHostOption_Difficulty, serverSettings.getInt(L"difficulty", 1));
	app.SetGameHostOption(eGameHostOption_Gamertags, 1);
	app.SetGameHostOption(eGameHostOption_GameType, serverSettings.getInt(L"gamemode", 0));
	app.SetGameHostOption(eGameHostOption_LevelType, 0);
	app.SetGameHostOption(eGameHostOption_Structures, serverSettings.getBoolean(L"generate-structures", true) ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_BonusChest, serverSettings.getBoolean(L"bonus-chest", false) ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_PvP, serverSettings.getBoolean(L"pvp", true) ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_TrustPlayers, serverSettings.getBoolean(L"trust-players", true) ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_FireSpreads, serverSettings.getBoolean(L"fire-spreads", true) ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_TNT, serverSettings.getBoolean(L"tnt", true) ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_HostCanFly, 1);
	app.SetGameHostOption(eGameHostOption_HostCanChangeHunger, 1);
	app.SetGameHostOption(eGameHostOption_HostCanBeInvisible, 1);
	app.SetGameHostOption(eGameHostOption_MobGriefing, 1);
	app.SetGameHostOption(eGameHostOption_KeepInventory, 0);
	app.SetGameHostOption(eGameHostOption_DoMobSpawning, 1);
	app.SetGameHostOption(eGameHostOption_DoMobLoot, 1);
	app.SetGameHostOption(eGameHostOption_DoTileDrops, 1);
	app.SetGameHostOption(eGameHostOption_NaturalRegeneration, 1);
	app.SetGameHostOption(eGameHostOption_DoDaylightCycle, 1);

	MinecraftServer::resetFlags();
	g_NetworkManager.HostGame(0, false, true, MINECRAFT_NET_MAX_PLAYERS, 0);

	if (!WinsockNetLayer::IsActive())
	{
		fprintf(stderr, "Failed to bind the server socket on %s:%d.\n", bindIp, port);
		return 1;
	}

	g_NetworkManager.FakeLocalPlayerJoined();

	NetworkGameInitData* param = new NetworkGameInitData();
	param->seed = 0;
	param->settings = app.GetGameHostOption(eGameHostOption_All);

	g_NetworkManager.ServerStoppedCreate(true);
	g_NetworkManager.ServerReadyCreate(true);

	C4JThread* thread = new C4JThread(&CGameNetworkManager::ServerThreadProc, param, "Server", 256 * 1024);
	thread->SetProcessor(CPU_CORE_SERVER);
	thread->Run();

	g_NetworkManager.ServerReadyWait();
	g_NetworkManager.ServerReadyDestroy();

	if (MinecraftServer::serverHalted())
	{
		fprintf(stderr, "The server halted during startup.\n");
		g_NetworkManager.LeaveGame(false);
		return 1;
	}

	app.SetGameStarted(true);
	g_NetworkManager.DoWork();

	printf("Server ready on %s:%d\n", bindIp, port);
	printf("Type 'help' for server commands.\n");
	fflush(stdout);

	C4JThread* consoleThread = new C4JThread(&HeadlessServerConsoleThreadProc, NULL, "Server console", 128 * 1024);
	consoleThread->Run();

	MSG msg = { 0 };
	while (WM_QUIT != msg.message && !app.m_bShutdown && !MinecraftServer::serverHalted())
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

		app.UpdateTime();
		ProfileManager.Tick();
		StorageManager.Tick();
		RenderManager.Tick();
		ui.tick();
		g_NetworkManager.DoWork();
		app.HandleXuiActions();

		Sleep(10);
	}

	printf("Stopping server...\n");
	fflush(stdout);

	app.m_bShutdown = true;
	MinecraftServer::HaltServer();
	g_NetworkManager.LeaveGame(false);
	return 0;
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
					   _In_opt_ HINSTANCE hPrevInstance,
					   _In_ LPTSTR    lpCmdLine,
					   _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// 4J-Win64: set CWD to exe dir so asset paths resolve correctly
	{
		char szExeDir[MAX_PATH] = {};
		GetModuleFileNameA(NULL, szExeDir, MAX_PATH);
		char *pSlash = strrchr(szExeDir, '\\');
		if (pSlash) { *(pSlash + 1) = '\0'; SetCurrentDirectoryA(szExeDir); }
	}

	// Declare DPI awareness so GetSystemMetrics returns physical pixels
	SetProcessDPIAware();
	g_iScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	g_iScreenHeight = GetSystemMetrics(SM_CYSCREEN);

	// Load username from username.txt
    char exePath[MAX_PATH] = {};
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    char *lastSlash = strrchr(exePath, '\\');
    if (lastSlash)
    {
        *(lastSlash + 1) = '\0';
    }

    char filePath[MAX_PATH] = {};
    _snprintf_s(filePath, sizeof(filePath), _TRUNCATE, "%susername.txt", exePath);

    FILE *f = nullptr;
    if (fopen_s(&f, filePath, "r") == 0 && f)
    {
        char buf[128] = {};
        if (fgets(buf, sizeof(buf), f))
        {
            int len = (int)strlen(buf);
            while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r' || buf[len - 1] == ' '))
            {
                buf[--len] = '\0';
            }

            if (len > 0)
            {
                strncpy_s(g_Win64Username, sizeof(g_Win64Username), buf, _TRUNCATE);
            }
        }
        fclose(f);
    }

	// Load stuff from launch options, including username
	Win64LaunchOptions launchOptions = ParseLaunchOptions();
	ApplyScreenMode(launchOptions.screenMode);

	// If no username, let's fall back
	if (g_Win64Username[0] == 0)
	{
        // Default username will be "Player"
        strncpy_s(g_Win64Username, sizeof(g_Win64Username), "Player", _TRUNCATE);
	}

	MultiByteToWideChar(CP_ACP, 0, g_Win64Username, -1, g_Win64UsernameW, 17);

	// Initialize global strings
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, launchOptions.serverMode ? SW_HIDE : nCmdShow))
	{
		return FALSE;
	}

	hMyInst=hInstance;

	if( FAILED( InitDevice() ) )
	{
		CleanupDevice();
		return 0;
	}

	// Restore fullscreen state from previous session
	if (LoadFullscreenOption() && !g_isFullscreen)
	{
		ToggleFullscreen();
	}

	if (launchOptions.serverMode)
	{
		int serverResult = RunHeadlessServer();
		CleanupDevice();
		return serverResult;
	}

#if 0
	// Main message loop
	MSG msg = {0};
	while( WM_QUIT != msg.message )
	{
		if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		else
		{
			Render();
		}
	}

	return (int) msg.wParam;
#endif

	static bool bTrialTimerDisplayed=true;

#ifdef MEMORY_TRACKING
	ResetMem();
	MEMORYSTATUS memStat;
	GlobalMemoryStatus(&memStat);
	printf("RESETMEM start: Avail. phys %d\n",memStat.dwAvailPhys/(1024*1024));
#endif

#if 0
	// Initialize D3D
	hr = InitD3D( &pDevice, &d3dpp );
	g_pD3DDevice = pDevice;
	if( FAILED(hr) )
	{
		app.DebugPrintf
			( "Failed initializing D3D.\n" );
		return -1;
	}

	// Initialize the application, assuming sharing of the d3d interface.
	hr = app.InitShared( pDevice, &d3dpp,
		XuiPNGTextureLoader );

	if ( FAILED(hr) )
	{
		app.DebugPrintf
			( "Failed initializing application.\n" );

		return -1;
	}

#endif
	Minecraft *pMinecraft = InitialiseMinecraftRuntime();
	if (pMinecraft == NULL)
	{
		CleanupDevice();
		return 1;
	}

	//app.TemporaryCreateGameStart();

	//Sleep(10000);
#if 0
	// Intro loop ?
	while(app.IntroRunning())
	{
		ProfileManager.Tick();
		// Tick XUI
		app.RunFrame();

		// 4J : WESTY : Added to ensure we always have clear background for intro.
		RenderManager.SetClearColour(D3DCOLOR_RGBA(0,0,0,255));
		RenderManager.Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Render XUI
		hr = app.Render();

		// Present the frame.
		RenderManager.Present();

		// Update XUI Timers
		hr = XuiTimersRun();
	}
#endif
	MSG msg = {0};
	while( WM_QUIT != msg.message && !app.m_bShutdown)
	{
		g_KBMInput.Tick();

		while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
			if (msg.message == WM_QUIT) break;
		}
		if (msg.message == WM_QUIT) break;

		RenderManager.StartFrame();
#if 0
		if(pMinecraft->soundEngine->isStreamingWavebankReady() &&
			!pMinecraft->soundEngine->isPlayingStreamingGameMusic() &&
			!pMinecraft->soundEngine->isPlayingStreamingCDMusic() )
		{
			// play some music in the menus
			pMinecraft->soundEngine->playStreaming(L"", 0, 0, 0, 0, 0, false);
		}
#endif

		// 		static bool bPlay=false;
		// 		if(bPlay)
		// 		{
		// 			bPlay=false;
		// 			app.audio.PlaySound();
		// 		}

		app.UpdateTime();
		PIXBeginNamedEvent(0,"Input manager tick");
		InputManager.Tick();

		// Detect KBM vs controller input mode
		if (InputManager.IsPadConnected(0))
		{
			bool controllerUsed = InputManager.ButtonPressed(0) ||
				InputManager.GetJoypadStick_LX(0, false) != 0.0f ||
				InputManager.GetJoypadStick_LY(0, false) != 0.0f ||
				InputManager.GetJoypadStick_RX(0, false) != 0.0f ||
				InputManager.GetJoypadStick_RY(0, false) != 0.0f;

			if (controllerUsed)
				g_KBMInput.SetKBMActive(false);
			else if (g_KBMInput.HasAnyInput())
				g_KBMInput.SetKBMActive(true);
		}
		else
		{
			g_KBMInput.SetKBMActive(true);
		}

		if (!g_KBMInput.IsMouseGrabbed())
		{
			if (!g_KBMInput.IsKBMActive())
				g_KBMInput.SetCursorHiddenForUI(true);
			else if (!g_KBMInput.IsScreenCursorHidden())
				g_KBMInput.SetCursorHiddenForUI(false);
		}

		PIXEndNamedEvent();
		PIXBeginNamedEvent(0,"Profile manager tick");
		//		ProfileManager.Tick();
		PIXEndNamedEvent();
		PIXBeginNamedEvent(0,"Storage manager tick");
		StorageManager.Tick();
		PIXEndNamedEvent();
		PIXBeginNamedEvent(0,"Render manager tick");
		RenderManager.Tick();
		PIXEndNamedEvent();

		// Tick the social networking manager.
		PIXBeginNamedEvent(0,"Social network manager tick");
		//		CSocialManager::Instance()->Tick();
		PIXEndNamedEvent();

		// Tick sentient.
		PIXBeginNamedEvent(0,"Sentient tick");
		MemSect(37);
		//		SentientManager.Tick();
		MemSect(0);
		PIXEndNamedEvent();

		PIXBeginNamedEvent(0,"Network manager do work #1");
		g_NetworkManager.DoWork();
		PIXEndNamedEvent();

		//		LeaderboardManager::Instance()->Tick();
		// Render game graphics.
		if(app.GetGameStarted())
		{
			pMinecraft->applyFrameMouseLook();  // Per-frame mouse look (before ticks + render)
			pMinecraft->run_middle();
			app.SetAppPaused( g_NetworkManager.IsLocalGame() && g_NetworkManager.GetPlayerCount() == 1 && ui.IsPauseMenuDisplayed(ProfileManager.GetPrimaryPad()) );
		}
		else
		{
			MemSect(28);
			pMinecraft->soundEngine->tick(NULL, 0.0f);
			MemSect(0);
			pMinecraft->textures->tick(true,false);
			IntCache::Reset();
			if( app.GetReallyChangingSessionType() )
			{
				pMinecraft->tickAllConnections();		// Added to stop timing out when we are waiting after converting to an offline game
			}
		}

		pMinecraft->soundEngine->playMusicTick();

#ifdef MEMORY_TRACKING
		static bool bResetMemTrack = false;
		static bool bDumpMemTrack = false;

		MemPixStuff();

		if( bResetMemTrack )
		{
			ResetMem();
			MEMORYSTATUS memStat;
			GlobalMemoryStatus(&memStat);
			printf("RESETMEM: Avail. phys %d\n",memStat.dwAvailPhys/(1024*1024));
			bResetMemTrack = false;
		}

		if( bDumpMemTrack )
		{
			DumpMem();
			bDumpMemTrack = false;
			MEMORYSTATUS memStat;
			GlobalMemoryStatus(&memStat);
			printf("DUMPMEM: Avail. phys %d\n",memStat.dwAvailPhys/(1024*1024));
			printf("Renderer used: %d\n",RenderManager.CBuffSize(-1));
		}
#endif
#if 0
		static bool bDumpTextureUsage = false;
		if( bDumpTextureUsage )
		{
			RenderManager.TextureGetStats();
			bDumpTextureUsage = false;
		}
#endif
		ui.tick();
		ui.render();
#if 0
		app.HandleButtonPresses();

		// store the minecraft renderstates, and re-set them after the xui render
		GetRenderAndSamplerStates(pDevice,RenderStateA,SamplerStateA);

		// Tick XUI
		PIXBeginNamedEvent(0,"Xui running");
		app.RunFrame();
		PIXEndNamedEvent();

		// Render XUI

		PIXBeginNamedEvent(0,"XUI render");
		MemSect(7);
		hr = app.Render();
		MemSect(0);
		GetRenderAndSamplerStates(pDevice,RenderStateA2,SamplerStateA2);
		PIXEndNamedEvent();

		for(int i=0;i<8;i++)
		{
			if(RenderStateA2[i]!=RenderStateA[i])
			{
				//printf("Reseting RenderStateA[%d] after a XUI render\n",i);
				pDevice->SetRenderState(RenderStateModes[i],RenderStateA[i]);
			}
		}
		for(int i=0;i<5;i++)
		{
			if(SamplerStateA2[i]!=SamplerStateA[i])
			{
				//printf("Reseting SamplerStateA[%d] after a XUI render\n",i);
				pDevice->SetSamplerState(0,SamplerStateModes[i],SamplerStateA[i]);
			}
		}

		RenderManager.Set_matrixDirty();
#endif
		// Present the frame.
		RenderManager.Present();

		ui.CheckMenuDisplayed();

		// Update mouse grab: grab when in-game and no menu is open
		{
			static bool altToggleSuppressCapture = false;
			bool shouldCapture = app.GetGameStarted() && !ui.GetMenuDisplayed(0) && pMinecraft->screen == NULL;
			// Left Alt key toggles capture on/off for debugging
			if (g_KBMInput.IsKeyPressed(VK_LMENU) || g_KBMInput.IsKeyPressed(VK_RMENU))
			{
				if (g_KBMInput.IsMouseGrabbed()) { g_KBMInput.SetMouseGrabbed(false); altToggleSuppressCapture = true; }
				else if (shouldCapture)   { g_KBMInput.SetMouseGrabbed(true);  altToggleSuppressCapture = false; }
			}
			else if (!shouldCapture)
			{
				if (g_KBMInput.IsMouseGrabbed()) g_KBMInput.SetMouseGrabbed(false);
				altToggleSuppressCapture = false;
			}
			else if (shouldCapture && !g_KBMInput.IsMouseGrabbed() && GetFocus() == g_hWnd && !altToggleSuppressCapture)
			{
				g_KBMInput.SetMouseGrabbed(true);
			}
		}

		// F1 toggles the HUD
		if (g_KBMInput.IsKeyPressed(VK_F1))
		{
			int primaryPad = ProfileManager.GetPrimaryPad();
			unsigned char displayHud = app.GetGameSettings(primaryPad, eGameSetting_DisplayHUD);
			app.SetGameSettings(primaryPad, eGameSetting_DisplayHUD, displayHud ? 0 : 1);
			app.SetGameSettings(primaryPad, eGameSetting_DisplayHand, displayHud ? 0 : 1);
		}

		// F3 toggles onscreen debug info
		if (g_KBMInput.IsKeyPressed(VK_F3))
		{
			if (Minecraft* pMinecraft = Minecraft::GetInstance())
			{
				if (pMinecraft->options)
				{
					pMinecraft->options->renderDebug = !pMinecraft->options->renderDebug;
				}
			}
		}

#ifdef _DEBUG_MENUS_ENABLED
		// F4 Open debug overlay
        if (g_KBMInput.IsKeyPressed(VK_F4))
        {
            if (Minecraft *pMinecraft = Minecraft::GetInstance())
            {
                if (pMinecraft->options &&
                    app.GetGameStarted() && !ui.GetMenuDisplayed(0) && pMinecraft->screen == NULL)
                {
                    ui.NavigateToScene(0, eUIScene_DebugOverlay, NULL, eUILayer_Debug);
                }
            }
        }

        // F6 Open debug console
        if (g_KBMInput.IsKeyPressed(VK_F6))
        {
        	static bool s_debugConsole = false;
        	s_debugConsole = !s_debugConsole;
        	ui.ShowUIDebugConsole(s_debugConsole);
        }
#endif

		// F11 Toggle fullscreen
		if (g_KBMInput.IsKeyPressed(VK_F11))
		{
			ToggleFullscreen();
		}

		// TAB opens game info menu. - Vvis :3 - Updated by detectiveren
		if (g_KBMInput.IsKeyPressed(VK_TAB) && !ui.GetMenuDisplayed(0))
		{
			if (Minecraft* pMinecraft = Minecraft::GetInstance())
			{
				{
					ui.NavigateToScene(0, eUIScene_InGameInfoMenu);

				}
			}
		}

#if 0
		// has the game defined profile data been changed (by a profile load)
		if(app.uiGameDefinedDataChangedBitmask!=0)
		{
			void *pData;
			for(int i=0;i<XUSER_MAX_COUNT;i++)
			{
				if(app.uiGameDefinedDataChangedBitmask&(1<<i))
				{\
				// It has - game needs to update its values with the data from the profile
				pData=ProfileManager.GetGameDefinedProfileData(i);
				// reset the changed flag
				app.ClearGameSettingsChangedFlag(i);
				app.DebugPrintf("***  - APPLYING GAME SETTINGS CHANGE for pad %d\n",i);
				app.ApplyGameSettingsChanged(i);

#ifdef _DEBUG_MENUS_ENABLED
				if(app.DebugSettingsOn())
				{
					app.ActionDebugMask(i);
				}
				else
				{
					// force debug mask off
					app.ActionDebugMask(i,true);
				}
#endif
				// clear the stats first - there could have beena signout and sign back in in the menus
				// need to clear the player stats - can't assume it'll be done in setlevel - we may not be in the game
				pMinecraft->stats[ i ]->clear();
				pMinecraft->stats[i]->parse(pData);
				}
			}

			// Check to see if we can post to social networks.
			CSocialManager::Instance()->RefreshPostingCapability();

			// clear the flag
			app.uiGameDefinedDataChangedBitmask=0;

			// Check if any profile write are needed
			app.CheckGameSettingsChanged();
		}
		PIXEndNamedEvent();
		app.TickDLCOffersRetrieved();
		app.TickTMSPPFilesRetrieved();

		PIXBeginNamedEvent(0,"Network manager do work #2");
		g_NetworkManager.DoWork();
		PIXEndNamedEvent();

		PIXBeginNamedEvent(0,"Misc extra xui");
		// Update XUI Timers
		hr = XuiTimersRun();

#endif
		// Any threading type things to deal with from the xui side?
		app.HandleXuiActions();

#if 0
		PIXEndNamedEvent();
#endif

		// 4J-PB - Update the trial timer display if we are in the trial version
		if(!ProfileManager.IsFullVersion())
		{
			// display the trial timer
			if(app.GetGameStarted())
			{
				// 4J-PB - if the game is paused, add the elapsed time to the trial timer count so it doesn't tick down
				if(app.IsAppPaused())
				{
					app.UpdateTrialPausedTimer();
				}
				ui.UpdateTrialTimer(ProfileManager.GetPrimaryPad());
			}
		}
		else
		{
			// need to turn off the trial timer if it was on , and we've unlocked the full version
			if(bTrialTimerDisplayed)
			{
				ui.ShowTrialTimer(false);
				bTrialTimerDisplayed=false;
			}
		}

		// Fix for #7318 - Title crashes after short soak in the leaderboards menu
		// A memory leak was caused because the icon renderer kept creating new Vec3's because the pool wasn't reset
		Vec3::resetPool();
	}

	// Free resources, unregister custom classes, and exit.
	//	app.Uninit();
	g_pd3dDevice->Release();
}

#ifdef MEMORY_TRACKING

int totalAllocGen = 0;
unordered_map<int,int> allocCounts;
bool trackEnable = false;
bool trackStarted = false;
volatile size_t sizeCheckMin = 1160;
volatile size_t sizeCheckMax = 1160;
volatile int sectCheck = 48;
CRITICAL_SECTION memCS;
DWORD tlsIdx;

LPVOID XMemAlloc(SIZE_T dwSize, DWORD dwAllocAttributes)
{
	if( !trackStarted )
	{
		void *p = XMemAllocDefault(dwSize,dwAllocAttributes);
		size_t realSize = XMemSizeDefault(p, dwAllocAttributes);
		totalAllocGen += realSize;
		return p;
	}

	EnterCriticalSection(&memCS);

	void *p=XMemAllocDefault(dwSize + 16,dwAllocAttributes);
	size_t realSize = XMemSizeDefault(p,dwAllocAttributes) - 16;

	if( trackEnable )
	{
#if 1
		int sect = ((int) TlsGetValue(tlsIdx)) & 0x3f;
		*(((unsigned char *)p)+realSize) = sect;

		if( ( realSize >= sizeCheckMin ) && ( realSize <= sizeCheckMax ) && ( ( sect == sectCheck ) || ( sectCheck == -1 ) ) )
		{
			app.DebugPrintf("Found one\n");
		}
#endif

		if( p )
		{
			totalAllocGen += realSize;
			trackEnable = false;
			int key = ( sect << 26 ) | realSize;
			int oldCount = allocCounts[key];
			allocCounts[key] = oldCount + 1;

			trackEnable = true;
		}
	}

	LeaveCriticalSection(&memCS);

	return p;
}

void* operator new (size_t size)
{
	return (unsigned char *)XMemAlloc(size,MAKE_XALLOC_ATTRIBUTES(0,FALSE,TRUE,FALSE,0,XALLOC_PHYSICAL_ALIGNMENT_DEFAULT,XALLOC_MEMPROTECT_READWRITE,FALSE,XALLOC_MEMTYPE_HEAP));
}

void operator delete (void *p)
{
	XMemFree(p,MAKE_XALLOC_ATTRIBUTES(0,FALSE,TRUE,FALSE,0,XALLOC_PHYSICAL_ALIGNMENT_DEFAULT,XALLOC_MEMPROTECT_READWRITE,FALSE,XALLOC_MEMTYPE_HEAP));
}

void WINAPI XMemFree(PVOID pAddress, DWORD dwAllocAttributes)
{
	bool special = false;
	if( dwAllocAttributes == 0 )
	{
		dwAllocAttributes = MAKE_XALLOC_ATTRIBUTES(0,FALSE,TRUE,FALSE,0,XALLOC_PHYSICAL_ALIGNMENT_DEFAULT,XALLOC_MEMPROTECT_READWRITE,FALSE,XALLOC_MEMTYPE_HEAP);
		special = true;
	}
	if(!trackStarted )
	{
		size_t realSize = XMemSizeDefault(pAddress, dwAllocAttributes);
		XMemFreeDefault(pAddress, dwAllocAttributes);
		totalAllocGen -= realSize;
		return;
	}
	EnterCriticalSection(&memCS);
	if( pAddress )
	{
		size_t realSize = XMemSizeDefault(pAddress, dwAllocAttributes) - 16;

		if(trackEnable)
		{
			int sect = *(((unsigned char *)pAddress)+realSize);
			totalAllocGen -= realSize;
			trackEnable = false;
			int key = ( sect << 26 ) | realSize;
			int oldCount = allocCounts[key];
			allocCounts[key] = oldCount - 1;
			trackEnable = true;
		}
		XMemFreeDefault(pAddress, dwAllocAttributes);
	}
	LeaveCriticalSection(&memCS);
}

SIZE_T WINAPI XMemSize(
	PVOID pAddress,
	DWORD dwAllocAttributes
	)
{
	if( trackStarted )
	{
		return XMemSizeDefault(pAddress, dwAllocAttributes) - 16;
	}
	else
	{
		return XMemSizeDefault(pAddress, dwAllocAttributes);
	}
}

void DumpMem()
{
	int totalLeak = 0;
	for(AUTO_VAR(it, allocCounts.begin()); it != allocCounts.end(); it++ )
	{
		if(it->second > 0 )
		{
			app.DebugPrintf("%d %d %d %d\n",( it->first >> 26 ) & 0x3f,it->first & 0x03ffffff, it->second, (it->first & 0x03ffffff) * it->second);
			totalLeak += ( it->first & 0x03ffffff ) * it->second;
		}
	}
	app.DebugPrintf("Total %d\n",totalLeak);
}

void ResetMem()
{
	if( !trackStarted )
	{
		trackEnable = true;
		trackStarted = true;
		totalAllocGen = 0;
		InitializeCriticalSection(&memCS);
		tlsIdx = TlsAlloc();
	}
	EnterCriticalSection(&memCS);
	trackEnable = false;
	allocCounts.clear();
	trackEnable = true;
	LeaveCriticalSection(&memCS);
}

void MemSect(int section)
{
	unsigned int value = (unsigned int)TlsGetValue(tlsIdx);
	if( section == 0 ) // pop
	{
		value = (value >> 6) & 0x03ffffff;
	}
	else
	{
		value = (value << 6) | section;
	}
	TlsSetValue(tlsIdx, (LPVOID)value);
}

void MemPixStuff()
{
	const int MAX_SECT = 46;

	int totals[MAX_SECT] = {0};

	for(AUTO_VAR(it, allocCounts.begin()); it != allocCounts.end(); it++ )
	{
		if(it->second > 0 )
		{
			int sect = ( it->first >> 26 ) & 0x3f;
			int bytes = it->first & 0x03ffffff;
			totals[sect] += bytes * it->second;
		}
	}

	unsigned int allSectsTotal = 0;
	for( int i = 0; i < MAX_SECT; i++ )
	{
		allSectsTotal += totals[i];
		PIXAddNamedCounter(((float)totals[i])/1024.0f,"MemSect%d",i);
	}

	PIXAddNamedCounter(((float)allSectsTotal)/(4096.0f),"MemSect total pages");
}

#endif
