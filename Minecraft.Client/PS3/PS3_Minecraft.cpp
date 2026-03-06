// Minecraft.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#ifdef __PS3__

#include <sys/process.h>
SYS_PROCESS_PARAM(1001, 0x10000);  // thread priority, and stack size

#include <sysutil/sysutil_sysparam.h>
#include <cell/sysmodule.h>
#include <sys/spu_initialize.h>
#include <fcntl.h>
#include <unistd.h>
#include <cell/audio.h>
#include <sysutil/sysutil_gamecontent.h>
//#include <sysutil/sysutil_screenshot.h>

#include "Leaderboards\PS3LeaderboardManager.h"
#include "PS3\PS3Extras\PS3Strings.h"
#include "PS3\PS3Extras\ShutdownManager.h"
#include <sysutil/sysutil_bgmplayback.h>
#include <sysutil/sysutil_msgdialog.h>

#include "PS3/Network/SonyRemoteStorage_PS3.h"

// define to use spurs (otherwise SPU threads or raw will be used)
#define USE_SPURS
#define SPU_SELF_PATH "PS3/SPU_Tasks/"

#ifdef USE_SPURS
#include <cell/spurs.h>
#else
#include <sys/spu_image.h>
#endif

/* Encrypted ID for protected data file (*) You must edit these binaries!! */
char secureFileId[CELL_SAVEDATA_SECUREFILEID_SIZE] =
{
	0xEE, 0xA9, 0x37, 0xCC,
	0x5B, 0xD4, 0xD9, 0x0D,
	0x55, 0xED, 0x25, 0x31,
	0xFA, 0x33, 0xBD, 0xC4
};

#define MILES_SPEAKERS 8  // default to 7.1 (can be 2, 6, 7, or 8)

//#define DISABLE_MILES_SOUND


//#define HEAPINSPECTOR_PS3	1
// when defining HEAPINSPECTOR_PS3, add this line to the linker settings
// --wrap malloc --wrap free --wrap memalign --wrap calloc --wrap realloc --wrap reallocalign  --wrap _malloc_init

#if HEAPINSPECTOR_PS3
#include "HeapInspector\Server\HeapInspectorServer.h"
#include "HeapInspector\Server\PS3\HeapHooks.hpp"
#endif

#include "PS3_App.h"
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
#include "..\Common\Console_Awards_enum.h"
#include "..\..\Minecraft.Client\Options.h"
#include "Sentient\SentientManager.h"
#include "..\..\Minecraft.World\IntCache.h"
#include "..\Textures.h"
#include "Resource.h"
#include "..\..\Minecraft.World\compression.h"
#include "..\..\Minecraft.World\OldChunkStorage.h"
#include "PS3\PS3Extras\EdgeZLib.h"
#include "C4JThread_SPU.h"
#include "Common\Network\Sony\SQRNetworkManager.h"
#include "Common\UI\IUIScene_PauseMenu.h"

#include <cell/sysmodule.h>

#define THEME_NAME		"584111F70AAAAAAA"
#define THEME_FILESIZE	2797568


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

// functions for storing and converting rich presence strings from wchar to utf8
uint8_t * AddRichPresenceString(int iID);
void FreeRichPresenceStrings();

#if HEAPINSPECTOR_PS3

std::vector<HeapInspectorServer::HeapInfo> GetHeapInfo()
{
	std::vector<HeapInspectorServer::HeapInfo> result = HeapInspectorServer::GetDefaultHeapInfo();
	HeapInspectorServer::HeapInfo localHeapInfo;
	localHeapInfo.m_Description = "VRAM";
	localHeapInfo.m_Range.m_Min = 0xc0000000;
	localHeapInfo.m_Range.m_Max = localHeapInfo.m_Range.m_Min + (249*1024*1024);
	result.push_back(localHeapInfo);
	return result;
}

extern "C" void* __real__malloc_init(size_t a_Boundary, size_t a_Size);
extern "C" void* __wrap__malloc_init(size_t a_Boundary, size_t a_Size)
{
	void* result = __real__malloc_init(a_Boundary, a_Size);
	HeapInspectorServer::Initialise(GetHeapInfo(), 3000, HeapInspectorServer::WaitForConnection_Enabled);
	return result;
}

#endif // HEAPINSPECTOR_PS3

//-------------------------------------------------------------------------------------
// Time             Since fAppTime is a float, we need to keep the quadword app time
//                  as a LARGE_INTEGER so that we don't lose precision after running
//                  for a long time.
//-------------------------------------------------------------------------------------

BOOL g_bWidescreen = TRUE;
//int  g_numberOfSpeakersForMiles = 2; // number of speakers to pass to Miles, this is setup from init_audio_hardware

void DefineActions(void)
{
	// The app needs to define the actions required, and the possible mappings for these

	// Split into Menu actions, and in-game actions
	if(InputManager.IsCircleCrossSwapped())
	{
		InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_A,							_360_JOY_BUTTON_B);
		InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_OK,							_360_JOY_BUTTON_B);
		InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_B,							_360_JOY_BUTTON_A);
		InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_CANCEL,						_360_JOY_BUTTON_A);
	}
	else
	{
		InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_A,							_360_JOY_BUTTON_A);
		InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_OK,							_360_JOY_BUTTON_A);
		InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_B,							_360_JOY_BUTTON_B);
		InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_CANCEL,						_360_JOY_BUTTON_B);
	}
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_X,							_360_JOY_BUTTON_X);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_Y,							_360_JOY_BUTTON_Y);
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

	if(InputManager.IsCircleCrossSwapped())
	{
		InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_A,							_360_JOY_BUTTON_B);
		InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_OK,							_360_JOY_BUTTON_B);
		InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_B,							_360_JOY_BUTTON_A);
		InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_CANCEL,						_360_JOY_BUTTON_A);
	}
	else
	{
		InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_A,							_360_JOY_BUTTON_A);
		InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_OK,							_360_JOY_BUTTON_A);
		InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_B,							_360_JOY_BUTTON_B);
		InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_CANCEL,						_360_JOY_BUTTON_B);
	}
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_X,							_360_JOY_BUTTON_X);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_Y,							_360_JOY_BUTTON_Y);
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

	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_JUMP,					_360_JOY_BUTTON_RT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_FORWARD,				_360_JOY_BUTTON_LSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_BACKWARD,				_360_JOY_BUTTON_LSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_LEFT,					_360_JOY_BUTTON_LSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_RIGHT,					_360_JOY_BUTTON_LSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_LOOK_LEFT,				_360_JOY_BUTTON_RSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_LOOK_RIGHT,				_360_JOY_BUTTON_RSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_LOOK_UP,				_360_JOY_BUTTON_RSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_LOOK_DOWN,				_360_JOY_BUTTON_RSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_USE,					_360_JOY_BUTTON_RB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_ACTION,					_360_JOY_BUTTON_LB);
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

	if(InputManager.IsCircleCrossSwapped())
	{
		InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_A,							_360_JOY_BUTTON_B);
		InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OK,							_360_JOY_BUTTON_B);
		InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_B,							_360_JOY_BUTTON_A);
		InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_CANCEL,						_360_JOY_BUTTON_A);
	}
	else
	{
		InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_A,							_360_JOY_BUTTON_A);
		InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OK,							_360_JOY_BUTTON_A);
		InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_B,							_360_JOY_BUTTON_B);
		InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_CANCEL,						_360_JOY_BUTTON_B);
	}
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_X,							_360_JOY_BUTTON_X);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_Y,							_360_JOY_BUTTON_Y);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_UP,							_360_JOY_BUTTON_DPAD_UP | _360_JOY_BUTTON_LSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_DOWN,						_360_JOY_BUTTON_DPAD_DOWN | _360_JOY_BUTTON_LSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_LEFT,						_360_JOY_BUTTON_DPAD_LEFT | _360_JOY_BUTTON_LSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_RIGHT,						_360_JOY_BUTTON_DPAD_RIGHT | _360_JOY_BUTTON_LSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_PAGEUP,						_360_JOY_BUTTON_DPAD_UP | _360_JOY_BUTTON_LB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_PAGEDOWN,					_360_JOY_BUTTON_RT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_RIGHT_SCROLL,				_360_JOY_BUTTON_RB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_LEFT_SCROLL,					_360_JOY_BUTTON_LB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_PAUSEMENU,					_360_JOY_BUTTON_START);

	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_STICK_PRESS,					_360_JOY_BUTTON_LTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OTHER_STICK_PRESS,			_360_JOY_BUTTON_RTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OTHER_STICK_UP,				_360_JOY_BUTTON_RSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OTHER_STICK_DOWN,			_360_JOY_BUTTON_RSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OTHER_STICK_LEFT,			_360_JOY_BUTTON_RSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OTHER_STICK_RIGHT,			_360_JOY_BUTTON_RSTICK_RIGHT);

	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_JUMP,					_360_JOY_BUTTON_LB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_FORWARD,				_360_JOY_BUTTON_LSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_BACKWARD,				_360_JOY_BUTTON_LSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_LEFT,					_360_JOY_BUTTON_LSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_RIGHT,					_360_JOY_BUTTON_LSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_LOOK_LEFT,				_360_JOY_BUTTON_RSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_LOOK_RIGHT,				_360_JOY_BUTTON_RSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_LOOK_UP,				_360_JOY_BUTTON_RSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_LOOK_DOWN,				_360_JOY_BUTTON_RSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_USE,					_360_JOY_BUTTON_RB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_ACTION,					_360_JOY_BUTTON_A);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_RIGHT_SCROLL,			_360_JOY_BUTTON_DPAD_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_LEFT_SCROLL,			_360_JOY_BUTTON_DPAD_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_INVENTORY,				_360_JOY_BUTTON_Y);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_PAUSEMENU,				_360_JOY_BUTTON_START);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_DROP,					_360_JOY_BUTTON_B);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_SNEAK_TOGGLE,			_360_JOY_BUTTON_LT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_CRAFTING,				_360_JOY_BUTTON_X);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_RENDER_THIRD_PERSON,	_360_JOY_BUTTON_LTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_GAME_INFO,				_360_JOY_BUTTON_BACK);

	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_DPAD_LEFT,				_360_JOY_BUTTON_DPAD_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_DPAD_RIGHT,				_360_JOY_BUTTON_DPAD_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_DPAD_UP,				_360_JOY_BUTTON_DPAD_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_DPAD_DOWN,				_360_JOY_BUTTON_DPAD_DOWN);
}


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


ID3D11Device*           g_pd3dDevice = NULL;
ID3D11DeviceContext*    g_pImmediateContext = NULL;
IDXGISwapChain*         g_pSwapChain = NULL;
bool					g_bBootedFromInvite = false;

//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{


    return S_OK;
}

//--------------------------------------------------------------------------------------
// Render the frame
//--------------------------------------------------------------------------------------
void Render()
{

}

//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice()
{

}

// simple function to load a file completely into memory
static void * load_file( char const * name )
{
	void * ret;
	int f;
	unsigned int size;

	f = open( name, O_RDONLY );
	if ( f == -1 ) return 0;

	size = lseek( f, 0, SEEK_END );

	ret = malloc( size + 127 );
	if ( ret == 0 )
	{
		close( f );
		return 0;
	}

	ret = (void*) ( ( ( (unsigned int) ret ) + 127 ) & ~127 );

	lseek( f, 0, SEEK_SET );
	read( f, ret, size );
	close( f );

	return ret;
}



void debugSaveGameDirect()
{

	C4JThread* thread = new C4JThread(&IUIScene_PauseMenu::SaveWorldThreadProc, NULL, "debugSaveGameDirect");
	thread->Run();
	thread->WaitForCompletion(1000);
}

void LoadSysModule(uint16_t module, const char* moduleName)
{
	int ret = cellSysmoduleLoadModule(module);
	if(ret != CELL_OK)
	{
#ifndef _CONTENT_PACKAGE
		printf("Error cellSysmoduleLoadModule %s failed (%d) \n", moduleName, ret );
#endif
		assert(0);
	}
}

#define LOAD_PS3_MODULE(m) LoadSysModule(m, #m)

int simpleMessageBoxCallback(	UINT uiTitle, UINT uiText,
								UINT *uiOptionA, UINT uiOptionC, DWORD dwPad,
								int(*Func) (LPVOID,int,const C4JStorage::EMessageResult),
								LPVOID lpParam )
{
	ui.RequestMessageBox(	uiTitle, uiText,
							uiOptionA, uiOptionC, dwPad,
							Func, lpParam, app.GetStringTable(),
							NULL, 0
						);

	return 0;
}

void RegisterAwardsWithProfileManager()
{
	// register the awards
	ProfileManager.RegisterAward(eAward_TakingInventory,	ACHIEVEMENT_01, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_GettingWood,		ACHIEVEMENT_02, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_Benchmarking,		ACHIEVEMENT_03, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_TimeToMine,			ACHIEVEMENT_04, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_HotTopic,			ACHIEVEMENT_05, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_AquireHardware,		ACHIEVEMENT_06, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_TimeToFarm,			ACHIEVEMENT_07, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_BakeBread,			ACHIEVEMENT_08, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_TheLie,				ACHIEVEMENT_09, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_GettingAnUpgrade,	ACHIEVEMENT_10, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_DeliciousFish,		ACHIEVEMENT_11, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_OnARail,			ACHIEVEMENT_12, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_TimeToStrike,		ACHIEVEMENT_13, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_MonsterHunter,		ACHIEVEMENT_14, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_CowTipper,			ACHIEVEMENT_15, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_WhenPigsFly,		ACHIEVEMENT_16, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_LeaderOfThePack,	ACHIEVEMENT_17, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_MOARTools,			ACHIEVEMENT_18, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_DispenseWithThis,	ACHIEVEMENT_19, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_InToTheNether,		ACHIEVEMENT_20, eAwardType_Achievement);

	ProfileManager.RegisterAward(eAward_snipeSkeleton,		ACHIEVEMENT_21, eAwardType_Achievement); // 'Sniper Duel'
	ProfileManager.RegisterAward(eAward_diamonds,			ACHIEVEMENT_22, eAwardType_Achievement); // 'DIAMONDS!'
	ProfileManager.RegisterAward(eAward_ghast,				ACHIEVEMENT_23, eAwardType_Achievement); // 'Return To Sender'
	ProfileManager.RegisterAward(eAward_blazeRod,			ACHIEVEMENT_24, eAwardType_Achievement); // 'Into Fire'
	ProfileManager.RegisterAward(eAward_potion,				ACHIEVEMENT_25, eAwardType_Achievement); // 'Local Brewery'
	ProfileManager.RegisterAward(eAward_theEnd,				ACHIEVEMENT_26, eAwardType_Achievement); // 'The End?'
	ProfileManager.RegisterAward(eAward_winGame,			ACHIEVEMENT_27, eAwardType_Achievement); // 'The End.'
	ProfileManager.RegisterAward(eAward_enchantments,		ACHIEVEMENT_28, eAwardType_Achievement); // 'Enchanter'

#if 0
	ProfileManager.RegisterAward(eAward_mine100Blocks,		GAMER_PICTURE_GAMERPIC1,			eAwardType_GamerPic,false,app.GetStringTable(),IDS_AWARD_TITLE,IDS_AWARD_GAMERPIC1,IDS_CONFIRM_OK);
	ProfileManager.RegisterAward(eAward_kill10Creepers,		GAMER_PICTURE_GAMERPIC2,			eAwardType_GamerPic,false,app.GetStringTable(),IDS_AWARD_TITLE,IDS_AWARD_GAMERPIC2,IDS_CONFIRM_OK);

	ProfileManager.RegisterAward(eAward_eatPorkChop,		AVATARASSETAWARD_PORKCHOP_TSHIRT,	eAwardType_AvatarItem,false,app.GetStringTable(),IDS_AWARD_TITLE,IDS_AWARD_AVATAR1,IDS_CONFIRM_OK);
	ProfileManager.RegisterAward(eAward_play100Days,		AVATARASSETAWARD_WATCH,				eAwardType_AvatarItem,false,app.GetStringTable(),IDS_AWARD_TITLE,IDS_AWARD_AVATAR2,IDS_CONFIRM_OK);
	ProfileManager.RegisterAward(eAward_arrowKillCreeper,	AVATARASSETAWARD_CAP,				eAwardType_AvatarItem,false,app.GetStringTable(),IDS_AWARD_TITLE,IDS_AWARD_AVATAR3,IDS_CONFIRM_OK);

	ProfileManager.RegisterAward(eAward_socialPost,			0,									eAwardType_Theme,false,app.GetStringTable(),IDS_AWARD_TITLE,IDS_AWARD_THEME,IDS_CONFIRM_OK,THEME_NAME,THEME_FILESIZE);
#endif
	// Rich Presence init - number of presences, number of contexts
	//printf("Rich presence strings are hard coded on PS3 for now, must change this!\n");
	ProfileManager.RichPresenceInit(-1,-1); // 4J-JEV - Presence and Context count not used.
	ProfileManager.SetRichPresenceSettingFn(SQRNetworkManager_PS3::SetRichPresence);
	char *pchRichPresenceString;

	pchRichPresenceString=(char *)AddRichPresenceString(IDS_RICHPRESENCE_GAMESTATE);
	ProfileManager.RichPresenceRegisterContext(CONTEXT_GAME_STATE, pchRichPresenceString);
	pchRichPresenceString=(char *)AddRichPresenceString(IDS_RICHPRESENCE_IDLE);
	ProfileManager.RichPresenceRegisterPresenceString(CONTEXT_PRESENCE_IDLE,					pchRichPresenceString);
	pchRichPresenceString=(char *)AddRichPresenceString(IDS_RICHPRESENCE_MENUS);
	ProfileManager.RichPresenceRegisterPresenceString(CONTEXT_PRESENCE_MENUS,					pchRichPresenceString);
	pchRichPresenceString=(char *)AddRichPresenceString(IDS_RICHPRESENCE_MULTIPLAYER);
	ProfileManager.RichPresenceRegisterPresenceString(CONTEXT_PRESENCE_MULTIPLAYER,				pchRichPresenceString);
	pchRichPresenceString=(char *)AddRichPresenceString(IDS_RICHPRESENCE_MULTIPLAYEROFFLINE);
	ProfileManager.RichPresenceRegisterPresenceString(CONTEXT_PRESENCE_MULTIPLAYEROFFLINE,		pchRichPresenceString);
	pchRichPresenceString=(char *)AddRichPresenceString(IDS_RICHPRESENCE_MULTIPLAYER_1P);
	ProfileManager.RichPresenceRegisterPresenceString(CONTEXT_PRESENCE_MULTIPLAYER_1P,			pchRichPresenceString);
	pchRichPresenceString=(char *)AddRichPresenceString(IDS_RICHPRESENCE_MULTIPLAYER_1POFFLINE);
	ProfileManager.RichPresenceRegisterPresenceString(CONTEXT_PRESENCE_MULTIPLAYER_1POFFLINE,	pchRichPresenceString);

	pchRichPresenceString=(char *)AddRichPresenceString(IDS_RICHPRESENCESTATE_BLANK);
	ProfileManager.RichPresenceRegisterContextString(CONTEXT_GAME_STATE, CONTEXT_GAME_STATE_BLANK,				pchRichPresenceString);
	pchRichPresenceString=(char *)AddRichPresenceString(IDS_RICHPRESENCESTATE_RIDING_PIG);
	ProfileManager.RichPresenceRegisterContextString(CONTEXT_GAME_STATE, CONTEXT_GAME_STATE_RIDING_PIG,			pchRichPresenceString);
	pchRichPresenceString=(char *)AddRichPresenceString(IDS_RICHPRESENCESTATE_RIDING_MINECART);
	ProfileManager.RichPresenceRegisterContextString(CONTEXT_GAME_STATE, CONTEXT_GAME_STATE_RIDING_MINECART,	pchRichPresenceString);
	pchRichPresenceString=(char *)AddRichPresenceString(IDS_RICHPRESENCESTATE_BOATING);
	ProfileManager.RichPresenceRegisterContextString(CONTEXT_GAME_STATE, CONTEXT_GAME_STATE_BOATING,			pchRichPresenceString);
	pchRichPresenceString=(char *)AddRichPresenceString(IDS_RICHPRESENCESTATE_FISHING);
	ProfileManager.RichPresenceRegisterContextString(CONTEXT_GAME_STATE, CONTEXT_GAME_STATE_FISHING,			pchRichPresenceString);
	pchRichPresenceString=(char *)AddRichPresenceString(IDS_RICHPRESENCESTATE_CRAFTING);
	ProfileManager.RichPresenceRegisterContextString(CONTEXT_GAME_STATE, CONTEXT_GAME_STATE_CRAFTING,			pchRichPresenceString);
	pchRichPresenceString=(char *)AddRichPresenceString(IDS_RICHPRESENCESTATE_FORGING);
	ProfileManager.RichPresenceRegisterContextString(CONTEXT_GAME_STATE, CONTEXT_GAME_STATE_FORGING,			pchRichPresenceString);
	pchRichPresenceString=(char *)AddRichPresenceString(IDS_RICHPRESENCESTATE_NETHER);
	ProfileManager.RichPresenceRegisterContextString(CONTEXT_GAME_STATE, CONTEXT_GAME_STATE_NETHER,				pchRichPresenceString);
	pchRichPresenceString=(char *)AddRichPresenceString(IDS_RICHPRESENCESTATE_CD);
	ProfileManager.RichPresenceRegisterContextString(CONTEXT_GAME_STATE, CONTEXT_GAME_STATE_CD,					pchRichPresenceString);
	pchRichPresenceString=(char *)AddRichPresenceString(IDS_RICHPRESENCESTATE_MAP);
	ProfileManager.RichPresenceRegisterContextString(CONTEXT_GAME_STATE, CONTEXT_GAME_STATE_MAP,				pchRichPresenceString);

	pchRichPresenceString=(char *)AddRichPresenceString(IDS_RICHPRESENCESTATE_ENCHANTING);
	ProfileManager.RichPresenceRegisterContextString(CONTEXT_GAME_STATE, CONTEXT_GAME_STATE_ENCHANTING,			pchRichPresenceString);
	pchRichPresenceString=(char *)AddRichPresenceString(IDS_RICHPRESENCESTATE_BREWING);
	ProfileManager.RichPresenceRegisterContextString(CONTEXT_GAME_STATE, CONTEXT_GAME_STATE_BREWING,			pchRichPresenceString);
	pchRichPresenceString=(char *)AddRichPresenceString(IDS_RICHPRESENCESTATE_ANVIL);
	ProfileManager.RichPresenceRegisterContextString(CONTEXT_GAME_STATE, CONTEXT_GAME_STATE_ANVIL,				pchRichPresenceString);
	pchRichPresenceString=(char *)AddRichPresenceString(IDS_RICHPRESENCESTATE_TRADING);
	ProfileManager.RichPresenceRegisterContextString(CONTEXT_GAME_STATE, CONTEXT_GAME_STATE_TRADING,			pchRichPresenceString);

}

int LoadSysModules()
{
	cellSysmoduleInitialize();

	LOAD_PS3_MODULE(CELL_SYSMODULE_FS );
	// 4J Stu - For Iggy
	LOAD_PS3_MODULE(CELL_SYSMODULE_RTC);
	// string conversions
	LOAD_PS3_MODULE(CELL_SYSMODULE_L10N );
	// on-screen keyboard
	LOAD_PS3_MODULE(CELL_SYSMODULE_SYSUTIL_OSK_EXT );
	// Networking
	//First load the Network module PRX
	LOAD_PS3_MODULE(CELL_SYSMODULE_NET);
	//Load the libnetctl network connection information module PRX
	LOAD_PS3_MODULE(CELL_SYSMODULE_NETCTL);
	LOAD_PS3_MODULE(CELL_SYSMODULE_SYSUTIL_NP2 );
	LOAD_PS3_MODULE(CELL_SYSMODULE_RUDP );
	LOAD_PS3_MODULE(CELL_SYSMODULE_HTTP);
	LOAD_PS3_MODULE(CELL_SYSMODULE_HTTPS);
	LOAD_PS3_MODULE(CELL_SYSMODULE_HTTP_UTIL);
	LOAD_PS3_MODULE(CELL_SYSMODULE_SSL);
	LOAD_PS3_MODULE(CELL_SYSMODULE_JPGDEC);
	LOAD_PS3_MODULE(CELL_SYSMODULE_SYSUTIL_AVCHAT2);

	// 4J-PB - Get launch parameters
	LOAD_PS3_MODULE(CELL_SYSMODULE_SYSUTIL_GAME);
	LOAD_PS3_MODULE(CELL_SYSMODULE_SYSUTIL_NP_COMMERCE2);

	LOAD_PS3_MODULE(CELL_SYSMODULE_AVCONF_EXT);

	LOAD_PS3_MODULE(CELL_SYSMODULE_SYSUTIL_SAVEDATA);

	unsigned int uiType ;
	unsigned int uiAttributes ;
	CellGameContentSize size,sizeBD;
	char *dirName=getDirName();
	char *contentInfoPath = getConsoleHomePath();
	char *usrdirPath= getUsrDirPath();
	char *contentInfoPathBDPatch = getConsoleHomePathBDPatch();
	char *usrdirPathBDPatch= getUsrDirPathBDPatch();
	int ret;
	bool bBootedFromBDPatch=false;

	memset(&size, 0x00, sizeof(CellGameContentSize));

	ret = cellGameBootCheck( &uiType, &uiAttributes, &size, dirName ) ;
	if(ret < 0)
	{
		DEBUG_PRINTF("cellGameBootCheck() Error: 0x%x\n", ret);
	}
	else
	{
		DEBUG_PRINTF("cellGameBootCheck() OK\n");
		DEBUG_PRINTF("  get_type = [%d] get_attributes = [0x%08x] dirName = [%s]\n", uiType, uiAttributes, dirName);
		DEBUG_PRINTF("  hddFreeSizeKB = [%d] sizeKB = [%d] sysSizeKB = [%d]\n", size.hddFreeSizeKB, size.sizeKB, size.sysSizeKB);
	}

	if (uiAttributes & CELL_GAME_ATTRIBUTE_INVITE_MESSAGE)
	{
		g_bBootedFromInvite = true;
		DEBUG_PRINTF("Booted from invite\n");
	}
	if (uiAttributes & CELL_GAME_ATTRIBUTE_CUSTOM_DATA_MESSAGE)
	{
		DEBUG_PRINTF("Booted from custom data\n");
		//bootedFromCustomData = true;
	}
	if(uiAttributes&CELL_GAME_ATTRIBUTE_PATCH)
	{
		// Booted from a patch (only for a disc boot game)
		// To access patch game data, obtain its path by separately using cellGamePatchCheck().
		bBootedFromBDPatch=true;
		app.SetBootedFromDiscPatch();
	}
	// true if booting from disc, false if booting from HDD
	StorageManager.SetBootTypeDisc(uiType == CELL_GAME_GAMETYPE_DISC);

	StorageManager.SetMessageBoxCallback(&simpleMessageBoxCallback);

	cellGameContentPermit(contentInfoPath,usrdirPath);
	DEBUG_PRINTF("contentInfoPath - %s\n",contentInfoPath);
	DEBUG_PRINTF("usrdirPath - %s\n",usrdirPath);

	ret=cellGamePatchCheck(&sizeBD,NULL);
	if(ret < 0)
	{
		DEBUG_PRINTF("cellGamePatchCheck() Error: 0x%x\n", ret);
	}
	else
	{
		DEBUG_PRINTF("cellGamePatchCheck() OK - we were booted from a disc patch!\n");

		cellGameContentPermit(contentInfoPathBDPatch,usrdirPathBDPatch);
		DEBUG_PRINTF("contentInfoPath - %s\n",contentInfoPathBDPatch);
		DEBUG_PRINTF("usrdirPath - %s\n",usrdirPathBDPatch);

		app.SetDiscPatchUsrDir(usrdirPathBDPatch);
	}

	if(uiAttributes&CELL_GAME_ATTRIBUTE_DEBUG)
	{
		DEBUG_PRINTF("Booted from debugger\n");
#ifndef _RELEASE_FOR_ART
		strcat(contentInfoPath,"/Minecraft.Client");
		strcpy(usrdirPath,contentInfoPath);
#endif
	}
	else
	{
		//strcat(contentInfoPath,"/");
		//strcat(usrdirPath,"/");
	}

	return size.hddFreeSizeKB;
}




int main()
{
	ShutdownManager::Initialise();

	sys_memory_info_t mem_info;
	sys_memory_get_user_memory_size(&mem_info);

#ifndef _CONTENT_PACKAGE
	printf("------------------------------------------------------\n");
	printf("------------------------------------------------------\n");
	printf("total_user_memory : %.02f\n", mem_info.total_user_memory / (1024.0f*1024.0f));
	printf("available_user_memory : %.02f", mem_info.available_user_memory / (1024.0f*1024.0f));
	printf("------------------------------------------------------\n");
	printf("------------------------------------------------------\n");
#endif

	//
	// initialize the ps3
	//

	int ihddFreeSizeKB=LoadSysModules();
	ProfileManager.SetHDDFreeKB(ihddFreeSizeKB);
	// Sony request that we have enough space for a save game, so people don't get hit with this when they first save. Going to make the space 20MB
	ProfileManager.SetMinSaveKB(20000);

	cellSysutilEnableBgmPlayback();

	C4JThread_SPU::initSPURS();

#ifndef DISABLE_MILES_SOUND

#ifdef USE_SPURS
	void * spurs_info[ 2 ];
	spurs_info[ 0 ] = C4JThread_SPU::getSpurs2();
	extern const CellSpursTaskBinInfo _binary_task_mssspurs_elf_taskbininfo;
	spurs_info[1] = (void*)_binary_task_mssspurs_elf_taskbininfo.eaElf;

	//
	// load it on an SPU
	//

	if ( !MilesStartAsyncThread( 0, spurs_info ) )
	{
#ifndef _CONTENT_PACKAGE
		printf( "Error loading SPU image.\n" );
#endif
		return( 5 );
	}

#else

	//
	// Start an SPU thread on SPU threads or RAW
	//

	{
		sys_spu_image_t spu_image;
		char selfPath[256];
		sprintf(selfPath,"%s%s%s", getUsrDirPath(), SPU_SELF_PATH, "mssraw.elf" );

		if ( sys_spu_image_open( &spu_image, selfPath ) != CELL_OK )
		{
			printf( "Error loading SPU image.\n" );
			return( 4 );
		}

		if ( !MilesStartAsyncThread( 0, &spu_image ) )
		{
			printf( "Error loading SPU image.\n" );
			return( 5);
		}

		//
		// now close the image, since the system caches it for us
		//

		sys_spu_image_close( &spu_image );
	}

#endif


	//
	// now initialize libAudio
	//

	cellAudioInit();

#endif // DISABLE_MILES_SOUND

	// 4J-JEV: Moved this here in case some archived files are compressed.
	Compression::CreateNewThreadStorage();

	EdgeZLib::Init(C4JThread_SPU::getSpurs2());

	RenderManager.Initialise();

	// Read the file containing the product codes
	if(app.ReadProductCodes()==FALSE)
	{
		// can't continue
		app.FatalLoadError();
	}

	StorageManager.SetDLCProductCode(app.GetProductCode(),app.GetDiscProductCode());
	if(StorageManager.GetBootTypeDisc())
	{
		StorageManager.SetBDPatchUsrDir(app.GetDiscPatchUsrDir());
	}
	StorageManager.SetProductUpgradeKey(app.GetUpgradeKey());
	ProfileManager.SetServiceID(app.GetCommerceCategory());

	switch(app.GetProductSKU())
	{
	case e_sku_SCEE:
		// 4J-PB - need to be online to do this check, so let's stick with the 7+, and move this

		if(StorageManager.GetBootTypeDisc())
		{
			// set Europe age, then hone down specific countries
			ProfileManager.SetMinimumAge(7,0); // PEGI 7+
			ProfileManager.SetGermanyMinimumAge(6); // USK 6+
			ProfileManager.SetAustraliaMinimumAge(8); // PG rating has no age, but for some reason the testers are saying it's 8
			ProfileManager.SetRussiaMinimumAge(6);
		}
		else
		{
			// PEGI 7+
			ProfileManager.SetMinimumAge(7,0);
		}
		break;
	case e_sku_SCEA:
		// ESRB EVERYONE 10+
		ProfileManager.SetMinimumAge(10,1);
		break;
	case e_sku_SCEJ:
		// CERO A
		ProfileManager.SetMinimumAge(0,2);
		break;
	}

	// Initialise NpBasic - this used to be in the network manager but other things need this so moved out on its own
	void *np_pool = memalign( 8, SQRNetworkManager::NP_POOL_SIZE );
	int ret = sceNp2Init( SQRNetworkManager::NP_POOL_SIZE, np_pool );
	if( ret < 0 )
	{
		assert(0);
	}


	app.loadMediaArchive();

	app.DebugPrintf("+++Main - after app.loadMediaArchive\n");

	app.loadStringTable();

	// Set the number of possible joypad layouts that the user can switch between, and the number of actions
	// Need to init Input before UI so we know what the Circle/Cross state is
	InputManager.Initialise(1,3,MINECRAFT_ACTION_MAX, ACTION_MAX_MENU);

	// Set the default joypad action mappings for Minecraft
	DefineActions();
	InputManager.SetJoypadMapVal(0,0);
	InputManager.SetKeyRepeatRate(0.3f,0.2f);

	if(RenderManager.IsWidescreen())
		ui.init(1280,720);
	else
		ui.init(1280,480);

	app.CommerceInit(); //  MGH - moved this here so GetCommerce isn't NULL
	// 4J-PB - Kick of the check for trial or full version - requires ui to be initialised
	app.GetCommerce()->CheckForTrialUpgradeKey();

	static bool bTrialTimerDisplayed=true;


	////////////////
	// Initialise //
	////////////////

#if 0
	// 4J Stu - XACT was creating these automatically, but we need them for QNet. The setup params
	// are just copied from a sample app and may need changed for our purposes
	// Start XAudio2
    hr = XAudio2Create( &g_pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR );
    if( FAILED( hr ) )
    {
        app.DebugPrintf( "Initializing XAudio2 failed (err = 0x%08x)!\n", hr );
		app.FatalLoadError();
    }

    // Create an XAudio2 mastering voice (utilized by XHV2 when voice data is mixed to main speakers)
    hr = g_pXAudio2->CreateMasteringVoice(&g_pXAudio2MasteringVoice, XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE, 0, 0, NULL);
    if ( FAILED( hr ) )
    {
        app.DebugPrintf( "Creating XAudio2 mastering voice failed (err = 0x%08x)!\n", hr );
		app.FatalLoadError();
    }
#endif

	app.InitTime();


	// Initialise the profile manager with the game Title ID, Offer ID, a profile version number, and the number of profile values and settings
	ProfileManager.Initialise(SQRNetworkManager_PS3::GetSceNpCommsId(),
		SQRNetworkManager_PS3::GetSceNpCommsSig(),
		PROFILE_VERSION_10,
		NUM_PROFILE_VALUES,
		NUM_PROFILE_SETTINGS,
		dwProfileSettingsA,
		app.GAME_DEFINED_PROFILE_DATA_BYTES*XUSER_MAX_COUNT,
		&app.uiGameDefinedDataChangedBitmask);

	app.DebugPrintf("+++Main - after ProfileManager.Initialise\n");

	// register the awards
	RegisterAwardsWithProfileManager();

	// register the get string function with the profile lib, so it can be called within the lib

	ProfileManager.SetGetStringFunc(&CConsoleMinecraftApp::GetString);
	ProfileManager.SetPlayerListTitleID(IDS_PLAYER_LIST_TITLE);
	ProfileManager.SetSignInQuestionID(IDS_SIGNIN_PSN);

	// initialise the storage manager with a callback for displaying the saving message
	StorageManager.Init(0,app.GetString(IDS_DEFAULT_SAVENAME),"savegame.dat",FIFTY_ONE_MB,&CConsoleMinecraftApp::DisplaySavingMessage,(LPVOID)&app,"");

#if 0

	// Set up the global title storage path
	StorageManager.StoreTMSPathName();
#endif

	// MGH - I've had to rearrange some of the ordering here, as I was getting conflicts with the sysutils for trophies and the upgrade key

	//app.FatalLoadError();


	// set a function to be called when there's a sign in change, so we can exit a level if the primary player signs out
	ProfileManager.SetSignInChangeCallback(&CConsoleMinecraftApp::SignInChangeCallback,(LPVOID)&app);

#if 0
	// set a function to be called when the ethernet is disconnected, so we can back out if required
	ProfileManager.SetNotificationsCallback(&CConsoleMinecraftApp::NotificationsCallback,(LPVOID)&app);
#endif
	// Set a callback for the default player options to be set - when there is no profile data for the player

	StorageManager.SetDefaultOptionsCallback(&CConsoleMinecraftApp::DefaultOptionsCallback,(LPVOID)&app);
	StorageManager.SetOptionsDataCallback(&CConsoleMinecraftApp::OptionsDataCallback,(LPVOID)&app);

	// Set a callback to deal with old profile versions needing updated to new versions
#if 0
	ProfileManager.SetOldProfileVersionCallback(&CConsoleMinecraftApp::OldProfileVersionCallback,(LPVOID)&app);

	// Set a callback for when there is a read error on profile data
	ProfileManager.SetProfileReadErrorCallback(&CConsoleMinecraftApp::ProfileReadErrorCallback,(LPVOID)&app);

	// QNet needs to be setup after profile manager, as we do not want its Notify listener to handle
	// XN_SYS_SIGNINCHANGED notifications. This does mean that we need to have a callback in the
	// ProfileManager for XN_LIVE_INVITE_ACCEPTED for QNet.
#endif // 0

	SonyRemoteStorage_PS3* pRemote = (SonyRemoteStorage_PS3*)app.getRemoteStorage();
	pRemote->SetSecureID(secureFileId);
	pRemote->SetServiceID(app.GetCommerceCategory());

	// Before using any save/load on PS3, we need a secure id set
	StorageManager.InitialiseProfileData(PROFILE_VERSION_10,
		NUM_PROFILE_VALUES,
		NUM_PROFILE_SETTINGS,
		dwProfileSettingsA,
		app.GAME_DEFINED_PROFILE_DATA_BYTES*XUSER_MAX_COUNT,
		&app.uiGameDefinedDataChangedBitmask);

	app.DebugPrintf("+++Main - after InitialiseProfileData\n");


	StorageManager.SetSecureID(secureFileId);
	// set the name of the folder on the XMB that the saves are grouped in
	// 4J-PB - TRC that we need to be able to see the BD in a different titled save from the digital game
	if(StorageManager.GetBootTypeDisc())
	{
		wstring wsTemp = app.GetString(IDS_GAMENAME);
		WCHAR wchTemp[64];
		wsTemp.append(L" (");
		mbstowcs(wchTemp,app.GetDiscProductCode(),64);
		wsTemp.append(wchTemp);
		wsTemp.append(L")");
		StorageManager.SetGameSaveFolderTitle((WCHAR *)(wsTemp.c_str()));
	}
	else
	{
		StorageManager.SetGameSaveFolderTitle((WCHAR *)app.GetString(IDS_GAMENAME));//"Minecraft: PlayStation�3 Edition");//GAMENAME);
	}
	StorageManager.SetSaveCacheFolderTitle((WCHAR *)app.GetString(IDS_SAVECACHEFILE));//"Minecraft: PlayStation�3 Edition");//GAMENAME);
	StorageManager.SetOptionsFolderTitle((WCHAR *)app.GetString(IDS_OPTIONSFILE));//"Minecraft: PlayStation�3 Edition");//GAMENAME);
	StorageManager.SetGameSaveFolderPrefix(app.GetSaveFolderPrefix());
	StorageManager.SetMaxSaves(99);
	byteArray baOptionsIcon = app.getArchiveFile(L"DefaultOptionsImage320x176.png");
	byteArray baSaveThumbnail = app.getArchiveFile(L"DefaultSaveThumbnail64x64.png");
	byteArray baSaveImage = app.getArchiveFile(L"DefaultSaveImage320x176.png");

	uint8_t *szTemp = mallocAndCreateUTF8ArrayFromString(IDS_NODEVICE_DECLINE);
	StorageManager.SetContinueWithoutSavingMessage((char *)szTemp);
	free(szTemp);
	StorageManager.SetDefaultImages((PBYTE)baOptionsIcon.data, baOptionsIcon.length,(PBYTE)baSaveImage.data, baSaveImage.length,(PBYTE)baSaveThumbnail.data, baSaveThumbnail.length);

	if(baOptionsIcon.data!=NULL)
	{
		delete [] baOptionsIcon.data;
	}

	if(baSaveThumbnail.data!=NULL)
	{
		delete [] baSaveThumbnail.data;
	}

	if(baSaveImage.data!=NULL)
	{
		delete [] baSaveImage.data;
	}

	wstring wsName=L"Graphics\\SaveChest.png";
	byteArray baSaveLoadIcon = app.getArchiveFile(wsName);
	if(baSaveLoadIcon.data!=NULL)
	{
		StorageManager.SetSaveLoadIcon((PBYTE)baSaveLoadIcon.data, baSaveLoadIcon.length);
		delete [] baSaveLoadIcon.data;
	}

	// defaults
	StorageManager.SetSaveTitle(L"Default Save");

	// debug switch to trial version
//	ProfileManager.SetDebugFullOverride(true);

	//ProfileManager.AddDLC(2);
	StorageManager.SetDLCPackageRoot("DLCDrive");
	StorageManager.RegisterMarketplaceCountsCallback(&CConsoleMinecraftApp::MarketplaceCountsCallback,(LPVOID)&app);

	// Initialise TLS for tesselator, for this main thread
	Tesselator::CreateNewThreadStorage(1024*1024);
	// Initialise TLS for AABB and Vec3 pools, for this main thread
	AABB::CreateNewThreadStorage();
	Vec3::CreateNewThreadStorage();
	IntCache::CreateNewThreadStorage();
	OldChunkStorage::CreateNewThreadStorage();
	Level::enableLightingCache();
	Tile::CreateNewThreadStorage();

	Minecraft::main();
	Minecraft *pMinecraft=Minecraft::GetInstance();

	app.InitGameSettings(); // - allocates the memory for the game settings
	// read the options here for controller 0 - this won't actually be actioned until a storagemanager tick later
	StorageManager.ReadFromProfile(0);

	// set the default profile values
	// 4J-PB - InitGameSettings already does this
	/*for(int i=0;i<XUSER_MAX_COUNT;i++)
	{
#ifdef __PS3__
		app.SetDefaultOptions(StorageManager.GetDashboardProfileSettings(i),i);
#else
		app.SetDefaultOptions(ProfileManager.GetDashboardProfileSettings(i),i);
#endif
	}*/

	// set the achievement text for a trial achievement, now we have the string table loaded
	ProfileManager.SetTrialTextStringTable(NULL, IDS_CONFIRM_OK, IDS_CONFIRM_CANCEL);
	ProfileManager.SetTrialAwardText(eAwardType_Achievement,IDS_UNLOCK_TITLE,IDS_UNLOCK_ACHIEVEMENT_TEXT);
#ifndef __PS3__
	ProfileManager.SetTrialAwardText(eAwardType_GamerPic,IDS_UNLOCK_TITLE,IDS_UNLOCK_GAMERPIC_TEXT);
	ProfileManager.SetTrialAwardText(eAwardType_AvatarItem,IDS_UNLOCK_TITLE,IDS_UNLOCK_AVATAR_TEXT);
#endif
	ProfileManager.SetTrialAwardText(eAwardType_Theme,IDS_UNLOCK_TITLE,IDS_UNLOCK_THEME_TEXT);
	ProfileManager.SetUpsellCallback(&app.UpsellReturnedCallback,&app);

	app.InitialiseTips();


	// set the callback for losing input (when a system type ui is up)

	InputManager.SetLostInputControlCallback(&UIController::SetSystemUIShowing,&ui);


#ifndef _FINAL_BUILD
#ifndef _DEBUG
	#pragma message(__LOC__"Need to define the _FINAL_BUILD before submission")
#endif
#endif



#if 0

	DWORD initData=0;
	// Set the default sound levels
	pMinecraft->options->set(Options::Option::MUSIC,1.0f);
	pMinecraft->options->set(Options::Option::SOUND,1.0f);

	app.NavigateToScene(XUSER_INDEX_ANY,eUIScene_Intro,&initData);
#endif



	LeaderboardManager::Instance()->OpenSession();
	//LeaderboardManager::Instance()->ReadStats_TopRank(DebugReadListener::m_instance, 0, LeaderboardManager::eStatsType_Travelling, 1, 1);

	// 4J-PB - we really need to know at this point if we are playing the trial or full game, so sleep until we know
	while(app.GetCommerce()->LicenseChecked()==false)
	{
		cellSysutilCheckCallback();
		Sleep(50);
	}
	// wait for the trophy init to complete - nonblocking semaphore
	while((ProfileManager.tryWaitTrophyInitComplete()<0)&&ShutdownManager::ShouldRun(ShutdownManager::eMainThread))
	{
		// keep looping
		RenderManager.Tick();
		RenderManager.Present();
		cellSysutilCheckCallback();
		Sleep(50);
	}

	app.DebugPrintf("+++Main - after tryWaitTrophyInitComplete\n");

	if(ShutdownManager::ShouldRun(ShutdownManager::eMainThread))
	{
		g_NetworkManager.Initialise();
		g_NetworkManager.SetLocalGame(true);
	}

	//ProfileManager.WaitTrophyInitComplete();

	// 4J-PB - get the chat/UGC restriction flag here so we can stop online games, and content rating and restriction
	// It's ok to do this for the primary PSN player here - it has this data locally. All other players need to do it on PSN sign in.

// 	bool bChatRestricted=false;
// 	ProfileManager.GetChatAndContentRestrictions(0,&bChatRestricted,NULL,NULL);

	// 4J-PB - really want to wait until we've read the options, so we can set the right language if they've chosen one other than the default
// 	bool bOptionsRead=false;
//
// 	while((bOptionsRead==false) && ShutdownManager::ShouldRun(ShutdownManager::eMainThread))
// 	{
// 		switch(app.GetOptionsCallbackStatus(0))
// 		{
// 		case C4JStorage::eOptions_Callback_Idle:
// 			break;
// 		default:
// 			bOptionsRead=true;
// 			break;
// 		}
// 		StorageManager.Tick();
// 	}
//
// 	if(ShutdownManager::ShouldRun(ShutdownManager::eMainThread))
// 	{
// 		app.loadStringTable();
// 		ui.SetupFont();
// 		//handleReload();
// 	}

	while( ShutdownManager::ShouldRun(ShutdownManager::eMainThread) )
	{
#if 0
		if(pMinecraft->soundEngine->isStreamingWavebankReady() &&
			!pMinecraft->soundEngine->isPlayingStreamingGameMusic() &&
			!pMinecraft->soundEngine->isPlayingStreamingCDMusic() )
		{
			// play some music in the menus
			pMinecraft->soundEngine->playStreaming(L"", 0, 0, 0, 0, 0, false);
		}
#endif
		app.UpdateTime();
		PIXBeginNamedEvent(0,"Input manager tick");
		InputManager.Tick();
		PIXEndNamedEvent();
		PIXBeginNamedEvent(0,"Profile manager tick");
		ProfileManager.Tick();
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
		// 4J-PB - these get set every tick causing the write profile flag to be true all the time -
// 		app.SetGameSettingsDebugMask(0,eDebugSetting_LoadSavesFromDisk);
// 		app.SetGameSettingsDebugMask(0,eDebugSetting_WriteSavesToDisk);
// 		app.SetLoadSavesFromFolderEnabled(true);
// 		app.SetWriteSavesToFolderEnabled(true);

		LeaderboardManager::Instance()->Tick();
		// Render game graphics.
		if(app.GetGameStarted())
		{

// 			if(InputManager.ButtonPressed(0, MINECRAFT_ACTION_SNEAK_TOGGLE))
// 			{
// 				app.DebugPrintf("saving game...\n");
// 				debugSaveGameDirect();
//
// 			}
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

		// Present the frame.
		PIXBeginNamedEvent(0,"Frame present");
		RenderManager.Present();
		PIXEndNamedEvent();

		RenderManager.Set_matrixDirty();

		ui.CheckMenuDisplayed();
		PIXBeginNamedEvent(0,"Profile load check");
		// has the game defined profile data been changed (by a profile load)
		if(app.uiGameDefinedDataChangedBitmask!=0)
		{
			void *pData;
			for(int i=0;i<XUSER_MAX_COUNT;i++)
			{
				if(app.uiGameDefinedDataChangedBitmask&(1<<i))
				{
					// It has - game needs to update its values with the data from the profile
					pData=StorageManager.GetGameDefinedProfileData(i);
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
			// PS3 TO DO - CSocialManager::Instance()->RefreshPostingCapability();

			// clear the flag
			app.uiGameDefinedDataChangedBitmask=0;

			// Check if any profile write are needed
			//app.CheckGameSettingsChanged();
		}
		PIXEndNamedEvent();

#if 0
		app.TickDLCOffersRetrieved();
		app.TickTMSPPFilesRetrieved();
#endif

		PIXBeginNamedEvent(0,"Network manager do work #2");
		g_NetworkManager.DoWork();
		PIXEndNamedEvent();

#if 0
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

		// PS3 DLC
		app.CommerceTick();

		// Fix for #7318 - Title crashes after short soak in the leaderboards menu
		// A memory leak was caused because the icon renderer kept creating new Vec3's because the pool wasn't reset
		Vec3::resetPool();
	}

	// Free resources, unregister custom classes, and exit.
//	app.Uninit();
// 	g_pd3dDevice->Release();

#if HEAPINSPECTOR_PS3
	HeapInspectorServer::Shutdown();
#endif // HEAPINSPECTOR_PS3

	ShutdownManager::MainThreadHandleShutdown();
}

vector<uint8_t *> vRichPresenceStrings;
uint8_t * AddRichPresenceString(int iID)
{
	uint8_t *strUtf8 = mallocAndCreateUTF8ArrayFromString(iID);
	if( strUtf8 != NULL )
	{
		vRichPresenceStrings.push_back(strUtf8);
	}
	return strUtf8;
}

void FreeRichPresenceStrings()
{
	uint8_t *strUtf8;
	for(int i=0;i<vRichPresenceStrings.size();i++)
	{
		strUtf8=vRichPresenceStrings.at(i);
		free(strUtf8);
	}
	vRichPresenceStrings.clear();
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
	for( auto it = allocCounts.begin(); it != allocCounts.end(); it++ )
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

	for( auto it = allocCounts.begin(); it != allocCounts.end(); it++ )
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

#endif // __PS3__