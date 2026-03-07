// Minecraft.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include "Leaderboards\PSVitaLeaderboardManager.h"
#include "PSVita\PSVitaExtras\ShutdownManager.h"

//#define HEAPINSPECTOR_PS3	1
// when defining HEAPINSPECTOR_PS3, add this line to the linker settings
// --wrap malloc --wrap free --wrap memalign --wrap calloc --wrap realloc --wrap reallocalign  --wrap _malloc_init 

#if HEAPINSPECTOR_PS3
#include "HeapInspector\Server\HeapInspectorServer.h"
#include "HeapInspector\Server\PS3\HeapHooks.hpp"
#endif

//#define DISABLE_MILES_SOUND

#include "PSVita_App.h"
#include "PSVitaExtras\PSVitaStrings.h"
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
#include "..\GameRenderer.h"
#include "Sentient\SentientManager.h"
#include "..\..\Minecraft.World\IntCache.h"
#include "..\Textures.h"
//#include "Resource.h"
#include "..\..\Minecraft.World\compression.h"
#include "..\..\Minecraft.World\OldChunkStorage.h"
//#include "PS3\PS3Extras\EdgeZLib.h"
#include "..\..\Minecraft.World\C4JThread.h"
#include "Common\Network\Sony\SQRNetworkManager.h"
#include "Common\UI\IUIScene_PauseMenu.h"
#include "Conf.h"
#include "PSVita/Network/PSVita_NPToolkit.h"
#include "PSVita\Network\SonyVoiceChat_Vita.h"
#include "..\..\Minecraft.World\FireworksRecipe.h"

#include <libsysmodule.h>
#include <libperf.h>
#include <ult.h>
#include <apputil.h>

#define THEME_NAME		"584111F70AAAAAAA"
#define THEME_FILESIZE	2797568

/* Encrypted ID for protected data file (*) You must edit these binaries!! */
/*char secureFileId[CELL_SAVEDATA_SECUREFILEID_SIZE] = 
{
0xEE, 0xA9, 0x37, 0xCC,
0x5B, 0xD4, 0xD9, 0x0D,
0x55, 0xED, 0x25, 0x31,
0xFA, 0x33, 0xBD, 0xC4
};*/


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

	/////////////////////////////////////
	// VITA
	/////////////////////////////////////

	// Split into Menu actions, and in-game actions
	if(InputManager.IsCircleCrossSwapped())
	{
		InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_A,							_PSV_JOY_BUTTON_O);
		InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_OK,							_PSV_JOY_BUTTON_O);
		InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_B,							_PSV_JOY_BUTTON_X);
		InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_CANCEL,						_PSV_JOY_BUTTON_X);
	}
	else
	{
		InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_A,							_PSV_JOY_BUTTON_X);
		InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_OK,							_PSV_JOY_BUTTON_X);
		InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_B,							_PSV_JOY_BUTTON_O);
		InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_CANCEL,						_PSV_JOY_BUTTON_O);
	}

	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_X,							_PSV_JOY_BUTTON_SQUARE);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_Y,							_PSV_JOY_BUTTON_TRIANGLE);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_UP,							_PSV_JOY_BUTTON_DPAD_UP | _360_JOY_BUTTON_LSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_DOWN,						_PSV_JOY_BUTTON_DPAD_DOWN | _360_JOY_BUTTON_LSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_LEFT,						_PSV_JOY_BUTTON_DPAD_LEFT | _360_JOY_BUTTON_LSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_RIGHT,						_PSV_JOY_BUTTON_DPAD_RIGHT | _360_JOY_BUTTON_LSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_PAGEUP,						_360_JOY_BUTTON_LT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_PAGEDOWN,					_360_JOY_BUTTON_BACK);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_RIGHT_SCROLL,				_PSV_JOY_BUTTON_R1);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_LEFT_SCROLL,					_PSV_JOY_BUTTON_L1);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_PAUSEMENU,					_PSV_JOY_BUTTON_START);

	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_STICK_PRESS,					_PSV_JOY_BUTTON_DPAD_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_OTHER_STICK_PRESS,			_360_JOY_BUTTON_RTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_OTHER_STICK_UP,				_360_JOY_BUTTON_RSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_OTHER_STICK_DOWN,			_360_JOY_BUTTON_RSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_OTHER_STICK_LEFT,			_360_JOY_BUTTON_RSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_OTHER_STICK_RIGHT,			_360_JOY_BUTTON_RSTICK_RIGHT);

	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_JUMP,					_PSV_JOY_BUTTON_X);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_FORWARD,				_360_JOY_BUTTON_LSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_BACKWARD,				_360_JOY_BUTTON_LSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_LEFT,					_360_JOY_BUTTON_LSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_RIGHT,					_360_JOY_BUTTON_LSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_LOOK_LEFT,				_360_JOY_BUTTON_RSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_LOOK_RIGHT,				_360_JOY_BUTTON_RSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_LOOK_UP,				_360_JOY_BUTTON_RSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_LOOK_DOWN,				_360_JOY_BUTTON_RSTICK_DOWN);

	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_USE,					_PSV_JOY_BUTTON_L1);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_ACTION,					_PSV_JOY_BUTTON_R1);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_RIGHT_SCROLL,			_PSV_JOY_BUTTON_DPAD_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_LEFT_SCROLL,			_PSV_JOY_BUTTON_DPAD_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_INVENTORY,				_PSV_JOY_BUTTON_TRIANGLE);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_PAUSEMENU,				_360_JOY_BUTTON_START);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_DROP,					_PSV_JOY_BUTTON_O);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_SNEAK_TOGGLE,			_PSV_JOY_BUTTON_DPAD_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_CRAFTING,				_PSV_JOY_BUTTON_SQUARE);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_RENDER_THIRD_PERSON,	_PSV_JOY_BUTTON_DPAD_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_GAME_INFO,				_360_JOY_BUTTON_BACK);

	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_DPAD_LEFT,				_PSV_JOY_BUTTON_DPAD_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_DPAD_RIGHT,				_PSV_JOY_BUTTON_DPAD_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_DPAD_UP,				_PSV_JOY_BUTTON_DPAD_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_DPAD_DOWN,				_PSV_JOY_BUTTON_DPAD_DOWN);
	/////////////////////////////////////
	// VITA TV (Dualshock 3/4 mapping)
	/////////////////////////////////////

	// Note: this is mapping 0 from the PS3 version

	// Split into Menu actions, and in-game actions
	if(InputManager.IsCircleCrossSwapped() )
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
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_PAGEUP,						_360_JOY_BUTTON_LT);
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

	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_JUMP,					_360_JOY_BUTTON_A);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_FORWARD,				_360_JOY_BUTTON_LSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_BACKWARD,				_360_JOY_BUTTON_LSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_LEFT,					_360_JOY_BUTTON_LSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_RIGHT,					_360_JOY_BUTTON_LSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_LOOK_LEFT,				_360_JOY_BUTTON_RSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_LOOK_RIGHT,				_360_JOY_BUTTON_RSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_LOOK_UP,				_360_JOY_BUTTON_RSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_LOOK_DOWN,				_360_JOY_BUTTON_RSTICK_DOWN);

	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_USE,					_360_JOY_BUTTON_LT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_ACTION,					_360_JOY_BUTTON_RT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_RIGHT_SCROLL,			_360_JOY_BUTTON_RB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_LEFT_SCROLL,			_360_JOY_BUTTON_LB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_INVENTORY,				_360_JOY_BUTTON_Y);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_PAUSEMENU,				_360_JOY_BUTTON_START);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_DROP,					_360_JOY_BUTTON_B);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_SNEAK_TOGGLE,			_360_JOY_BUTTON_RTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_CRAFTING,				_360_JOY_BUTTON_X);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_RENDER_THIRD_PERSON,	_360_JOY_BUTTON_LTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_GAME_INFO,				_360_JOY_BUTTON_BACK);
	
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_DPAD_LEFT,				_360_JOY_BUTTON_DPAD_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_DPAD_RIGHT,				_360_JOY_BUTTON_DPAD_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_DPAD_UP,				_360_JOY_BUTTON_DPAD_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_DPAD_DOWN,				_360_JOY_BUTTON_DPAD_DOWN);	
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


void debugSaveGameDirect()
{

	C4JThread* thread = new C4JThread(&IUIScene_PauseMenu::SaveWorldThreadProc, NULL, "debugSaveGameDirect");
	thread->Run();
	thread->WaitForCompletion(1000);
}

int simpleMessageBoxCallback(	UINT uiTitle, UINT uiText, 
							 UINT *uiOptionA, UINT uiOptionC, DWORD dwPad,
							 int(*Func) (LPVOID,int,const C4JStorage::EMessageResult),
							 LPVOID lpParam )
{
	ui.RequestErrorMessage(	uiTitle, uiText, uiOptionA, uiOptionC, dwPad, Func, lpParam);

	return 0;
}

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

	//Chris TODO
	ProfileManager.SetRichPresenceSettingFn(SQRNetworkManager_Vita::SetRichPresence);
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



void LoadSysModule(uint16_t module, const char* moduleName)
{
	int ret = sceSysmoduleLoadModule(module);
	if(ret != SCE_OK)
	{
#ifndef _CONTENT_PACKAGE
		printf("Error sceSysmoduleLoadModule %s failed (%d) \n", moduleName, ret );
		// are you running the debugger and don't have the Debugging/Mapping File set?  - $(ProjectDir)\PSVita\configuration.psp2path
#endif
		assert(0);
	}
}

#define LOAD_PSVITA_MODULE(m) LoadSysModule(m, #m)


int LoadSysModules()
{
	//	LOAD_PSVITA_MODULE(SCE_SYSMODULE_PERF);
	LOAD_PSVITA_MODULE(SCE_SYSMODULE_ULT);
	LOAD_PSVITA_MODULE(SCE_SYSMODULE_RUDP);
	LOAD_PSVITA_MODULE(SCE_SYSMODULE_NP_MATCHING2);
	LOAD_PSVITA_MODULE(SCE_SYSMODULE_NET);
	LOAD_PSVITA_MODULE(SCE_SYSMODULE_PSPNET_ADHOC);
	LOAD_PSVITA_MODULE(SCE_SYSMODULE_NET_ADHOC_MATCHING);
 	LOAD_PSVITA_MODULE(SCE_SYSMODULE_HTTP);
	LOAD_PSVITA_MODULE(SCE_SYSMODULE_HTTPS);
	LOAD_PSVITA_MODULE(SCE_SYSMODULE_NP_COMMERCE2);
	LOAD_PSVITA_MODULE(SCE_SYSMODULE_NP_SCORE_RANKING);

	return 0;
}


int main()
{
	//int* a = new int(5);

	PSVitaInit();

	ShutdownManager::Initialise();

	SceKernelFreeMemorySizeInfo mem_info;
	mem_info.size = sizeof(SceKernelFreeMemorySizeInfo);
	int Err = sceKernelGetFreeMemorySize(&mem_info);

#ifndef _CONTENT_PACKAGE
	printf("------------------------------------------------------\n");
	printf("------------------------------------------------------\n");
	//	printf("total_user_memory : %.02f\n", mem_info.total_user_memory / (1024.0f*1024.0f));
	printf("available_user_memory : %.02f\n", mem_info.sizeMain / (1024.0f*1024.0f));
	printf("available_video_memory : %.02f\n", mem_info.sizeCdram / (1024.0f*1024.0f));
	printf("------------------------------------------------------\n");
	printf("------------------------------------------------------\n");
#endif

	//
	// initialize the Vita
	//	

	int ihddFreeSizeKB=LoadSysModules();

	/* Set the parameters to be passed to initialization function sceAppUtilInit() */
	SceAppUtilInitParam initParam;
	SceAppUtilBootParam bootParam;
	memset(&initParam, 0, sizeof(SceAppUtilInitParam));
	memset(&bootParam, 0, sizeof(SceAppUtilBootParam));

	/* Initialize the library */
	SceInt32 ret = sceAppUtilInit( &initParam, &bootParam );



	static bool bTrialTimerDisplayed=true;

	// 4J-JEV: Moved this here in case some archived files are compressed.
	//	Compression::CreateNewThreadStorage();

	app.loadMediaArchive();
	RenderManager.Initialise();

	// Read the file containing the product codes
	if(app.ReadProductCodes()==FALSE)
	{
		// can't continue
		app.FatalLoadError();
	}

	app.InitTime();
	PSVitaNPToolkit::init();

	// initialise the storage manager with a default save display name, a Minimum save size, and a callback for displaying the saving message
	StorageManager.Init( 0, L"savegame.dat", "savePackName", FIFTY_ONE_MB, &CConsoleMinecraftApp::DisplaySavingMessage, (LPVOID)&app, NULL);
	StorageManager.SetDLCProductCode(app.GetProductCode());
	StorageManager.SetProductUpgradeKey(app.GetUpgradeKey());
	ProfileManager.SetServiceID(app.GetCommerceCategory());

	bool bCircleCrossSwapped = false;
	switch(app.GetProductSKU())
	{
	case e_sku_SCEE:
		// 4J-PB - need to be online to do this check, so let's stick with the 7+, and move this
		/*if(StorageManager.GetBootTypeDisc())
		{*/
		// set Europe age, then hone down specific countries
		ProfileManager.SetMinimumAge(7,0); // PEGI 7+
		ProfileManager.SetGermanyMinimumAge(6); // USK 6+
		ProfileManager.SetAustraliaMinimumAge(8); // PG rating has no age, but for some reason the testers are saying it's 8
		ProfileManager.SetRussiaMinimumAge(6);
		ProfileManager.SetKoreaMinimumAge(0);
		ProfileManager.SetJapanMinimumAge(0);
		/*}
		else
		{
			// PEGI 7+
			ProfileManager.SetMinimumAge(7,0);
		}*/
		break;
	case e_sku_SCEA:
		// ESRB EVERYONE 10+
		ProfileManager.SetMinimumAge(10,1);
		break;
	case e_sku_SCEJ:
		// CERO A
		ProfileManager.SetMinimumAge(0,2);
		bCircleCrossSwapped = true;
		break;
	}

	InputManager.SetCircleCrossSwapped(bCircleCrossSwapped);

	app.loadStringTable();

	// Vita
	//g_iScreenWidth = 720;
	//g_iScreenHeight = 408;

	// Vita native
	//g_iScreenWidth = 960;
	//g_iScreenHeight = 544;
	ui.init(720, 408);

	////////////////
	// Initialise //
	////////////////

	// Set the number of possible joypad layouts that the user can switch between, and the number of actions
	InputManager.Initialise(1,3,MINECRAFT_ACTION_MAX, ACTION_MAX_MENU);

	// Set the default joypad action mappings for Minecraft
	DefineActions();
	InputManager.SetJoypadMapVal(0,0);
	InputManager.SetKeyRepeatRate(0.3f,0.2f);

	//Minimum age must be set prior to profile init

	ProfileManager.Initialise( 
 		s_npCommunicationConfig,
		app.GetCommerceCategory(),// 		s_serviceId,
		PROFILE_VERSION_CURRENT,
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

	// defaults
	StorageManager.ResetSaveData();
	StorageManager.SetSaveTitle(L"Default Save");
	StorageManager.SetGameSaveFolderTitle((WCHAR *)app.GetString(IDS_GAMENAME));
	StorageManager.SetSaveCacheFolderTitle((WCHAR *)app.GetString(IDS_SAVECACHEFILE));
	StorageManager.SetOptionsFolderTitle((WCHAR *)app.GetString(IDS_OPTIONSFILE));
	StorageManager.SetGameSaveFolderPrefix(app.GetSaveFolderPrefix());	
	StorageManager.SetMaxSaves(99);

	byteArray baOptionsIcon = app.getArchiveFile(L"DefaultOptionsImage320x176.png");
	byteArray baSaveThumbnail = app.getArchiveFile(L"DefaultSaveThumbnail64x64.png");
	byteArray baSaveImage = app.getArchiveFile(L"DefaultSaveImage320x176.png");

	StorageManager.InitialiseProfileData(PROFILE_VERSION_CURRENT,
		NUM_PROFILE_VALUES,
		NUM_PROFILE_SETTINGS,
		dwProfileSettingsA,
		app.GAME_DEFINED_PROFILE_DATA_BYTES*XUSER_MAX_COUNT,
		&app.uiGameDefinedDataChangedBitmask);

	StorageManager.SetDefaultImages((PBYTE)baOptionsIcon.data, baOptionsIcon.length,(PBYTE)baSaveImage.data, baSaveImage.length,(PBYTE)baSaveThumbnail.data, baSaveThumbnail.length);

	if(baOptionsIcon.data!=NULL){ delete [] baOptionsIcon.data;	}
	if(baSaveThumbnail.data!=NULL){	delete [] baSaveThumbnail.data; }
	if(baSaveImage.data!=NULL){	delete [] baSaveImage.data;	}

	StorageManager.SetIncompleteSaveCallback(CConsoleMinecraftApp::Callback_SaveGameIncomplete, (LPVOID)&app);

#if 0
	// Set up the global title storage path
	StorageManager.StoreTMSPathName();
#endif

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
	StorageManager.SetOldProfileVersionCallback(&CConsoleMinecraftApp::OldProfileVersionCallback,(LPVOID)&app);

#if 0
	// Set a callback for when there is a read error on profile data
	//StorageManager.SetProfileReadErrorCallback(&CConsoleMinecraftApp::ProfileReadErrorCallback,(LPVOID)&app);
#endif

	StorageManager.SetDLCInfoMap(app.GetSonyDLCMap());
	app.CommerceInit(); //  MGH - moved this here so GetCommerce isn't NULL

	// 4J-PB - Kick of the check for trial or full version - requires ui to be initialised
	app.GetCommerce()->CheckForTrialUpgradeKey();



	// debug switch to trial version
	//ProfileManager.SetDebugFullOverride(false);

	//ProfileManager.AddDLC(2);
	StorageManager.SetDLCPackageRoot("DLCDrive");
#if 0
	StorageManager.RegisterMarketplaceCountsCallback(&CConsoleMinecraftApp::MarketplaceCountsCallback,(LPVOID)&app);
	// Kinect !

	if(XNuiGetHardwareStatus()!=0)
	{
		// If the Kinect Sensor is not physically connected, this function returns 0.
		NuiInitialize(NUI_INITIALIZE_FLAG_USES_HIGH_QUALITY_COLOR | NUI_INITIALIZE_FLAG_USES_DEPTH |
			NUI_INITIALIZE_FLAG_EXTRAPOLATE_FLOOR_PLANE | NUI_INITIALIZE_FLAG_USES_FITNESS | NUI_INITIALIZE_FLAG_NUI_GUIDE_DISABLED | NUI_INITIALIZE_FLAG_SUPPRESS_AUTOMATIC_UI,NUI_INITIALIZE_DEFAULT_HARDWARE_THREAD );
	}

	// Sentient !
	hr = SentientManager.Init();

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
	FireworksRecipe::CreateNewThreadStorage();

	Minecraft::main();
	Minecraft *pMinecraft=Minecraft::GetInstance();

	//#if 0
	//bool bDisplayPauseMenu=false;

	// set the default gamma level
	float fVal=50.0f*327.68f;
	RenderManager.UpdateGamma((unsigned short)fVal);

	// load any skins
	//app.AddSkinsToMemoryTextureFiles();

	// set the achievement text for a trial achievement, now we have the string table loaded
	//Chris TODO
	//ProfileManager.SetTrialTextStringTable(app.GetStringTable(),IDS_CONFIRM_OK, IDS_CONFIRM_CANCEL);
	ProfileManager.SetTrialAwardText(eAwardType_Achievement,IDS_UNLOCK_TITLE,IDS_UNLOCK_ACHIEVEMENT_TEXT);
	//ProfileManager.SetTrialAwardText(eAwardType_GamerPic,IDS_UNLOCK_TITLE,IDS_UNLOCK_GAMERPIC_TEXT);
	//ProfileManager.SetTrialAwardText(eAwardType_AvatarItem,IDS_UNLOCK_TITLE,IDS_UNLOCK_AVATAR_TEXT);
	ProfileManager.SetTrialAwardText(eAwardType_Theme,IDS_UNLOCK_TITLE,IDS_UNLOCK_THEME_TEXT);
	ProfileManager.SetUpsellCallback(&app.UpsellReturnedCallback,&app);

	// Set up a debug character press sequence
#ifndef _FINAL_BUILD
	app.SetDebugSequence("LRLRYYY");
#endif

	// Initialise the social networking manager.
	//Chris TODO
	//CSocialManager::Instance()->Initialise();

	// Update the base scene quick selects now that the minecraft class exists
	//CXuiSceneBase::UpdateScreenSettings(0);
	//#endif
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

	// wait for the trophy init to complete - nonblocking semaphore
	while(( !ProfileManager.AreTrophiesInstalled() ) && ShutdownManager::ShouldRun(ShutdownManager::eMainThread))
	{
		RenderManager.StartFrame();
		ProfileManager.Tick();
		RenderManager.Tick();
		RenderManager.Present();		
	}

	// QNet needs to be setup after profile manager, as we do not want its Notify listener to handle
	// XN_SYS_SIGNINCHANGED notifications. This does mean that we need to have a callback in the
	// ProfileManager for XN_LIVE_INVITE_ACCEPTED for QNet.
	g_NetworkManager.Initialise();
	g_NetworkManager.SetLocalGame(true);

	// Set the default sound levels
	pMinecraft->options->set(Options::Option::MUSIC,1.0f);
	pMinecraft->options->set(Options::Option::SOUND,1.0f);

	app.InitGameSettings();
	// read the options here for controller 0 - this won't actually be actioned until a storagemanager tick later
	StorageManager.ReadFromProfile(0);

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
	while( true )
	{
		SonyVoiceChat_Vita::tick();

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

		pMinecraft->gameRenderer->ApplyGammaPostProcess();

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

			//Check to see if we can post to social networks.
			//CD - Removing this until support exists
			//CSocialManager::Instance()->RefreshPostingCapability();

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
#if 0 
		PIXBeginNamedEvent(0,"Misc extra xui");
		// Update XUI Timers
		hr = XuiTimersRun();

#endif // #if 0
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
		app.AppEventTick();

		app.SaveDataTick();

		// Fix for #7318 - Title crashes after short soak in the leaderboards menu
		// A memory leak was caused because the icon renderer kept creating new Vec3's because the pool wasn't reset
		Vec3::resetPool();

		//		sceRazorCpuSync();
#if 0 //ndef _CONTENT_PACKAGE
		if( InputManager.ButtonDown(0, MINECRAFT_ACTION_DPAD_LEFT) )
		{
			malloc_managed_size mmsize;
			malloc_stats(&mmsize);
			app.DebugPrintf("Free mem = %d\n", (mmsize.current_system_size - mmsize.current_inuse_size) / (1024*1024));
		}
#endif
	}

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
