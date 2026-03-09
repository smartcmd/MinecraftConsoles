// Minecraft.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <xbdm.h>
#include <assert.h>
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
#include "Social\SocialManager.h"
#include "Leaderboards\XboxLeaderboardManager.h"
#include "..\Common\XUI\XUI_Scene_Container.h"
#include "..\..\Minecraft.Client\Tesselator.h"
#include "..\..\Minecraft.Client\Options.h"
#include "Sentient\SentientManager.h"
#include "..\..\Minecraft.World\IntCache.h"
#include "..\Textures.h"
#include "..\..\Minecraft.World\compression.h"
#include "..\..\Minecraft.World\OldChunkStorage.h"
//#include "..\..\Minecraft.Client\SoundEngine.h"

#include <NuiApi.h>

void GetRenderAndSamplerStates(IDirect3DDevice9 *pDevice,DWORD *RenderStateA,DWORD *SamplerStateA);
void SetRenderAndSamplerStates(IDirect3DDevice9 *pDevice,DWORD *RenderStateA,DWORD *SamplerStateA);

// 4J-PB - Done - #pragma message(__LOC__"Theme should be GREEN SIGNED before _FINAL_BUILD")

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
	XPROFILE_OPTION_CONTROLLER_VIBRATION,
	XPROFILE_GAMER_YAXIS_INVERSION,
	XPROFILE_GAMER_CONTROL_SENSITIVITY,
	XPROFILE_GAMER_ACTION_MOVEMENT_CONTROL,
	XPROFILE_TITLE_SPECIFIC1,
};

D3DRENDERSTATETYPE RenderStateModes[]=
{
	D3DRS_FILLMODE,
	D3DRS_CULLMODE,
	D3DRS_ALPHABLENDENABLE,
	D3DRS_VIEWPORTENABLE,
	D3DRS_BLENDOP,
	D3DRS_SRCBLEND,
	D3DRS_DESTBLEND,
	D3DRS_COLORWRITEENABLE
};

D3DSAMPLERSTATETYPE SamplerStateModes[]=
{
	D3DSAMP_MINFILTER,
	D3DSAMP_MAGFILTER,
	D3DSAMP_MIPFILTER,
	D3DSAMP_ADDRESSU,
	D3DSAMP_ADDRESSV
};

//-------------------------------------------------------------------------------------
// Time             Since fAppTime is a float, we need to keep the quadword app time 
//                  as a LARGE_INTEGER so that we don't lose precision after running
//                  for a long time.
//-------------------------------------------------------------------------------------


BOOL g_bWidescreen = TRUE;


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
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_PAUSEMENU,					_360_JOY_BUTTON_START);

	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_STICK_PRESS,					_360_JOY_BUTTON_LTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OTHER_STICK_PRESS,			_360_JOY_BUTTON_RTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OTHER_STICK_UP,				_360_JOY_BUTTON_RSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OTHER_STICK_DOWN,			_360_JOY_BUTTON_RSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OTHER_STICK_LEFT,			_360_JOY_BUTTON_RSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OTHER_STICK_RIGHT,			_360_JOY_BUTTON_RSTICK_RIGHT);

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
	
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_DPAD_LEFT,				_360_JOY_BUTTON_DPAD_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_DPAD_RIGHT,				_360_JOY_BUTTON_DPAD_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_DPAD_UP,				_360_JOY_BUTTON_DPAD_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_DPAD_DOWN,				_360_JOY_BUTTON_DPAD_DOWN);	
}




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
		nullptr,
		D3DCREATE_HARDWARE_VERTEXPROCESSING|D3DCREATE_BUFFER_2_FRAMES,
		pd3dPP,
		ppDevice );
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

IDirect3DDevice9 *g_pD3DDevice;

//-------------------------------------------------------------------------------------
// Name: main()
// Desc: The application's entry point
//-------------------------------------------------------------------------------------
int __cdecl main()
{
	IDirect3DDevice9 *pDevice;
	D3DPRESENT_PARAMETERS d3dpp;
	HRESULT hr;
	static bool bTrialTimerDisplayed=true;

	//__debugbreak();

#ifdef MEMORY_TRACKING
	ResetMem();
	MEMORYSTATUS memStat;
	GlobalMemoryStatus(&memStat);
	printf("RESETMEM start: Avail. phys %d\n",memStat.dwAvailPhys/(1024*1024));
#endif
	// capture the launch data if there is any
	app.StoreLaunchData();

#ifdef _DEBUG
	// Output more verbose errors for sessions
	XDebugSetSystemOutputLevel( HXAMAPP_XGI, 3 );
#endif

//	DumpMem();

	// Declare an instance of the XUI framework.
	
	// XUI render can change the following renderstates, so store then before and re-set them after the xui render
	/* The following D3DRENDERSTATETYPE D3D render states.

		D3DRS_FILLMODE

		D3DRS_CULLMODE

		D3DRS_ALPHABLENDENABLE

		D3DRS_VIEWPORTENABLE

		D3DRS_BLENDOP

		D3DRS_SRCBLEND

		D3DRS_DESTBLEND

		D3DRS_COLORWRITEENABLE
		*/

	DWORD RenderStateA[8];
	DWORD SamplerStateA[5];
	DWORD RenderStateA2[8];
	DWORD SamplerStateA2[5];

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
	
	RenderManager.Initialise(pDevice);

	////////////////
	// Initialise //
	////////////////

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
	hr = g_pXAudio2->CreateMasteringVoice(&g_pXAudio2MasteringVoice, XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE, 0, 0, nullptr);
	if ( FAILED( hr ) )
	{
		app.DebugPrintf( "Creating XAudio2 mastering voice failed (err = 0x%08x)!\n", hr );
		app.FatalLoadError();
	}

	app.InitTime();

	// Set the number of possible joypad layouts that the user can switch between, and the number of actions
	InputManager.Initialise(1,3,MINECRAFT_ACTION_MAX, ACTION_MAX_MENU);

	// Set the default joypad action mappings for Minecraft
	DefineActions();
	InputManager.SetJoypadMapVal(0,0);
	InputManager.SetKeyRepeatRate(0.3f,0.2f);


	// Initialise the profile manager with the game Title ID, Offer ID, a profile version number, and the number of profile values and settings
	ProfileManager.Initialise(TITLEID_MINECRAFT,
							app.m_dwOfferID,
							PROFILE_VERSION_10,
							NUM_PROFILE_VALUES,
							NUM_PROFILE_SETTINGS,
							dwProfileSettingsA,
							app.GAME_DEFINED_PROFILE_DATA_BYTES*XUSER_MAX_COUNT,
							&app.uiGameDefinedDataChangedBitmask
							);

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

	ProfileManager.RegisterAward(eAward_mine100Blocks,		GAMER_PICTURE_GAMERPIC1,			eAwardType_GamerPic,false,app.GetStringTable(),IDS_AWARD_TITLE,IDS_AWARD_GAMERPIC1,IDS_CONFIRM_OK);
	ProfileManager.RegisterAward(eAward_kill10Creepers,		GAMER_PICTURE_GAMERPIC2,			eAwardType_GamerPic,false,app.GetStringTable(),IDS_AWARD_TITLE,IDS_AWARD_GAMERPIC2,IDS_CONFIRM_OK);

	ProfileManager.RegisterAward(eAward_eatPorkChop,		AVATARASSETAWARD_PORKCHOP_TSHIRT,	eAwardType_AvatarItem,false,app.GetStringTable(),IDS_AWARD_TITLE,IDS_AWARD_AVATAR1,IDS_CONFIRM_OK);
	ProfileManager.RegisterAward(eAward_play100Days,		AVATARASSETAWARD_WATCH,				eAwardType_AvatarItem,false,app.GetStringTable(),IDS_AWARD_TITLE,IDS_AWARD_AVATAR2,IDS_CONFIRM_OK);
	ProfileManager.RegisterAward(eAward_arrowKillCreeper,	AVATARASSETAWARD_CAP,				eAwardType_AvatarItem,false,app.GetStringTable(),IDS_AWARD_TITLE,IDS_AWARD_AVATAR3,IDS_CONFIRM_OK);

	ProfileManager.RegisterAward(eAward_socialPost,			0,									eAwardType_Theme,false,app.GetStringTable(),IDS_AWARD_TITLE,IDS_AWARD_THEME,IDS_CONFIRM_OK,THEME_NAME,THEME_FILESIZE);

	// Rich Presence init - number of presences, number of contexts
	ProfileManager.RichPresenceInit(4,1);
	ProfileManager.RegisterRichPresenceContext(CONTEXT_GAME_STATE);

	app.LoadXuiResources();

	// initialise the storage manager with a default save display name, a Minimum save size, and a callback for displaying the saving message
	StorageManager.Init(app.GetString(IDS_DEFAULT_SAVENAME),"savegame.dat",FIFTY_ONE_MB,&CConsoleMinecraftApp::DisplaySavingMessage,(LPVOID)&app,GROUP_ID);
	// Set up the global title storage path
	StorageManager.StoreTMSPathName();

	// set a function to be called when there's a sign in change, so we can exit a level if the primary player signs out
	ProfileManager.SetSignInChangeCallback(&CConsoleMinecraftApp::SignInChangeCallback,(LPVOID)&app);

	// set a function to be called when the ethernet is disconnected, so we can back out if required
	ProfileManager.SetNotificationsCallback(&CConsoleMinecraftApp::NotificationsCallback,(LPVOID)&app);
	
	// Set a callback for the default player options to be set - when there is no profile data for the player
	ProfileManager.SetDefaultOptionsCallback(&CConsoleMinecraftApp::DefaultOptionsCallback,(LPVOID)&app);
	// Set a callback to deal with old profile versions needing updated to new versions
	ProfileManager.SetOldProfileVersionCallback(&CConsoleMinecraftApp::OldProfileVersionCallback,(LPVOID)&app);

	// Set a callback for when there is a read error on profile data
	ProfileManager.SetProfileReadErrorCallback(&CConsoleMinecraftApp::ProfileReadErrorCallback,(LPVOID)&app);


	// QNet needs to be setup after profile manager, as we do not want its Notify listener to handle
	// XN_SYS_SIGNINCHANGED notifications. This does mean that we need to have a callback in the
	// ProfileManager for XN_LIVE_INVITE_ACCEPTED for QNet.
	g_NetworkManager.Initialise();

	app.InitGameSettings();

	// debug switch to trial version
	ProfileManager.SetDebugFullOverride(true);

	//ProfileManager.AddDLC(2);
	StorageManager.SetDLCPackageRoot("DLCDrive");
	StorageManager.RegisterMarketplaceCountsCallback(&CConsoleMinecraftApp::MarketplaceCountsCallback,(LPVOID)&app);
	// Kinect !

// 	if(XNuiGetHardwareStatus()!=0)
// 	{
// 		// If the Kinect Sensor is not physically connected, this function returns 0. 
// 		NuiInitialize(NUI_INITIALIZE_FLAG_USES_HIGH_QUALITY_COLOR | NUI_INITIALIZE_FLAG_USES_DEPTH | 
// 			NUI_INITIALIZE_FLAG_EXTRAPOLATE_FLOOR_PLANE | NUI_INITIALIZE_FLAG_USES_FITNESS | NUI_INITIALIZE_FLAG_NUI_GUIDE_DISABLED | NUI_INITIALIZE_FLAG_SUPPRESS_AUTOMATIC_UI,NUI_INITIALIZE_DEFAULT_HARDWARE_THREAD );
// 	}

	// Sentient !
	hr = TelemetryManager->Init();  

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
	Minecraft *pMinecraft=Minecraft::GetInstance();
	//bool bDisplayPauseMenu=false;

	// set the default gamma level
	float fVal=50.0f*327.68f;
	RenderManager.UpdateGamma((unsigned short)fVal);

	// load any skins
	//app.AddSkinsToMemoryTextureFiles();

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

	app.InitialiseTips();

	DWORD initData=0;



#ifndef _FINAL_BUILD
#ifndef _DEBUG
	#pragma message(__LOC__"Need to define the _FINAL_BUILD before submission")
#endif
#endif

	// 4J-PB - Having to add in code to stop people being able to play a build from Partnernet unless they know the code
#ifdef _CONTENT_PACKAGE
#ifndef _FINAL_BUILD
// 	app.SetPartnernetPasswordRunning(true);
// 
// 	app.NavigateToScene(XUSER_INDEX_ANY,eUIScene_PartnernetPassword,&initData);
// 	
// 	while(app.PartnernetPasswordRunning())
// 	{
// 		ProfileManager.Tick();
// 		InputManager.Tick(); // need to ruin this for the edit box
// 
// 		// Tick XUI
// 		app.RunFrame();
// 
// 		// 4J : WESTY : Added to ensure we always have clear background for intro.
// 		RenderManager.SetClearColour(D3DCOLOR_RGBA(0,0,0,255));
// 		RenderManager.Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
// 
// 		// Render XUI
// 		hr = app.Render();
// 
// 		// Present the frame.
// 		RenderManager.Present();
// 
// 		// Update XUI Timers
// 		hr = XuiTimersRun();
// 	}
#endif
#endif
	// Set the default sound levels
	pMinecraft->options->set(Options::Option::MUSIC,1.0f);
	pMinecraft->options->set(Options::Option::SOUND,1.0f);

	app.NavigateToScene(XUSER_INDEX_ANY,eUIScene_Intro,&initData);

	//Sleep(10000);
	
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

	while( TRUE ) 
	{
		if(pMinecraft->soundEngine->isStreamingWavebankReady() &&
			!pMinecraft->soundEngine->GetIsPlayingStreamingGameMusic() &&
			!pMinecraft->soundEngine->GetIsPlayingStreamingCDMusic() )
		{
			// play some music in the menus
			pMinecraft->soundEngine->playStreaming(L"", 0, 0, 0, 0, 0, false);
		}
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
		CSocialManager::Instance()->Tick();
		PIXEndNamedEvent();
		
		// Tick sentient.
		PIXBeginNamedEvent(0,"Sentient tick");
		MemSect(37);
		TelemetryManager->Tick();
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
			app.SetAppPaused( g_NetworkManager.IsLocalGame() && g_NetworkManager.GetPlayerCount() == 1 && app.IsPauseMenuDisplayed(ProfileManager.GetPrimaryPad()) );
		}
		else
		{
			MemSect(28);
			pMinecraft->soundEngine->tick(nullptr, 0.0f);
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
		// Present the frame.
		PIXBeginNamedEvent(0,"Frame present");
		RenderManager.Present();
		PIXEndNamedEvent();

		app.CheckMenuDisplayed();
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

					// Update the selected item position based on the user's UI setting
					CXuiSceneBase::UpdateSelectedItemPos(i);
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

		// Any threading type things to deal with from the xui side?
		app.HandleXuiActions();
		PIXEndNamedEvent();

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
				CXuiSceneBase::UpdateTrialTimer(ProfileManager.GetPrimaryPad());
			}
		}
		else
		{
			// need to turn off the trial timer if it was on , and we've unlocked the full version
			if(bTrialTimerDisplayed)
			{
				CXuiSceneBase::ShowTrialTimer(FALSE);
				bTrialTimerDisplayed=false;
			}
		}

		// Fix for #7318 - Title crashes after short soak in the leaderboards menu
		// A memory leak was caused because the icon renderer kept creating new Vec3's because the pool wasn't reset
		Vec3::resetPool();
	}

	// Free resources, unregister custom classes, and exit.
	app.Uninit();
	pDevice->Release();
}


void GetRenderAndSamplerStates(IDirect3DDevice9 *pDevice,DWORD *RenderStateA,DWORD *SamplerStateA)
{
	for(int i=0;i<8;i++)
	{
		pDevice->GetRenderState(RenderStateModes[i],&RenderStateA[i]);
	}

	for(int i=0;i<5;i++)
	{
		pDevice->GetSamplerState(0,SamplerStateModes[i],&SamplerStateA[i]);
	}
}

void SetRenderAndSamplerStates(IDirect3DDevice9 *pDevice,DWORD *RenderStateA,DWORD *SamplerStateA)
{
	for(int i=0;i<8;i++)
	{
		pDevice->SetRenderState(RenderStateModes[i],RenderStateA[i]);
	}

	for(int i=0;i<5;i++)
	{
		pDevice->SetSamplerState(0,SamplerStateModes[i],SamplerStateA[i]);
	}
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
	for(auto it = allocCounts.begin(); it != allocCounts.end(); it++ )
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
	const int MAX_SECT = 56;

	int totals[MAX_SECT] = {0};

	for(auto it = allocCounts.begin(); it != allocCounts.end(); it++ )
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
