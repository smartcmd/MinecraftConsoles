// Minecraft.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include <assert.h>
#include <system_service.h>
#include <codecvt>
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
#include "..\..\Minecraft.World\compression.h"
#include "..\..\Minecraft.World\OldChunkStorage.h"
#include "Leaderboards\OrbisLeaderboardManager.h"

#include "Network/Orbis_NPToolkit.h"
#include "Orbis\Network\SonyVoiceChat_Orbis.h"

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

// functions for storing and converting rich presence strings from wchar to utf8
uint8_t * AddRichPresenceString(int iID);
void FreeRichPresenceStrings();

BOOL g_bWidescreen = TRUE;


void DefineActions(void)
{
	// The app needs to define the actions required, and the possible mappings for these

	// Split into Menu actions, and in-game actions
	
	if(InputManager.IsCircleCrossSwapped())
	{
		InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_A,							_PS4_JOY_BUTTON_O);
		InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_OK,							_PS4_JOY_BUTTON_O);
		InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_B,							_PS4_JOY_BUTTON_X);
		InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_CANCEL,						_PS4_JOY_BUTTON_X);
	}
	else
	{
		InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_A,							_PS4_JOY_BUTTON_X);
		InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_OK,							_PS4_JOY_BUTTON_X);
		InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_B,							_PS4_JOY_BUTTON_O);
		InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_CANCEL,						_PS4_JOY_BUTTON_O);
	}
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_X,							_PS4_JOY_BUTTON_SQUARE);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_Y,							_PS4_JOY_BUTTON_TRIANGLE);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_UP,							_PS4_JOY_BUTTON_DPAD_UP		| _PS4_JOY_BUTTON_LSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_DOWN,						_PS4_JOY_BUTTON_DPAD_DOWN	| _PS4_JOY_BUTTON_LSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_LEFT,						_PS4_JOY_BUTTON_DPAD_LEFT	| _PS4_JOY_BUTTON_LSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_RIGHT,						_PS4_JOY_BUTTON_DPAD_RIGHT	| _PS4_JOY_BUTTON_LSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_PAGEUP,						_PS4_JOY_BUTTON_L2);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_PAGEDOWN,					_PS4_JOY_BUTTON_R2);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_RIGHT_SCROLL,				_PS4_JOY_BUTTON_R1);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_LEFT_SCROLL,					_PS4_JOY_BUTTON_L1);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_PAUSEMENU,					_PS4_JOY_BUTTON_OPTIONS);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_STICK_PRESS,					_PS4_JOY_BUTTON_L3);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_OTHER_STICK_PRESS,			_PS4_JOY_BUTTON_R3);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_OTHER_STICK_UP,				_PS4_JOY_BUTTON_RSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_OTHER_STICK_DOWN,			_PS4_JOY_BUTTON_RSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_OTHER_STICK_LEFT,			_PS4_JOY_BUTTON_RSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_OTHER_STICK_RIGHT,			_PS4_JOY_BUTTON_RSTICK_RIGHT);

	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_JUMP,					_PS4_JOY_BUTTON_X);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_FORWARD,				_PS4_JOY_BUTTON_LSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_BACKWARD,				_PS4_JOY_BUTTON_LSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_LEFT,					_PS4_JOY_BUTTON_LSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_RIGHT,					_PS4_JOY_BUTTON_LSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_LOOK_LEFT,				_PS4_JOY_BUTTON_RSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_LOOK_RIGHT,				_PS4_JOY_BUTTON_RSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_LOOK_UP,				_PS4_JOY_BUTTON_RSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_LOOK_DOWN,				_PS4_JOY_BUTTON_RSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_USE,					_PS4_JOY_BUTTON_L2);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_ACTION,					_PS4_JOY_BUTTON_R2);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_RIGHT_SCROLL,			_PS4_JOY_BUTTON_R1);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_LEFT_SCROLL,			_PS4_JOY_BUTTON_L1);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_INVENTORY,				_PS4_JOY_BUTTON_TRIANGLE);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_PAUSEMENU,				_PS4_JOY_BUTTON_OPTIONS);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_DROP,					_PS4_JOY_BUTTON_O);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_SNEAK_TOGGLE,			_PS4_JOY_BUTTON_R3);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_CRAFTING,				_PS4_JOY_BUTTON_SQUARE);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_RENDER_THIRD_PERSON,	_PS4_JOY_BUTTON_L3);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_GAME_INFO,				_PS4_JOY_BUTTON_TOUCHPAD);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_DPAD_LEFT,				_PS4_JOY_BUTTON_DPAD_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_DPAD_RIGHT,				_PS4_JOY_BUTTON_DPAD_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_DPAD_UP,				_PS4_JOY_BUTTON_DPAD_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_DPAD_DOWN,				_PS4_JOY_BUTTON_DPAD_DOWN);		

	if(InputManager.IsCircleCrossSwapped())
	{
		InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_A,							_PS4_JOY_BUTTON_O);
		InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_OK,							_PS4_JOY_BUTTON_O);
		InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_B,							_PS4_JOY_BUTTON_X);
		InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_CANCEL,						_PS4_JOY_BUTTON_X);
	}
	else
	{
		InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_A,							_PS4_JOY_BUTTON_X);
		InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_OK,							_PS4_JOY_BUTTON_X);
		InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_B,							_PS4_JOY_BUTTON_O);
		InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_CANCEL,						_PS4_JOY_BUTTON_O);
	}

	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_X,							_PS4_JOY_BUTTON_SQUARE);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_Y,							_PS4_JOY_BUTTON_TRIANGLE);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_UP,							_PS4_JOY_BUTTON_DPAD_UP		| _PS4_JOY_BUTTON_LSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_DOWN,						_PS4_JOY_BUTTON_DPAD_DOWN	| _PS4_JOY_BUTTON_LSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_LEFT,						_PS4_JOY_BUTTON_DPAD_LEFT	| _PS4_JOY_BUTTON_LSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_RIGHT,						_PS4_JOY_BUTTON_DPAD_RIGHT	| _PS4_JOY_BUTTON_LSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_PAGEUP,						_PS4_JOY_BUTTON_L1);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_PAGEDOWN,					_PS4_JOY_BUTTON_R2);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_RIGHT_SCROLL,				_PS4_JOY_BUTTON_R1);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_LEFT_SCROLL,					_PS4_JOY_BUTTON_L1);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_PAUSEMENU,					_PS4_JOY_BUTTON_TOUCHPAD);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_STICK_PRESS,					_PS4_JOY_BUTTON_L3);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_OTHER_STICK_PRESS,			_PS4_JOY_BUTTON_R3);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_OTHER_STICK_UP,				_PS4_JOY_BUTTON_RSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_OTHER_STICK_DOWN,			_PS4_JOY_BUTTON_RSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_OTHER_STICK_LEFT,			_PS4_JOY_BUTTON_RSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_OTHER_STICK_RIGHT,			_PS4_JOY_BUTTON_RSTICK_RIGHT);

	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_JUMP,					_PS4_JOY_BUTTON_R1);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_FORWARD,				_PS4_JOY_BUTTON_LSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_BACKWARD,				_PS4_JOY_BUTTON_LSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_LEFT,					_PS4_JOY_BUTTON_LSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_RIGHT,					_PS4_JOY_BUTTON_LSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_LOOK_LEFT,				_PS4_JOY_BUTTON_RSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_LOOK_RIGHT,				_PS4_JOY_BUTTON_RSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_LOOK_UP,				_PS4_JOY_BUTTON_RSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_LOOK_DOWN,				_PS4_JOY_BUTTON_RSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_USE,					_PS4_JOY_BUTTON_R2);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_ACTION,					_PS4_JOY_BUTTON_L2);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_RIGHT_SCROLL,			_PS4_JOY_BUTTON_DPAD_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_LEFT_SCROLL,			_PS4_JOY_BUTTON_DPAD_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_INVENTORY,				_PS4_JOY_BUTTON_TRIANGLE);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_PAUSEMENU,				_PS4_JOY_BUTTON_OPTIONS);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_DROP,					_PS4_JOY_BUTTON_O);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_SNEAK_TOGGLE,			_PS4_JOY_BUTTON_L3);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_CRAFTING,				_PS4_JOY_BUTTON_SQUARE);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_RENDER_THIRD_PERSON,	_PS4_JOY_BUTTON_R3);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_GAME_INFO,				_PS4_JOY_BUTTON_TOUCHPAD);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_DPAD_LEFT,				_PS4_JOY_BUTTON_DPAD_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_DPAD_RIGHT,				_PS4_JOY_BUTTON_DPAD_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_DPAD_UP,				_PS4_JOY_BUTTON_DPAD_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_DPAD_DOWN,				_PS4_JOY_BUTTON_DPAD_DOWN);	

	if(InputManager.IsCircleCrossSwapped())
	{
		InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_A,							_PS4_JOY_BUTTON_O);
		InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OK,							_PS4_JOY_BUTTON_O);
		InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_B,							_PS4_JOY_BUTTON_X);
		InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_CANCEL,						_PS4_JOY_BUTTON_X);
	}
	else
	{
		InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_A,							_PS4_JOY_BUTTON_X);
		InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OK,							_PS4_JOY_BUTTON_X);
		InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_B,							_PS4_JOY_BUTTON_O);
		InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_CANCEL,						_PS4_JOY_BUTTON_O);
	}
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_X,							_PS4_JOY_BUTTON_SQUARE);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_Y,							_PS4_JOY_BUTTON_TRIANGLE);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_UP,							_PS4_JOY_BUTTON_DPAD_UP		| _PS4_JOY_BUTTON_LSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_DOWN,						_PS4_JOY_BUTTON_DPAD_DOWN	| _PS4_JOY_BUTTON_LSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_LEFT,						_PS4_JOY_BUTTON_DPAD_LEFT	| _PS4_JOY_BUTTON_LSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_RIGHT,						_PS4_JOY_BUTTON_DPAD_RIGHT	| _PS4_JOY_BUTTON_LSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_PAGEUP,						_PS4_JOY_BUTTON_DPAD_UP		| _PS4_JOY_BUTTON_L1);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_PAGEDOWN,					_PS4_JOY_BUTTON_R2);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_RIGHT_SCROLL,				_PS4_JOY_BUTTON_R1);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_LEFT_SCROLL,					_PS4_JOY_BUTTON_L1);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_PAUSEMENU,					_PS4_JOY_BUTTON_OPTIONS);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_STICK_PRESS,					_PS4_JOY_BUTTON_L3);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OTHER_STICK_PRESS,			_PS4_JOY_BUTTON_R3);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OTHER_STICK_UP,				_PS4_JOY_BUTTON_RSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OTHER_STICK_DOWN,			_PS4_JOY_BUTTON_RSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OTHER_STICK_LEFT,			_PS4_JOY_BUTTON_RSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OTHER_STICK_RIGHT,			_PS4_JOY_BUTTON_RSTICK_RIGHT);

	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_JUMP,					_PS4_JOY_BUTTON_L2);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_FORWARD,				_PS4_JOY_BUTTON_LSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_BACKWARD,				_PS4_JOY_BUTTON_LSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_LEFT,					_PS4_JOY_BUTTON_LSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_RIGHT,					_PS4_JOY_BUTTON_LSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_LOOK_LEFT,				_PS4_JOY_BUTTON_RSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_LOOK_RIGHT,				_PS4_JOY_BUTTON_RSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_LOOK_UP,				_PS4_JOY_BUTTON_RSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_LOOK_DOWN,				_PS4_JOY_BUTTON_RSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_USE,					_PS4_JOY_BUTTON_R2);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_ACTION,					_PS4_JOY_BUTTON_X);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_RIGHT_SCROLL,			_PS4_JOY_BUTTON_DPAD_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_LEFT_SCROLL,			_PS4_JOY_BUTTON_DPAD_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_INVENTORY,				_PS4_JOY_BUTTON_TRIANGLE);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_PAUSEMENU,				_PS4_JOY_BUTTON_OPTIONS);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_DROP,					_PS4_JOY_BUTTON_O);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_SNEAK_TOGGLE,			_PS4_JOY_BUTTON_L1);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_CRAFTING,				_PS4_JOY_BUTTON_SQUARE);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_RENDER_THIRD_PERSON,	_PS4_JOY_BUTTON_L3);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_GAME_INFO,				_PS4_JOY_BUTTON_TOUCHPAD);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_DPAD_LEFT,				_PS4_JOY_BUTTON_DPAD_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_DPAD_RIGHT,				_PS4_JOY_BUTTON_DPAD_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_DPAD_UP,				_PS4_JOY_BUTTON_DPAD_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_DPAD_DOWN,				_PS4_JOY_BUTTON_DPAD_DOWN);	

	// Vita remote play map
	if(InputManager.IsCircleCrossSwapped())
	{
		InputManager.SetGameJoypadMaps(MAP_STYLE_3,ACTION_MENU_A,							_PS4_JOY_BUTTON_O);
		InputManager.SetGameJoypadMaps(MAP_STYLE_3,ACTION_MENU_OK,							_PS4_JOY_BUTTON_O);
		InputManager.SetGameJoypadMaps(MAP_STYLE_3,ACTION_MENU_B,							_PS4_JOY_BUTTON_X);
		InputManager.SetGameJoypadMaps(MAP_STYLE_3,ACTION_MENU_CANCEL,						_PS4_JOY_BUTTON_X);
	}
	else
	{
		InputManager.SetGameJoypadMaps(MAP_STYLE_3,ACTION_MENU_A,							_PS4_JOY_BUTTON_X);
		InputManager.SetGameJoypadMaps(MAP_STYLE_3,ACTION_MENU_OK,							_PS4_JOY_BUTTON_X);
		InputManager.SetGameJoypadMaps(MAP_STYLE_3,ACTION_MENU_B,							_PS4_JOY_BUTTON_O);
		InputManager.SetGameJoypadMaps(MAP_STYLE_3,ACTION_MENU_CANCEL,						_PS4_JOY_BUTTON_O);
	}
	InputManager.SetGameJoypadMaps(MAP_STYLE_3,ACTION_MENU_X,							_PS4_JOY_BUTTON_SQUARE);
	InputManager.SetGameJoypadMaps(MAP_STYLE_3,ACTION_MENU_Y,							_PS4_JOY_BUTTON_TRIANGLE);
	InputManager.SetGameJoypadMaps(MAP_STYLE_3,ACTION_MENU_UP,							_PS4_JOY_BUTTON_DPAD_UP		| _PS4_JOY_BUTTON_LSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_3,ACTION_MENU_DOWN,						_PS4_JOY_BUTTON_DPAD_DOWN	| _PS4_JOY_BUTTON_LSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_3,ACTION_MENU_LEFT,						_PS4_JOY_BUTTON_DPAD_LEFT	| _PS4_JOY_BUTTON_LSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_3,ACTION_MENU_RIGHT,						_PS4_JOY_BUTTON_DPAD_RIGHT	| _PS4_JOY_BUTTON_LSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_3,ACTION_MENU_PAUSEMENU,					_PS4_JOY_BUTTON_OPTIONS);	
	InputManager.SetGameJoypadMaps(MAP_STYLE_3,ACTION_MENU_OTHER_STICK_UP,				_PS4_JOY_BUTTON_RSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_3,ACTION_MENU_OTHER_STICK_DOWN,			_PS4_JOY_BUTTON_RSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_3,ACTION_MENU_OTHER_STICK_LEFT,			_PS4_JOY_BUTTON_RSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_3,ACTION_MENU_OTHER_STICK_RIGHT,			_PS4_JOY_BUTTON_RSTICK_RIGHT);

	InputManager.SetGameJoypadMaps(MAP_STYLE_3,ACTION_MENU_PAGEUP,						_PS4_JOY_BUTTON_L2);
	InputManager.SetGameJoypadMaps(MAP_STYLE_3,ACTION_MENU_PAGEDOWN,					_PS4_JOY_BUTTON_R2);
	InputManager.SetGameJoypadMaps(MAP_STYLE_3,ACTION_MENU_RIGHT_SCROLL,				_PS4_JOY_BUTTON_R1);
	InputManager.SetGameJoypadMaps(MAP_STYLE_3,ACTION_MENU_LEFT_SCROLL,					_PS4_JOY_BUTTON_L1);
	InputManager.SetGameJoypadMaps(MAP_STYLE_3,ACTION_MENU_STICK_PRESS,					_PS4_JOY_BUTTON_L3);
	InputManager.SetGameJoypadMaps(MAP_STYLE_3,ACTION_MENU_OTHER_STICK_PRESS,			_PS4_JOY_BUTTON_R3);

	InputManager.SetGameJoypadMaps(MAP_STYLE_3,MINECRAFT_ACTION_JUMP,					_PS4_JOY_BUTTON_X);
	InputManager.SetGameJoypadMaps(MAP_STYLE_3,MINECRAFT_ACTION_FORWARD,				_PS4_JOY_BUTTON_LSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_3,MINECRAFT_ACTION_BACKWARD,				_PS4_JOY_BUTTON_LSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_3,MINECRAFT_ACTION_LEFT,					_PS4_JOY_BUTTON_LSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_3,MINECRAFT_ACTION_RIGHT,					_PS4_JOY_BUTTON_LSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_3,MINECRAFT_ACTION_LOOK_LEFT,				_PS4_JOY_BUTTON_RSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_3,MINECRAFT_ACTION_LOOK_RIGHT,				_PS4_JOY_BUTTON_RSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_3,MINECRAFT_ACTION_LOOK_UP,				_PS4_JOY_BUTTON_RSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_3,MINECRAFT_ACTION_LOOK_DOWN,				_PS4_JOY_BUTTON_RSTICK_DOWN);
	
	InputManager.SetGameJoypadMaps(MAP_STYLE_3,MINECRAFT_ACTION_INVENTORY,				_PS4_JOY_BUTTON_TRIANGLE);
	InputManager.SetGameJoypadMaps(MAP_STYLE_3,MINECRAFT_ACTION_PAUSEMENU,				_PS4_JOY_BUTTON_OPTIONS);
	InputManager.SetGameJoypadMaps(MAP_STYLE_3,MINECRAFT_ACTION_DROP,					_PS4_JOY_BUTTON_O);	
	InputManager.SetGameJoypadMaps(MAP_STYLE_3,MINECRAFT_ACTION_CRAFTING,				_PS4_JOY_BUTTON_SQUARE);	
	InputManager.SetGameJoypadMaps(MAP_STYLE_3,MINECRAFT_ACTION_GAME_INFO,				_PS4_JOY_BUTTON_TOUCHPAD);

	InputManager.SetGameJoypadMaps(MAP_STYLE_3,MINECRAFT_ACTION_USE,					_PS4_JOY_BUTTON_L1);
	InputManager.SetGameJoypadMaps(MAP_STYLE_3,MINECRAFT_ACTION_ACTION,					_PS4_JOY_BUTTON_R1);
	InputManager.SetGameJoypadMaps(MAP_STYLE_3,MINECRAFT_ACTION_RIGHT_SCROLL,			_PS4_JOY_BUTTON_R2 | _PS4_JOY_BUTTON_DPAD_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_3,MINECRAFT_ACTION_LEFT_SCROLL,			_PS4_JOY_BUTTON_L2 | _PS4_JOY_BUTTON_DPAD_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_3,MINECRAFT_ACTION_RENDER_THIRD_PERSON,	_PS4_JOY_BUTTON_L3 | _PS4_JOY_BUTTON_DPAD_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_3,MINECRAFT_ACTION_SNEAK_TOGGLE,			_PS4_JOY_BUTTON_R3 | _PS4_JOY_BUTTON_DPAD_DOWN);

	// Touchpad mapping
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_TOUCHPAD_PRESS,				_PS4_JOY_BUTTON_TOUCHPAD);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_TOUCHPAD_PRESS,				_PS4_JOY_BUTTON_TOUCHPAD);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_TOUCHPAD_PRESS,				_PS4_JOY_BUTTON_TOUCHPAD);
	InputManager.SetGameJoypadMaps(MAP_STYLE_3,ACTION_MENU_TOUCHPAD_PRESS,				_PS4_JOY_BUTTON_TOUCHPAD);

	// Partial mapping for Vita TV (applied over other mappings)

	InputManager.SetGameJoypadMaps(PARTIAL_MAP_1, MINECRAFT_ACTION_GAME_INFO,			_PS4_JOY_BUTTON_DPAD_UP);
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
	pd3dPP->PresentationInterval   = D3DPRESENT_INTERVAL_ONE;
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

#ifndef __ORBIS__
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
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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

   g_hWnd = CreateWindow("MinecraftClass", "Minecraft", WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!g_hWnd)
   {
      return FALSE;
   }

   ShowWindow(g_hWnd, nCmdShow);
   UpdateWindow(g_hWnd);

   return TRUE;
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
#endif


void RegisterAwardsWithProfileManager()
{
	// register the awards
	ProfileManager.RegisterAward(eAward_TakingInventory,		ACHIEVEMENT_01, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_GettingWood,			ACHIEVEMENT_02, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_Benchmarking,			ACHIEVEMENT_03, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_TimeToMine,				ACHIEVEMENT_04, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_HotTopic,				ACHIEVEMENT_05, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_AquireHardware,			ACHIEVEMENT_06, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_TimeToFarm,				ACHIEVEMENT_07, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_BakeBread,				ACHIEVEMENT_08, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_TheLie,					ACHIEVEMENT_09, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_GettingAnUpgrade,		ACHIEVEMENT_10, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_DeliciousFish,			ACHIEVEMENT_11, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_OnARail,				ACHIEVEMENT_12, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_TimeToStrike,			ACHIEVEMENT_13, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_MonsterHunter,			ACHIEVEMENT_14, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_CowTipper,				ACHIEVEMENT_15, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_WhenPigsFly,			ACHIEVEMENT_16, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_LeaderOfThePack,		ACHIEVEMENT_17, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_MOARTools,				ACHIEVEMENT_18, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_DispenseWithThis,		ACHIEVEMENT_19, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_InToTheNether,			ACHIEVEMENT_20, eAwardType_Achievement);

	ProfileManager.RegisterAward(eAward_snipeSkeleton,			ACHIEVEMENT_21, eAwardType_Achievement); // 'Sniper Duel'
	ProfileManager.RegisterAward(eAward_diamonds,				ACHIEVEMENT_22, eAwardType_Achievement); // 'DIAMONDS!'
	ProfileManager.RegisterAward(eAward_ghast,					ACHIEVEMENT_23, eAwardType_Achievement); // 'Return To Sender'
	ProfileManager.RegisterAward(eAward_blazeRod,				ACHIEVEMENT_24, eAwardType_Achievement); // 'Into Fire'
	ProfileManager.RegisterAward(eAward_potion,					ACHIEVEMENT_25, eAwardType_Achievement); // 'Local Brewery'
	ProfileManager.RegisterAward(eAward_theEnd,					ACHIEVEMENT_26, eAwardType_Achievement); // 'The End?'
	ProfileManager.RegisterAward(eAward_winGame,				ACHIEVEMENT_27, eAwardType_Achievement); // 'The End.'
	ProfileManager.RegisterAward(eAward_enchantments,			ACHIEVEMENT_28, eAwardType_Achievement); // 'Enchanter'

#ifdef _EXTENDED_ACHIEVEMENTS
	ProfileManager.RegisterAward(eAward_overkill,				ACHIEVEMENT_29, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_bookcase,				ACHIEVEMENT_30, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_adventuringTime,		ACHIEVEMENT_31, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_repopulation,			ACHIEVEMENT_32, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_diamondsToYou,			ACHIEVEMENT_33, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_eatPorkChop,			ACHIEVEMENT_34, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_play100Days,			ACHIEVEMENT_35, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_arrowKillCreeper,		ACHIEVEMENT_36, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_theHaggler,				ACHIEVEMENT_37, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_potPlanter,				ACHIEVEMENT_38, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_itsASign,				ACHIEVEMENT_39, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_ironBelly,				ACHIEVEMENT_40, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_haveAShearfulDay,		ACHIEVEMENT_41, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_rainbowCollection,		ACHIEVEMENT_42, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_stayinFrosty,			ACHIEVEMENT_43, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_chestfulOfCobblestone,	ACHIEVEMENT_44, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_renewableEnergy,		ACHIEVEMENT_45, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_musicToMyEars,			ACHIEVEMENT_46, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_bodyGuard,				ACHIEVEMENT_47, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_ironMan,				ACHIEVEMENT_48, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_zombieDoctor,			ACHIEVEMENT_49, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_lionTamer,				ACHIEVEMENT_50, eAwardType_Achievement);
#endif

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
	ProfileManager.RichPresenceInit(4,1);
	ProfileManager.SetRichPresenceSettingFn(SQRNetworkManager_Orbis::SetRichPresence);
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

int StartMinecraftThreadProc( void* lpParameter )
{
	Vec3::UseDefaultThreadStorage();
	AABB::UseDefaultThreadStorage();
	Tesselator::CreateNewThreadStorage(1024*1024);
	RenderManager.InitialiseContext();
	Minecraft::start(wstring(),wstring());
	delete Tesselator::getInstance();
	return 0;
}

int main(int argc, const char *argv[] )
{
	app.DebugPrintf("---main()\n");
	OrbisInit();


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





	char *usrdirPath= getUsrDirPath();

	strcpy(usrdirPath,"/app0");

	RenderManager.Initialise();

	// Read the file containing the product codes
	app.DebugPrintf("---ReadProductCodes()\n");
	if(app.ReadProductCodes()==FALSE)
	{
		// can't continue
		app.FatalLoadError();
	}

// 	StorageManager.SetDLCProductCode(app.GetProductCode());
// 	StorageManager.SetProductUpgradeKey(app.GetUpgradeKey());
	//	ProfileManager.SetServiceID(app.GetCommerceCategory());

	SceNpContentRestriction param;
	memset(&param, 0x0, sizeof(param));
	param.size = sizeof(param);

	SceNpAgeRestriction ageRestriction[5];
	memset(ageRestriction, 0x0, sizeof(ageRestriction));

	bool bCircleCrossSwapped = false;
	switch(app.GetProductSKU())
	{
	case e_sku_SCEA:
		// ESRB EVERYONE 10+
		param.defaultAgeRestriction = 10;
		break;
	case e_sku_SCEJ:
		bCircleCrossSwapped = true;
		// 4J Stu - Intentional fall-through
	case e_sku_SCEE:
		// PEGI 7+
		param.defaultAgeRestriction = 7;



		// USK 6+
		strncpy(ageRestriction[0].countryCode.data, "de" , 2);
		ageRestriction[0].age = 6;

		// PG rating has no age, but for some reason the testers are saying it's 8
		strncpy(ageRestriction[1].countryCode.data, "au" , 2);
		ageRestriction[1].age = 8;

		strncpy(ageRestriction[2].countryCode.data, "ru" , 2);
		ageRestriction[2].age = 6;

		strncpy(ageRestriction[3].countryCode.data, "jp" , 2);
		ageRestriction[3].age = 0;

		strncpy(ageRestriction[4].countryCode.data, "kr" , 2);
		ageRestriction[4].age = 0;

		param.ageRestrictionCount = 5;
		param.ageRestriction = ageRestriction;
		break;
	}

	InputManager.SetCircleCrossSwapped(bCircleCrossSwapped);

	int err = sceNpSetContentRestriction(&param);
	if (err < 0){
		/* Error handling */
		app.DebugPrintf("sceNpSetContentRestriction failed with error %08x\n", err);
	}

	err = sceGameLiveStreamingInitialize(SCE_GAME_LIVE_STREAMING_HEAP_SIZE);

	if(err < 0)
	{
		app.DebugPrintf("Failed to init sceGameLiveStreamingInitialize. %08x\n", err);
	}

	err = sceGameLiveStreamingSetStandbyScreenResource("standy_screen");

	if(err < 0)
	{
		app.DebugPrintf("Failed to set special standby screen resource. %08x\n", err);
	}


	app.loadMediaArchive();
	app.loadStringTable();
	ui.init(1920,1080);

	// storage manager is needed for the trial key check
	StorageManager.Init(0,app.GetString(IDS_DEFAULT_SAVENAME),"savegame.dat",FIFTY_ONE_MB,&CConsoleMinecraftApp::DisplaySavingMessage,(LPVOID)&app,"");
	StorageManager.SetSaveTitleExtraFileSuffix(app.GetString(IDS_SAVE_SUBTITLE_SUFFIX));
	StorageManager.SetDLCInfoMap(app.GetSonyDLCMap());
	app.CommerceInit(); //  MGH - moved this here so GetCommerce isn't NULL
	// 4J-PB - Kick of the check for trial or full version - requires ui to be initialised
	app.GetCommerce()->CheckForTrialUpgradeKey();

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
	NPToolkit.init();

	// Set the number of possible joypad layouts that the user can switch between, and the number of actions
	InputManager.Initialise(1,5,MINECRAFT_ACTION_MAX, ACTION_MAX_MENU);

	// Set the default joypad action mappings for Minecraft
	DefineActions();
	InputManager.SetJoypadMapVal(0,0);
	InputManager.SetKeyRepeatRate(0.3f,0.2f);

	// looks like the PS4 controller is a good bit more sensitive than PS3. Defaults in the lib are 10000 for deadzone and 32767 for movement range
	InputManager.SetDeadzoneAndMovementRange(10000,20000,32767);

	// Initialise the profile manager with the game Title ID, Offer ID, a profile version number, and the number of profile values and settings

	SceNpCommunicationId commsId;
	SceNpCommunicationSignature commsSig;

	ProfileManager.Initialise(	&commsId, //SQRNetworkManager::GetSceNpCommsId(),
								&commsSig, //SQRNetworkManager::GetSceNpCommsSig(),
								PROFILE_VERSION_BUILD_JUNE14,
								NUM_PROFILE_VALUES,
								NUM_PROFILE_SETTINGS,
								dwProfileSettingsA,
								app.GAME_DEFINED_PROFILE_DATA_BYTES*XUSER_MAX_COUNT,
								&app.uiGameDefinedDataChangedBitmask);

	// register the awards
	RegisterAwardsWithProfileManager();

	// register the get string function with the profile lib, so it can be called within the lib

	ProfileManager.SetGetStringFunc(&CConsoleMinecraftApp::GetString);
	ProfileManager.SetPlayerListTitleID(IDS_PLAYER_LIST_TITLE);




	StorageManager.SetGameSaveFolderTitle((WCHAR *)app.GetString(IDS_GAMENAME));
	StorageManager.SetSaveCacheFolderTitle((WCHAR *)app.GetString(IDS_SAVECACHEFILE));
	StorageManager.SetOptionsFolderTitle((WCHAR *)app.GetString(IDS_OPTIONSFILE));
	StorageManager.SetCorruptSaveName((WCHAR *)app.GetString(IDS_CORRUPTSAVE_TITLE));
#if (defined _FINAL_BUILD) || (defined _ART_BUILD)
	StorageManager.SetGameSaveFolderPrefix(app.GetSaveFolderPrefix());		
#else
	// Use debug directory to prevent debug saves being loaded/edited in package build (since debug can't edit package save games this causes various problems) 
	StorageManager.SetGameSaveFolderPrefix("DEBUG01899");
#endif
	StorageManager.SetMaxSaves(99);

	byteArray baOptionsIcon = app.getArchiveFile(L"DefaultOptionsImage228x128.png");
	byteArray baSaveThumbnail = app.getArchiveFile(L"DefaultSaveThumbnail64x64.png");
	byteArray baSaveImage = app.getArchiveFile(L"DefaultSaveImage228x128.png");

	StorageManager.InitialiseProfileData(PROFILE_VERSION_BUILD_JUNE14,
		NUM_PROFILE_VALUES,
		NUM_PROFILE_SETTINGS,
		dwProfileSettingsA,
		app.GAME_DEFINED_PROFILE_DATA_BYTES*XUSER_MAX_COUNT,
		&app.uiGameDefinedDataChangedBitmask);

	StorageManager.SetDefaultImages((PBYTE)baOptionsIcon.data, baOptionsIcon.length,(PBYTE)baSaveImage.data, baSaveImage.length,(PBYTE)baSaveThumbnail.data, baSaveThumbnail.length);

	// Set function to be called if a save game operation can't complete due to running out of storage space etc.
	StorageManager.SetIncompleteSaveCallback(CConsoleMinecraftApp::Callback_SaveGameIncomplete, (LPVOID)&app);

	// Temporary - set a single user for quadrant 0 for the storage manager. Will need to do more to link this aspect of the storage manager is once we have proper profile management.

	SceUserServiceUserId aUserIds[4];
	memset(aUserIds, 0, sizeof(aUserIds));
	int ret = sceUserServiceGetInitialUser(&aUserIds[0]);
	assert(ret == SCE_OK);
	StorageManager.SetQuadrantUserIds(aUserIds);

#if 0
	// Set up the global title storage path
	StorageManager.StoreTMSPathName();


	// set a function to be called when the ethernet is disconnected, so we can back out if required
	ProfileManager.SetNotificationsCallback(&CConsoleMinecraftApp::NotificationsCallback,(LPVOID)&app);
	
#endif
	// set a function to be called when there's a sign in change, so we can exit a level if the primary player signs out
	ProfileManager.SetSignInChangeCallback(&CConsoleMinecraftApp::SignInChangeCallback,(LPVOID)&app);


	// Set a callback for the default player options to be set - when there is no profile data for the player
	StorageManager.SetDefaultOptionsCallback(&CConsoleMinecraftApp::DefaultOptionsCallback,(LPVOID)&app);
	StorageManager.SetOptionsDataCallback(&CConsoleMinecraftApp::OptionsDataCallback,(LPVOID)&app);

	// Set a callback to deal with old profile versions needing updated to new versions
	StorageManager.SetOldProfileVersionCallback(&CConsoleMinecraftApp::OldProfileVersionCallback,(LPVOID)&app);

	// Set a callback for when there is a read error on profile data
	//StorageManager.SetProfileReadErrorCallback(&CConsoleMinecraftApp::ProfileReadErrorCallback,(LPVOID)&app);


	// QNet needs to be setup after profile manager, as we do not want its Notify listener to handle
	// XN_SYS_SIGNINCHANGED notifications. This does mean that we need to have a callback in the
	// ProfileManager for XN_LIVE_INVITE_ACCEPTED for QNet.

	g_NetworkManager.Initialise();

	// debug switch to trial version
 	ProfileManager.SetDebugFullOverride(true);
#if 0

	//ProfileManager.AddDLC(2);
	StorageManager.SetDLCPackageRoot("DLCDrive");
	StorageManager.RegisterMarketplaceCountsCallback(&CConsoleMinecraftApp::MarketplaceCountsCallback,(LPVOID)&app);
	// Kinect !

	if(XNuiGetHardwareStatus()!=0)
	{
		// If the Kinect Sensor is not physically connected, this function returns 0. 
		NuiInitialize(NUI_INITIALIZE_FLAG_USES_HIGH_QUALITY_COLOR | NUI_INITIALIZE_FLAG_USES_DEPTH | 
			NUI_INITIALIZE_FLAG_EXTRAPOLATE_FLOOR_PLANE | NUI_INITIALIZE_FLAG_USES_FITNESS | NUI_INITIALIZE_FLAG_NUI_GUIDE_DISABLED | NUI_INITIALIZE_FLAG_SUPPRESS_AUTOMATIC_UI,NUI_INITIALIZE_DEFAULT_HARDWARE_THREAD );
	}

	// Sentient !
	hr = TelemetryManager->Init();  


#endif
	// Initialise TLS for tesselator, for this main thread
	Tesselator::CreateNewThreadStorage(1024*1024);
	// Initialise TLS for AABB and Vec3 pools, for this main thread
	AABB::CreateNewThreadStorage();
	Vec3::CreateNewThreadStorage();
	IntCache::CreateNewThreadStorage();
	Compression::CreateNewThreadStorage();	
	OldChunkStorage::CreateNewThreadStorage();
	Level::enableLightingCache();
	Tile::CreateNewThreadStorage();

	Minecraft::main();

	// Minecraft::main () used to call Minecraft::Start, but this takes ~2.5 seconds, so now running this in another thread
	// so we can do some basic renderer calls whilst it is happening. This is at attempt to stop getting TRC failure on SubmitDone taking > 5 seconds on boot
	C4JThread *minecraftThread = new C4JThread(&StartMinecraftThreadProc, NULL, "Running minecraft start");
	minecraftThread->Run();	
	do
	{
		RenderManager.StartFrame();
		Sleep(20);
		RenderManager.Present();
	} while (minecraftThread->isRunning());
	delete minecraftThread;

	Minecraft *pMinecraft=Minecraft::GetInstance();

	app.InitGameSettings();
	// read the options here for controller 0
	StorageManager.ReadFromProfile(0);

	// 4J-PB - have to hide the auto splashscreen, or we never see our game
	sceSystemServiceHideSplashScreen();


#if 0
	//bool bDisplayPauseMenu=false;

	// set the default gamma level
	float fVal=50.0f*327.68f;
	RenderManager.UpdateGamma((unsigned short)fVal);

	// load any skins
	//app.AddSkinsToMemoryTextureFiles();
#endif
	// set the default profile values
	for(int i=0;i<XUSER_MAX_COUNT;i++)
	{	
		app.SetDefaultOptions(StorageManager.GetDashboardProfileSettings(i),i);
	}
#if 0

	// set the achievement text for a trial achievement, now we have the string table loaded
	ProfileManager.SetTrialTextStringTable(app.GetStringTable(),IDS_CONFIRM_OK, IDS_CONFIRM_CANCEL);
	ProfileManager.SetTrialAwardText(eAwardType_Achievement,IDS_UNLOCK_TITLE,IDS_UNLOCK_ACHIEVEMENT_TEXT);
	ProfileManager.SetTrialAwardText(eAwardType_GamerPic,IDS_UNLOCK_TITLE,IDS_UNLOCK_GAMERPIC_TEXT);
	ProfileManager.SetTrialAwardText(eAwardType_AvatarItem,IDS_UNLOCK_TITLE,IDS_UNLOCK_AVATAR_TEXT);
	ProfileManager.SetTrialAwardText(eAwardType_Theme,IDS_UNLOCK_TITLE,IDS_UNLOCK_THEME_TEXT);
	ProfileManager.SetUpsellCallback(&app.UpsellReturnedCallback,&app);

	// Set up a debug character press sequence
#ifndef _FINAL_BUILD
	app.SetDebugSequence("LRLRYYY");
#endif

	// Initialise the social networking manager.
	CSocialManager::Instance()->Initialise();

	// Update the base scene quick selects now that the minecraft class exists
	//CXuiSceneBase::UpdateScreenSettings(0);
#endif
	app.InitialiseTips();
#if 0

	DWORD initData=0;



#ifndef _FINAL_BUILD
#ifndef _DEBUG
	#pragma message(__LOC__"Need to define the _FINAL_BUILD before submission")
#endif
#endif

	// Set the default sound levels
	pMinecraft->options->set(Options::Option::MUSIC,1.0f);
	pMinecraft->options->set(Options::Option::SOUND,1.0f);

	app.NavigateToScene(XUSER_INDEX_ANY,eUIScene_Intro,&initData);
#endif
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

	// 4J-PB - Moved here because it crashed when the network init was called if left where it was before.
	// We should track down why though...
	app.DebugPrintf("---init sound engine()\n");

	pMinecraft->soundEngine->init(NULL);

	while (TRUE)
	{
		SonyVoiceChat_Orbis::tick();
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

		LeaderboardManager::Instance()->Tick();
		// Render game graphics.
		if(app.GetGameStarted()) 
		{
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

		static bool bInitnet=false;

		if(bInitnet)
		{
			g_NetworkManager.Initialise();
		}

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

			// clear the flag
			app.uiGameDefinedDataChangedBitmask=0;
		}
		PIXEndNamedEvent();

		// 4J-PB - Monitor the options save for a space issue
		bool bOptionsNoSpace=false;
		for(int i=0;i<XUSER_MAX_COUNT;i++)
		{
			if(bOptionsNoSpace==false)
			{			
				if(app.GetOptionsCallbackStatus(i)==C4JStorage::eOptions_Callback_Write_Fail_NoSpace)
				{
					// get the game to bring up the save space handling
					bOptionsNoSpace=true;
					app.SetOptionsCallbackStatus(i,C4JStorage::eOptions_Callback_Idle);
					app.SetAction(i,eAppAction_OptionsSaveNoSpace);
				}
			}
		}


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

		// PS4 DLC
		app.CommerceTick();
		app.SystemServiceTick();

		app.SaveDataDialogTick();
		app.PatchAvailableDialogTick();

		// Fix for #7318 - Title crashes after short soak in the leaderboards menu
		// A memory leak was caused because the icon renderer kept creating new Vec3's because the pool wasn't reset
		Vec3::resetPool();
	}

	// Free resources, unregister custom classes, and exit.
//	app.Uninit();
//	g_pd3dDevice->Release();
}


vector<uint8_t *> vRichPresenceStrings;

// convert wstring to UTF-8 string
std::string wstring_to_utf8 (const std::wstring& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	return myconv.to_bytes(str);
}

uint8_t *mallocAndCreateUTF8ArrayFromString(int iID)
{
	int result;
	LPCWSTR wchString=app.GetString(iID);

	std::wstring srcString = wchString;
	std::string dstString = wstring_to_utf8(srcString);

	int dst_len = dstString.size()+1;
	uint8_t *strUtf8=(uint8_t *)malloc(dst_len);
	memcpy(strUtf8, dstString.c_str(), dst_len);

	return strUtf8;
}

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
