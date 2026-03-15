#include "stdafx.h"
#include "..\..\Minecraft.World\net.minecraft.world.entity.item.h"
#include "..\..\Minecraft.World\net.minecraft.world.entity.player.h"
#include "..\..\Minecraft.World\net.minecraft.world.level.tile.entity.h"
#include "..\..\Minecraft.World\net.minecraft.world.phys.h"
#include "..\..\Minecraft.World\InputOutputStream.h"
#include "..\..\Minecraft.World\compression.h"
#include "..\Options.h"
#include "..\MinecraftServer.h"
#include "..\MultiPlayerLevel.h"
#include "..\GameRenderer.h"
#include "..\ProgressRenderer.h"
#include "..\LevelRenderer.h"
#include "..\MobSkinMemTextureProcessor.h"
#include "..\Minecraft.h"
#include "..\ClientConnection.h"
#include "..\MultiPlayerLocalPlayer.h"
#include "..\LocalPlayer.h"
#include "..\..\Minecraft.World\Player.h"
#include "..\..\Minecraft.World\Inventory.h"
#include "..\..\Minecraft.World\Level.h"
#include "..\..\Minecraft.World\FurnaceTileEntity.h"
#include "..\..\Minecraft.World\Container.h"
#include "..\..\Minecraft.World\DispenserTileEntity.h"
#include "..\..\Minecraft.World\SignTileEntity.h"
#include "..\StatsCounter.h"
#include "..\GameMode.h"
#include "..\Xbox\Social\SocialManager.h"
#include "Tutorial\TutorialMode.h"
#if defined _XBOX || defined _WINDOWS64
#include "..\Xbox\XML\ATGXmlParser.h"
#include "..\Xbox\XML\xmlFilesCallback.h"
#endif
#include "Minecraft_Macros.h"
#include "..\PlayerList.h"
#include "..\ServerPlayer.h"
#include "GameRules\ConsoleGameRules.h"
#include "GameRules\ConsoleSchematicFile.h"
#include "..\User.h"
#include "..\..\Minecraft.World\LevelData.h"
#if defined(_WINDOWS64) && defined(MINECRAFT_SERVER_BUILD)
#include "..\..\Minecraft.Server\ServerLogManager.h"
#endif
#include "..\..\Minecraft.World\net.minecraft.world.entity.player.h"
#include "..\EntityRenderDispatcher.h"
#include "..\..\Minecraft.World\compression.h"
#include "..\TexturePackRepository.h"
#include "..\DLCTexturePack.h"
#include "DLC\DLCPack.h"
#include "..\StringTable.h"
#ifndef _XBOX
#include "..\ArchiveFile.h"
#endif
#include "..\Minecraft.h"
#ifdef _XBOX
#include "..\Xbox\GameConfig\Minecraft.spa.h"
#include "..\Xbox\Network\NetworkPlayerXbox.h"
#include "XUI\XUI_TextEntry.h"
#include "XUI\XUI_XZP_Icons.h"
#include "XUI\XUI_PauseMenu.h"
#else
#include "UI\UI.h"
#include "UI\UIScene_PauseMenu.h"
#endif
#ifdef __PS3__
#include <sys/tty.h>
#endif
#ifdef __ORBIS__
#include <save_data_dialog.h>
#endif

#include "..\Common\Leaderboards\LeaderboardManager.h"

//CMinecraftApp app;
unsigned int CMinecraftApp::m_uiLastSignInData = 0;

const float CMinecraftApp::fSafeZoneX = 64.0f; // 5% of 1280
const float CMinecraftApp::fSafeZoneY = 36.0f; // 5% of 720

int CMinecraftApp::s_iHTMLFontSizesA[eHTMLSize_COUNT] =
{
#ifdef _XBOX
	14,12,14,24
#else
	//20,15,20,24
	20,13,20,26
#endif
};


CMinecraftApp::CMinecraftApp()
{
	if(GAME_SETTINGS_PROFILE_DATA_BYTES != sizeof(GAME_SETTINGS))
	{
		// 4J Stu - See comment for GAME_SETTINGS_PROFILE_DATA_BYTES in Xbox_App.h
		DebugPrintf("WARNING: The size of the profile GAME_SETTINGS struct has changed, so all stat data is likely incorrect. Is: %d, Should be: %d\n",sizeof(GAME_SETTINGS),GAME_SETTINGS_PROFILE_DATA_BYTES);
#ifndef _CONTENT_PACKAGE
		__debugbreak();
#endif
	}

	for(int i=0;i<XUSER_MAX_COUNT;i++)
	{
		m_eTMSAction[i]=eTMSAction_Idle;
		m_eXuiAction[i]=eAppAction_Idle;
		m_eXuiActionParam[i] = nullptr;
		//m_dwAdditionalModelParts[i] = 0;

		if(FAILED(XUserGetSigninInfo(i,XUSER_GET_SIGNIN_INFO_OFFLINE_XUID_ONLY ,&m_currentSigninInfo[i])))
		{
			m_currentSigninInfo[i].xuid = INVALID_XUID;
			m_currentSigninInfo[i].dwGuestNumber = 0;
		}
		DebugPrintf("Player at index %d has guest number %d\n", i,m_currentSigninInfo[i].dwGuestNumber );

		m_bRead_BannedListA[i]=false;
		SetBanListCheck(i,false);

		m_uiOpacityCountDown[i]=0;

	}
	m_eGlobalXuiAction=eAppAction_Idle;
	m_eGlobalXuiServerAction=eXuiServerAction_Idle;

	m_bResourcesLoaded=false;
	m_bGameStarted=false;
	m_bIsAppPaused=false;
	//m_bSplitScreenEnabled = false;


	m_bIntroRunning=false;
	m_eGameMode=eMode_Singleplayer;
	m_bLoadSavesFromFolderEnabled = false;
	m_bWriteSavesToFolderEnabled = false;
	//m_bInterfaceRenderingOff = false;
	//m_bHandRenderingOff = false;
	m_bTutorialMode = false;
	m_disconnectReason = DisconnectPacket::eDisconnect_None;
	m_bLiveLinkRequired = false;
	m_bChangingSessionType = false;
	m_bReallyChangingSessionType = false;

#ifdef _DEBUG_MENUS_ENABLED

#ifdef _CONTENT_PACKAGE
	m_bDebugOptions=false; // make them off by default in a content package build
#else
	m_bDebugOptions=true;
#endif
#else
	m_bDebugOptions=false;
#endif

	//ZeroMemory(m_PreviewBuffer,sizeof(XSOCIAL_PREVIEWIMAGE)*XUSER_MAX_COUNT);

	m_xuidNotch = INVALID_XUID;

	ZeroMemory(&m_InviteData,sizeof(JoinFromInviteData) );

	// 	m_bRead_TMS_XUIDS_XML=false;
	// 	m_bRead_TMS_DLCINFO_XML=false;

	m_pDLCFileBuffer=nullptr;
	m_dwDLCFileSize=0;
	m_pBannedListFileBuffer=nullptr;
	m_dwBannedListFileSize=0;

	m_bDefaultCapeInstallAttempted=false;
	m_bDLCInstallProcessCompleted=false;
	m_bDLCInstallPending=false;
	m_iTotalDLC = 0;
	m_iTotalDLCInstalled = 0;
	mfTrialPausedTime=0.0f;
	m_uiAutosaveTimer=0;
	ZeroMemory(m_pszUniqueMapName,14);


	m_bNewDLCAvailable=false;
	m_bSeenNewDLCTip=false;

	m_uiGameHostSettings=0;

#ifdef _LARGE_WORLDS
	m_GameNewWorldSize = 0;
	m_bGameNewWorldSizeUseMoat = false;
	m_GameNewHellScale = 0;
#endif

	ZeroMemory(m_playerColours,MINECRAFT_NET_MAX_PLAYERS);

	m_iDLCOfferC=0;
	m_bAllDLCContentRetrieved=true;
	InitializeCriticalSection(&csDLCDownloadQueue);
	m_bAllTMSContentRetrieved=true;
	m_bTickTMSDLCFiles=true;
	InitializeCriticalSection(&csTMSPPDownloadQueue);
	InitializeCriticalSection(&csAdditionalModelParts);
	InitializeCriticalSection(&csAdditionalSkinBoxes);
	InitializeCriticalSection(&csAnimOverrideBitmask);
	InitializeCriticalSection(&csMemFilesLock);
	InitializeCriticalSection(&csMemTPDLock);

	InitializeCriticalSection(&m_saveNotificationCriticalSection);
	m_saveNotificationDepth = 0;

	m_dwRequiredTexturePackID=0;

	m_bResetNether=false;

#ifdef _XBOX
	//	m_bTransferSavesToXboxOne=false;
	//	m_uiTransferSlotC=5;
#endif

#if (defined _CONTENT_PACAKGE) || (defined _XBOX)
	m_bUseDPadForDebug = false;
#else
	m_bUseDPadForDebug = true;
#endif

#if ( defined __PS3__ || defined __ORBIS__ || defined _DURANGO  || defined __PSVITA__)
	for(int i=0;i<XUSER_MAX_COUNT;i++)
	{
		m_eOptionsStatusA[i]=C4JStorage::eOptions_Callback_Idle;
	}
#endif

	for(int i=0;i<XUSER_MAX_COUNT;i++)
	{
		m_vBannedListA[i] = new vector<PBANNEDLISTDATA>;
	}

	LocaleAndLanguageInit();

#ifdef _XBOX_ONE
	m_hasReachedMainMenu = false;
#endif
}



void CMinecraftApp::DebugPrintf(const char *szFormat, ...)
{

#ifndef _FINAL_BUILD
    va_list ap;
    va_start(ap, szFormat);
#if defined(_WINDOWS64) && defined(MINECRAFT_SERVER_BUILD)
    // Dedicated server routes client debug spew through ServerLogger so CLI output stays prompt-safe.
    if (ServerRuntime::ServerLogManager::ShouldForwardClientDebugLogs())
    {
        ServerRuntime::ServerLogManager::ForwardClientAppDebugLogV(szFormat, ap);
        va_end(ap);
        return;
    }
#endif
    char    buf[1024];
    vsnprintf(buf, sizeof(buf), szFormat, ap);
    va_end(ap);
    OutputDebugStringA(buf);
#endif

}

void CMinecraftApp::DebugPrintf(int user, const char *szFormat, ...)
{
#ifndef _FINAL_BUILD
    if(user == USER_NONE)
        return;
    va_list ap;
    va_start(ap, szFormat);
#if defined(_WINDOWS64) && defined(MINECRAFT_SERVER_BUILD)
    // Dedicated server routes client debug spew through ServerLogger so CLI output stays prompt-safe.
    if (ServerRuntime::ServerLogManager::ShouldForwardClientDebugLogs())
    {
        ServerRuntime::ServerLogManager::ForwardClientUserDebugLogV(user, szFormat, ap);
        va_end(ap);
        return;
    }
#endif
    char    buf[1024];
    vsnprintf(buf, sizeof(buf), szFormat, ap);
    va_end(ap);
#ifdef __PS3__
    unsigned int writelen;
    sys_tty_write(SYS_TTYP_USER1 + ( user - 1 ), buf, strlen(buf), &writelen );
#elif defined __PSVITA__
    switch(user)
    {
    case 0:
        {
            SceUID tty2 = sceIoOpen("tty2:", SCE_O_WRONLY, 0);
            if(tty2>=0)
            {
                std::string string1(buf);
                sceIoWrite(tty2, string1.c_str(), string1.length());
                sceIoClose(tty2);
            }
        }
        break;
    case 1:
        {
            SceUID tty3 = sceIoOpen("tty3:", SCE_O_WRONLY, 0);
            if(tty3>=0)
            {
                std::string string1(buf);
                sceIoWrite(tty3, string1.c_str(), string1.length());
                sceIoClose(tty3);
            }
        }
        break;
    default:
        OutputDebugStringA(buf);
        break;
    }
#else
    OutputDebugStringA(buf);
#endif
#ifndef _XBOX
    if(user == USER_UI)
    {
        ui.logDebugString(buf);
    }
#endif
#endif
}

LPCWSTR CMinecraftApp::GetString(int iID)
{
	//return L"Değişiklikler ve Yenilikler";
	//return L"ÕÕÕÕÖÖÖÖ";
	return app.m_stringTable->getString(iID);
}

void CMinecraftApp::SetAction(int iPad, eXuiAction action, LPVOID param)
{
	if( ( m_eXuiAction[iPad] == eAppAction_ReloadTexturePack ) && ( action == eAppAction_EthernetDisconnected ) )
	{
		app.DebugPrintf("Invalid change of App action for pad %d from %d to %d, ignoring\n", iPad, m_eXuiAction[iPad], action);
	}
	else if( ( m_eXuiAction[iPad] == eAppAction_ReloadTexturePack ) && ( action == eAppAction_ExitWorld ) )
	{
		app.DebugPrintf("Invalid change of App action for pad %d from %d to %d, ignoring\n", iPad, m_eXuiAction[iPad], action);
	}
	else if(m_eXuiAction[iPad] == eAppAction_ExitWorldCapturedThumbnail && action != eAppAction_Idle)
	{
		app.DebugPrintf("Invalid change of App action for pad %d from %d to %d, ignoring\n", iPad, m_eXuiAction[iPad], action);
	}
	else
	{
		app.DebugPrintf("Changing App action for pad %d from %d to %d\n", iPad, m_eXuiAction[iPad], action);
		m_eXuiAction[iPad]=action;
		m_eXuiActionParam[iPad] = param;
	}
}

bool CMinecraftApp::IsAppPaused()
{
#if defined(_XBOX_ONE) || defined(__ORBIS__)
	bool paused = m_bIsAppPaused;
	EnterCriticalSection(&m_saveNotificationCriticalSection);
	if( g_NetworkManager.IsLocalGame() && g_NetworkManager.GetPlayerCount() == 1 )
	{
		paused |= m_saveNotificationDepth > 0;
	}
	LeaveCriticalSection(&m_saveNotificationCriticalSection);
	return paused;
#else
	return m_bIsAppPaused;
#endif
}

void CMinecraftApp::SetAppPaused(bool val)
{
	m_bIsAppPaused = val;
}

void CMinecraftApp::HandleButtonPresses()
{
	for(int i=0;i<4;i++)
	{
		HandleButtonPresses(i);
	}
}

void CMinecraftApp::HandleButtonPresses(int iPad)
{

	// 		// test an update of the profile data
	// 		void *pData=ProfileManager.GetGameDefinedProfileData(iPad);
	//
	// 		unsigned char *pchData= (unsigned char *)pData;
	// 		int iCount=0;
	// 		for(int i=0;i<GAME_DEFINED_PROFILE_DATA_BYTES;i++)
	// 		{
	// 			pchData[i]=0xBC;
	// 			//if(iCount==255) iCount = 0;
	// 		}
	//  		ProfileManager.WriteToProfile(iPad,true);
}

bool CMinecraftApp::LoadInventoryMenu(int iPad,shared_ptr<LocalPlayer> player,bool bNavigateBack)
{
	bool success = true;

	InventoryScreenInput* initData = new InventoryScreenInput();
	initData->player = player;
	initData->bNavigateBack=bNavigateBack;
	initData->iPad = iPad;

	if(app.GetLocalPlayerCount()>1)
	{
		initData->bSplitscreen=true;
		success = ui.NavigateToScene(iPad,eUIScene_InventoryMenu,initData);
	}
	else
	{
		initData->bSplitscreen=false;
		success = ui.NavigateToScene(iPad,eUIScene_InventoryMenu,initData);
	}

	return success;
}

bool CMinecraftApp::LoadCreativeMenu(int iPad,shared_ptr<LocalPlayer> player,bool bNavigateBack)
{
	bool success = true;

	InventoryScreenInput* initData = new InventoryScreenInput();
	initData->player = player;
	initData->bNavigateBack=bNavigateBack;
	initData->iPad = iPad;

	if(app.GetLocalPlayerCount()>1)
	{
		initData->bSplitscreen=true;
		success = ui.NavigateToScene(iPad,eUIScene_CreativeMenu,initData);
	}
	else
	{
		initData->bSplitscreen=false;
		success = ui.NavigateToScene(iPad,eUIScene_CreativeMenu,initData);
	}

	return success;
}

bool CMinecraftApp::LoadCrafting2x2Menu(int iPad,shared_ptr<LocalPlayer> player)
{
	bool success = true;

	CraftingPanelScreenInput* initData = new CraftingPanelScreenInput();
	initData->player = player;
	initData->iContainerType=RECIPE_TYPE_2x2;
	initData->iPad = iPad;
	initData->x = 0;
	initData->y = 0;
	initData->z = 0;

	if(app.GetLocalPlayerCount()>1)
	{
		initData->bSplitscreen=true;
		success = ui.NavigateToScene(iPad,eUIScene_Crafting2x2Menu, initData);
	}
	else
	{
		initData->bSplitscreen=false;
		success = ui.NavigateToScene(iPad,eUIScene_Crafting2x2Menu, initData);
	}

	return success;
}

bool CMinecraftApp::LoadCrafting3x3Menu(int iPad,shared_ptr<LocalPlayer> player, int x, int y, int z)
{
	bool success = true;

	CraftingPanelScreenInput* initData = new CraftingPanelScreenInput();
	initData->player = player;
	initData->iContainerType=RECIPE_TYPE_3x3;
	initData->iPad = iPad;
	initData->x = x;
	initData->y = y;
	initData->z = z;

	if(app.GetLocalPlayerCount()>1)
	{
		initData->bSplitscreen=true;
		success = ui.NavigateToScene(iPad,eUIScene_Crafting3x3Menu, initData);
	}
	else
	{
		initData->bSplitscreen=false;
		success = ui.NavigateToScene(iPad,eUIScene_Crafting3x3Menu, initData);
	}

	return success;
}

bool CMinecraftApp::LoadFireworksMenu(int iPad,shared_ptr<LocalPlayer> player, int x, int y, int z)
{
	bool success = true;

	FireworksScreenInput* initData = new FireworksScreenInput();
	initData->player = player;
	initData->iPad = iPad;
	initData->x = x;
	initData->y = y;
	initData->z = z;

	if(app.GetLocalPlayerCount()>1)
	{
		initData->bSplitscreen=true;
		success = ui.NavigateToScene(iPad,eUIScene_FireworksMenu, initData);
	}
	else
	{
		initData->bSplitscreen=false;
		success = ui.NavigateToScene(iPad,eUIScene_FireworksMenu, initData);
	}

	return success;
}

bool CMinecraftApp::LoadEnchantingMenu(int iPad,shared_ptr<Inventory> inventory, int x, int y, int z, Level *level, const wstring &name)
{
	bool success = true;

	EnchantingScreenInput* initData = new EnchantingScreenInput();
	initData->inventory = inventory;
	initData->level = level;
	initData->x = x;
	initData->y = y;
	initData->z = z;
	initData->iPad = iPad;
	initData->name = name;

	if(app.GetLocalPlayerCount()>1)
	{
		initData->bSplitscreen=true;
		success = ui.NavigateToScene(iPad,eUIScene_EnchantingMenu, initData);
	}
	else
	{
		initData->bSplitscreen=false;
		success = ui.NavigateToScene(iPad,eUIScene_EnchantingMenu, initData);
	}

	return success;
}

bool CMinecraftApp::LoadFurnaceMenu(int iPad,shared_ptr<Inventory> inventory, shared_ptr<FurnaceTileEntity> furnace)
{
	bool success = true;

	FurnaceScreenInput* initData = new FurnaceScreenInput();

	initData->furnace = furnace;
	initData->inventory = inventory;
	initData->iPad = iPad;

	// Load the scene.
	if(app.GetLocalPlayerCount()>1)
	{
		initData->bSplitscreen=true;
		success = ui.NavigateToScene(iPad,eUIScene_FurnaceMenu, initData);
	}
	else
	{
		initData->bSplitscreen=false;
		success = ui.NavigateToScene(iPad,eUIScene_FurnaceMenu, initData);
	}

	return success;
}

bool CMinecraftApp::LoadBrewingStandMenu(int iPad,shared_ptr<Inventory> inventory, shared_ptr<BrewingStandTileEntity> brewingStand)
{
	bool success = true;

	BrewingScreenInput* initData = new BrewingScreenInput();

	initData->brewingStand = brewingStand;
	initData->inventory = inventory;
	initData->iPad = iPad;

	// Load the scene.
	if(app.GetLocalPlayerCount()>1)
	{
		initData->bSplitscreen=true;
		success = ui.NavigateToScene(iPad,eUIScene_BrewingStandMenu, initData);
	}
	else
	{
		initData->bSplitscreen=false;
		success = ui.NavigateToScene(iPad,eUIScene_BrewingStandMenu, initData);
	}

	return success;
}


bool CMinecraftApp::LoadContainerMenu(int iPad,shared_ptr<Container> inventory, shared_ptr<Container> container)
{
	bool success = true;

	ContainerScreenInput* initData = new ContainerScreenInput();

	initData->inventory = inventory;
	initData->container = container;
	initData->iPad		= iPad;

	// Load the scene.
	if(app.GetLocalPlayerCount()>1)
	{
		initData->bSplitscreen=true;

		bool bLargeChest = (initData->container->getContainerSize() > 3*9)?true:false;
		if(bLargeChest)
		{
			success = ui.NavigateToScene(iPad,eUIScene_LargeContainerMenu,initData);
		}
		else
		{
			success = ui.NavigateToScene(iPad,eUIScene_ContainerMenu,initData);
		}
	}
	else
	{
		initData->bSplitscreen=false;
		success = ui.NavigateToScene(iPad,eUIScene_ContainerMenu,initData);
	}

	return success;
}

bool CMinecraftApp::LoadTrapMenu(int iPad,shared_ptr<Container> inventory, shared_ptr<DispenserTileEntity> trap)
{
	bool success = true;

	TrapScreenInput* initData = new TrapScreenInput();

	initData->inventory = inventory;
	initData->trap = trap;
	initData->iPad = iPad;

	// Load the scene.
	if(app.GetLocalPlayerCount()>1)
	{
		initData->bSplitscreen=true;
		success = ui.NavigateToScene(iPad,eUIScene_DispenserMenu, initData);
	}
	else
	{
		initData->bSplitscreen=false;
		success = ui.NavigateToScene(iPad,eUIScene_DispenserMenu, initData);
	}

	return success;
}

bool CMinecraftApp::LoadSignEntryMenu(int iPad,shared_ptr<SignTileEntity> sign)
{
	bool success = true;

	SignEntryScreenInput* initData = new SignEntryScreenInput();

	initData->sign = sign;
	initData->iPad = iPad;

	success = ui.NavigateToScene(iPad,eUIScene_SignEntryMenu, initData);

	delete initData;

	return success;
}

bool CMinecraftApp::LoadRepairingMenu(int iPad,shared_ptr<Inventory> inventory, Level *level, int x, int y, int z)
{
	bool success = true;

	AnvilScreenInput *initData = new AnvilScreenInput();
	initData->inventory = inventory;
	initData->level = level;
	initData->x = x;
	initData->y = y;
	initData->z = z;
	initData->iPad = iPad;
	if(app.GetLocalPlayerCount()>1) initData->bSplitscreen=true;
	else initData->bSplitscreen=false;

	success = ui.NavigateToScene(iPad,eUIScene_AnvilMenu, initData);

	return success;
}

bool CMinecraftApp::LoadTradingMenu(int iPad, shared_ptr<Inventory> inventory, shared_ptr<Merchant> trader, Level *level, const wstring &name)
{
	bool success = true;

	TradingScreenInput *initData = new TradingScreenInput();
	initData->inventory = inventory;
	initData->trader = trader;
	initData->level = level;
	initData->iPad = iPad;
	if(app.GetLocalPlayerCount()>1) initData->bSplitscreen=true;
	else initData->bSplitscreen=false;

	success = ui.NavigateToScene(iPad,eUIScene_TradingMenu, initData);

	return success;
}

bool CMinecraftApp::LoadHopperMenu(int iPad ,shared_ptr<Inventory> inventory, shared_ptr<HopperTileEntity> hopper)
{
	bool success = true;

	HopperScreenInput *initData = new HopperScreenInput();
	initData->inventory = inventory;
	initData->hopper = hopper;
	initData->iPad = iPad;
	if(app.GetLocalPlayerCount()>1) initData->bSplitscreen=true;
	else initData->bSplitscreen=false;

	success = ui.NavigateToScene(iPad,eUIScene_HopperMenu, initData);

	return success;
}

bool CMinecraftApp::LoadHopperMenu(int iPad ,shared_ptr<Inventory> inventory, shared_ptr<MinecartHopper> hopper)
{
	bool success = true;

	HopperScreenInput *initData = new HopperScreenInput();
	initData->inventory = inventory;
	initData->hopper = dynamic_pointer_cast<Container>(hopper);
	initData->iPad = iPad;
	if(app.GetLocalPlayerCount()>1) initData->bSplitscreen=true;
	else initData->bSplitscreen=false;

	success = ui.NavigateToScene(iPad,eUIScene_HopperMenu, initData);

	return success;
}


bool CMinecraftApp::LoadHorseMenu(int iPad ,shared_ptr<Inventory> inventory, shared_ptr<Container> container, shared_ptr<EntityHorse> horse)
{
	bool success = true;

	HorseScreenInput *initData = new HorseScreenInput();
	initData->inventory = inventory;
	initData->container = container;
	initData->horse = horse;
	initData->iPad = iPad;
	if(app.GetLocalPlayerCount()>1) initData->bSplitscreen=true;
	else initData->bSplitscreen=false;

	success = ui.NavigateToScene(iPad,eUIScene_HorseMenu, initData);

	return success;
}

bool CMinecraftApp::LoadBeaconMenu(int iPad ,shared_ptr<Inventory> inventory, shared_ptr<BeaconTileEntity> beacon)
{
	bool success = true;

	BeaconScreenInput *initData = new BeaconScreenInput();
	initData->inventory = inventory;
	initData->beacon = beacon;
	initData->iPad = iPad;
	if(app.GetLocalPlayerCount()>1) initData->bSplitscreen=true;
	else initData->bSplitscreen=false;

	success = ui.NavigateToScene(iPad,eUIScene_BeaconMenu, initData);

	return success;
}

//////////////////////////////////////////////
// GAME SETTINGS
//////////////////////////////////////////////

#ifdef _WINDOWS64
static void Win64_GetSettingsPath(char *outPath, DWORD size)
{
    GetModuleFileNameA(nullptr, outPath, size);
    char *lastSlash = strrchr(outPath, '\\');
    if (lastSlash) *(lastSlash + 1) = '\0';
    strncat_s(outPath, size, "settings.dat", _TRUNCATE);
}
static void Win64_SaveSettings(GAME_SETTINGS *gs)
{
    if (!gs) return;
    char filePath[MAX_PATH] = {};
    Win64_GetSettingsPath(filePath, MAX_PATH);
    FILE *f = nullptr;
    if (fopen_s(&f, filePath, "wb") == 0 && f)
    {
        fwrite(gs, sizeof(GAME_SETTINGS), 1, f);
        fclose(f);
    }
}
static void Win64_LoadSettings(GAME_SETTINGS *gs)
{
    if (!gs) return;
    char filePath[MAX_PATH] = {};
    Win64_GetSettingsPath(filePath, MAX_PATH);
    FILE *f = nullptr;
    if (fopen_s(&f, filePath, "rb") == 0 && f)
    {
        GAME_SETTINGS temp = {};
        if (fread(&temp, sizeof(GAME_SETTINGS), 1, f) == 1)
            memcpy(gs, &temp, sizeof(GAME_SETTINGS));
        fclose(f);
    }
}
#endif

void CMinecraftApp::InitGameSettings()
{
	for(int i=0;i<XUSER_MAX_COUNT;i++)
	{
#if (defined __PS3__ || defined __ORBIS__ || defined _DURANGO  || defined __PSVITA__)
		GameSettingsA[i]=(GAME_SETTINGS *)StorageManager.GetGameDefinedProfileData(i);
#else
		GameSettingsA[i]=static_cast<GAME_SETTINGS *>(ProfileManager.GetGameDefinedProfileData(i));
#endif
		// clear the flag to say the settings have changed
		GameSettingsA[i]->bSettingsChanged=false;

		//SetDefaultGameSettings(i); - done on a callback from the profile manager

		// 4J-PB - adding in for Windows & PS3 to set the defaults for the joypad
#if defined _WINDOWS64// || defined __PSVITA__
		C_4JProfile::PROFILESETTINGS *pProfileSettings=ProfileManager.GetDashboardProfileSettings(i);
		// clear this for now - it will come from reading the system values
		memset(pProfileSettings,0,sizeof(C_4JProfile::PROFILESETTINGS));
		SetDefaultOptions(pProfileSettings,i);
		Win64_LoadSettings(GameSettingsA[i]);
		ApplyGameSettingsChanged(i);
#elif defined __PS3__ || defined __ORBIS__ || defined _DURANGO  || defined __PSVITA__
		C4JStorage::PROFILESETTINGS *pProfileSettings=StorageManager.GetDashboardProfileSettings(i);
		// 4J-PB - don't cause an options write to happen here
		SetDefaultOptions(pProfileSettings,i,false);

#endif
	}
}

#if (defined __PS3__ || defined __ORBIS__ || defined _DURANGO  || defined __PSVITA__)
int CMinecraftApp::SetDefaultOptions(C4JStorage::PROFILESETTINGS *pSettings,const int iPad,bool bWriteProfile)
#else
int CMinecraftApp::SetDefaultOptions(C_4JProfile::PROFILESETTINGS *pSettings,const int iPad)
#endif
{
	SetGameSettings(iPad,eGameSetting_MusicVolume,DEFAULT_VOLUME_LEVEL);
	SetGameSettings(iPad,eGameSetting_SoundFXVolume,DEFAULT_VOLUME_LEVEL);
	SetGameSettings(iPad,eGameSetting_RenderDistance,16);
	SetGameSettings(iPad,eGameSetting_Gamma,50);
	SetGameSettings(iPad,eGameSetting_FOV,0);

	// 4J-PB - Don't reset the difficult level if we're in-game
	if(Minecraft::GetInstance()->level==nullptr)
	{
		app.DebugPrintf("SetDefaultOptions - Difficulty = 1\n");
		SetGameSettings(iPad,eGameSetting_Difficulty,1);
	}
	SetGameSettings(iPad,eGameSetting_Sensitivity_InGame,100);
	SetGameSettings(iPad,eGameSetting_ViewBob,1);
	SetGameSettings(iPad,eGameSetting_ControlScheme,0);
	SetGameSettings(iPad,eGameSetting_ControlInvertLook,(pSettings->iYAxisInversion!=0)?1:0);
	SetGameSettings(iPad,eGameSetting_ControlSouthPaw,pSettings->bSwapSticks?1:0);
	SetGameSettings(iPad,eGameSetting_SplitScreenVertical,0);
	SetGameSettings(iPad,eGameSetting_GamertagsVisible,1);

	// Interim TU 1.6.6
	SetGameSettings(iPad,eGameSetting_Sensitivity_InMenu,100);
	SetGameSettings(iPad,eGameSetting_DisplaySplitscreenGamertags,1);
	SetGameSettings(iPad,eGameSetting_Hints,1);
	SetGameSettings(iPad,eGameSetting_Autosave,2);
	SetGameSettings(iPad,eGameSetting_Tooltips,1);
	SetGameSettings(iPad,eGameSetting_InterfaceOpacity,80);

	// TU 5
	SetGameSettings(iPad,eGameSetting_Clouds,1);
	SetGameSettings(iPad,eGameSetting_Online,1);
	SetGameSettings(iPad,eGameSetting_InviteOnly,0);
	SetGameSettings(iPad,eGameSetting_FriendsOfFriends,1);

	// default the update changes message to zero
	// 4J-PB - We'll only display the message if the profile is pre-TU5
	//SetGameSettings(iPad,eGameSetting_DisplayUpdateMessage,0);

	// TU 6
	SetGameSettings(iPad,eGameSetting_BedrockFog,0);
	SetGameSettings(iPad,eGameSetting_DisplayHUD,1);
	SetGameSettings(iPad,eGameSetting_DisplayHand,1);

	// TU 7
	SetGameSettings(iPad,eGameSetting_CustomSkinAnim,1);

	// TU 9
	SetGameSettings(iPad,eGameSetting_DeathMessages,1);
	SetGameSettings(iPad,eGameSetting_UISize,1);
	SetGameSettings(iPad,eGameSetting_UISizeSplitscreen,2);
	SetGameSettings(iPad,eGameSetting_AnimatedCharacter,1);

	// TU 12
	GameSettingsA[iPad]->ucCurrentFavoriteSkinPos=0;
	for(int i=0;i<MAX_FAVORITE_SKINS;i++)
	{
		GameSettingsA[iPad]->uiFavoriteSkinA[i]=0xFFFFFFFF;
	}

	// TU 13
	GameSettingsA[iPad]->uiMashUpPackWorldsDisplay=0xFFFFFFFF;

	// 1.6.4
	app.SetGameHostOption(eGameHostOption_MobGriefing, 1);
	app.SetGameHostOption(eGameHostOption_KeepInventory, 0);
	app.SetGameHostOption(eGameHostOption_DoMobSpawning, 1 );
	app.SetGameHostOption(eGameHostOption_DoMobLoot, 1 );
	app.SetGameHostOption(eGameHostOption_DoTileDrops, 1 );
	app.SetGameHostOption(eGameHostOption_NaturalRegeneration, 1 );
	app.SetGameHostOption(eGameHostOption_DoDaylightCycle, 1 );

	// 4J-PB - leave these in, or remove from everywhere they are referenced!
	// Although probably best to leave in unless we split the profile settings into platform specific classes - having different meaning per platform for the same bitmask could get confusing
	//#ifdef __PS3__
	// PS3DEC13
	SetGameSettings(iPad,eGameSetting_PS3_EULA_Read,0); // EULA not read

	// PS3 1.05 - added Greek

	// 4J-JEV: We cannot change these in-game, as they could affect localised strings and font.
	// XB1: Fix for #172947 - Content: Gameplay: While playing in language different form system default one and resetting options to their defaults in active gameplay causes in-game language to change and HUD to disappear
	if (!app.GetGameStarted())
	{
		GameSettingsA[iPad]->ucLanguage = MINECRAFT_LANGUAGE_DEFAULT; // use the system language
		GameSettingsA[iPad]->ucLocale = MINECRAFT_LANGUAGE_DEFAULT; // use the system locale
	}

	//#endif

#if (defined __PS3__ || defined __ORBIS__ || defined _DURANGO  || defined __PSVITA__)
	GameSettingsA[iPad]->bSettingsChanged=bWriteProfile;
#endif

	return 0;
}

#if ( defined  __PS3__  || defined __ORBIS__ || defined _DURANGO  || defined __PSVITA__)
int CMinecraftApp::DefaultOptionsCallback(LPVOID pParam,C4JStorage::PROFILESETTINGS *pSettings, const int iPad)
#else
int CMinecraftApp::DefaultOptionsCallback(LPVOID pParam,C_4JProfile::PROFILESETTINGS *pSettings, const int iPad)
#endif
{
	CMinecraftApp *pApp=static_cast<CMinecraftApp *>(pParam);

	// flag the default options to be set

	pApp->DebugPrintf("Setting default options for player %d", iPad);
	pApp->SetAction(iPad,eAppAction_SetDefaultOptions, (LPVOID)pSettings);
	//pApp->SetDefaultOptions(pSettings,iPad);

	// if the profile data has been changed, then force a profile write
	// It seems we're allowed to break the 5 minute rule if it's the result of a user action
	//pApp->CheckGameSettingsChanged();

	return 0;
}

#if ( defined  __PS3__  || defined __ORBIS__ || defined _DURANGO  || defined __PSVITA__)

wstring CMinecraftApp::toStringOptionsStatus(const C4JStorage::eOptionsCallback &eStatus)
{
#ifndef _CONTENT_PACKAGE
	switch(eStatus)
	{
		case C4JStorage::eOptions_Callback_Idle:						return L"Idle";
		case C4JStorage::eOptions_Callback_Write:						return L"Write";
		case C4JStorage::eOptions_Callback_Write_Fail_NoSpace:			return L"Write_Fail_NoSpace";
		case C4JStorage::eOptions_Callback_Write_Fail:					return L"Write_Fail";
		case C4JStorage::eOptions_Callback_Read:						return L"Read";
		case C4JStorage::eOptions_Callback_Read_Fail:					return L"Read_Fail";
		case C4JStorage::eOptions_Callback_Read_FileNotFound:			return L"Read_FileNotFound";
		case C4JStorage::eOptions_Callback_Read_Corrupt:				return L"Read_Corrupt";
		case C4JStorage::eOptions_Callback_Read_CorruptDeletePending:	return L"Read_CorruptDeletePending";
		case C4JStorage::eOptions_Callback_Read_CorruptDeleted:			return L"Read_CorruptDeleted";
		default:														return L"[UNRECOGNISED_OPTIONS_STATUS]";
	}
#else
	return L"";
#endif
}

#ifdef __ORBIS__
int CMinecraftApp::OptionsDataCallback(LPVOID pParam,int iPad,unsigned short usVersion,C4JStorage::eOptionsCallback eStatus,int iBlocksRequired)
{
	CMinecraftApp *pApp=(CMinecraftApp *)pParam;
	pApp->m_eOptionsStatusA[iPad]=eStatus;
	pApp->m_eOptionsBlocksRequiredA[iPad]=iBlocksRequired;
	return 0;
}

int CMinecraftApp::GetOptionsBlocksRequired(int iPad)
{
	return m_eOptionsBlocksRequiredA[iPad];
}

#else
int CMinecraftApp::OptionsDataCallback(LPVOID pParam,int iPad,unsigned short usVersion,C4JStorage::eOptionsCallback eStatus)
{
	CMinecraftApp *pApp=(CMinecraftApp *)pParam;

#ifndef _CONTENT_PACKAGE
		pApp->DebugPrintf("[OptionsDataCallback] Pad_%i: new status == %ls(%i).\n", iPad, pApp->toStringOptionsStatus(eStatus).c_str(), (int) eStatus);
#endif

	pApp->m_eOptionsStatusA[iPad] = eStatus;

	return 0;
}
#endif

C4JStorage::eOptionsCallback CMinecraftApp::GetOptionsCallbackStatus(int iPad)
{
	return m_eOptionsStatusA[iPad];
}

void CMinecraftApp::SetOptionsCallbackStatus(int iPad, C4JStorage::eOptionsCallback eStatus)
{
	m_eOptionsStatusA[iPad]=eStatus;
}
#endif

int CMinecraftApp::OldProfileVersionCallback(LPVOID pParam,unsigned char *pucData, const unsigned short usVersion, const int iPad)
{
	// check what needs to be done with this version to update to the current one

	switch(usVersion)
	{
#ifdef _XBOX
	case PROFILE_VERSION_1:
	case PROFILE_VERSION_2:
		// need to fill in values for the new profile data. No need to save the profile - that'll happen if they get changed, or if the auto save for the profile kicks in
		{
			GAME_SETTINGS *pGameSettings=(GAME_SETTINGS *)pucData;
			pGameSettings->ucMenuSensitivity=100; //eGameSetting_Sensitivity_InMenu
			pGameSettings->ucInterfaceOpacity=80; //eGameSetting_Sensitivity_InMenu
			pGameSettings->usBitmaskValues|=0x0200; //eGameSetting_DisplaySplitscreenGamertags - on
			pGameSettings->usBitmaskValues|=0x0400; //eGameSetting_Hints - on
			pGameSettings->usBitmaskValues|=0x1000; //eGameSetting_Autosave - 2
			pGameSettings->usBitmaskValues|=0x8000; //eGameSetting_Tooltips - on

			// 4J-PB - Let's also award all the achievements they have again because of the profile bug that seemed to stop the awards of some
			// Changing this to check the system achievements at sign-in and award any that the game says we have and the system says we haven't
			//ProfileManager.ReAwardAchievements(iPad);

			pGameSettings->uiBitmaskValues=0L; // reset
			pGameSettings->uiBitmaskValues|=GAMESETTING_CLOUDS;					//eGameSetting_Clouds - on
			pGameSettings->uiBitmaskValues|=GAMESETTING_ONLINE;					//eGameSetting_GameSetting_Online - on
			//eGameSetting_GameSetting_Invite - off
			pGameSettings->uiBitmaskValues|=GAMESETTING_FRIENDSOFFRIENDS;		//eGameSetting_GameSetting_FriendsOfFriends - on
			pGameSettings->uiBitmaskValues|=GAMESETTING_DISPLAYUPDATEMSG;		//eGameSetting_DisplayUpdateMessage (counter)
			// TU6
			pGameSettings->uiBitmaskValues&=~GAMESETTING_BEDROCKFOG;			//eGameSetting_BedrockFog - off
			pGameSettings->uiBitmaskValues|=GAMESETTING_DISPLAYHUD;				//eGameSetting_DisplayHUD - on
			pGameSettings->uiBitmaskValues|=GAMESETTING_DISPLAYHAND;			//eGameSetting_DisplayHand - on
			// TU7
			pGameSettings->uiBitmaskValues|=GAMESETTING_CUSTOMSKINANIM;			//eGameSetting_CustomSkinAnim - on
			// TU9
			pGameSettings->uiBitmaskValues|=GAMESETTING_DEATHMESSAGES;			//eGameSetting_DeathMessages - on
			pGameSettings->uiBitmaskValues|=(GAMESETTING_UISIZE&0x00000800);				// uisize 2
			pGameSettings->uiBitmaskValues|=(GAMESETTING_UISIZE_SPLITSCREEN&0x00004000);	// splitscreen ui size 3
			pGameSettings->uiBitmaskValues|=GAMESETTING_ANIMATEDCHARACTER;		//eGameSetting_AnimatedCharacter - on
			// TU12
			// favorite skins added, but only set in TU12 - set to FFs
			for(int i=0;i<MAX_FAVORITE_SKINS;i++)
			{
				pGameSettings->uiFavoriteSkinA[i]=0xFFFFFFFF;
			}
			pGameSettings->ucCurrentFavoriteSkinPos=0;
			// Added a bitmask in TU13 to enable/disable display of the Mash-up pack worlds in the saves list
			pGameSettings->uiMashUpPackWorldsDisplay = 0xFFFFFFFF;

			// PS3 1.05 - added Greek
			pGameSettings->ucLanguage = MINECRAFT_LANGUAGE_DEFAULT; // use the system language
		}
		break;
	case PROFILE_VERSION_3:

		{
			GAME_SETTINGS *pGameSettings=(GAME_SETTINGS *)pucData;
			pGameSettings->uiBitmaskValues=0L; // reset
			pGameSettings->uiBitmaskValues|=GAMESETTING_CLOUDS;		//eGameSetting_Clouds - on
			pGameSettings->uiBitmaskValues|=GAMESETTING_ONLINE;		//eGameSetting_GameSetting_Online - on
			//eGameSetting_GameSetting_Invite - off
			pGameSettings->uiBitmaskValues|=GAMESETTING_FRIENDSOFFRIENDS;		//eGameSetting_GameSetting_FriendsOfFriends - on
			pGameSettings->uiBitmaskValues|=GAMESETTING_DISPLAYUPDATEMSG;		//eGameSetting_DisplayUpdateMessage (counter)
			// TU6
			pGameSettings->uiBitmaskValues&=~GAMESETTING_BEDROCKFOG;	//eGameSetting_BedrockFog - off
			pGameSettings->uiBitmaskValues|=GAMESETTING_DISPLAYHUD;		//eGameSetting_DisplayHUD - on
			pGameSettings->uiBitmaskValues|=GAMESETTING_DISPLAYHAND;		//eGameSetting_DisplayHand - on
			// TU7
			pGameSettings->uiBitmaskValues|=GAMESETTING_CUSTOMSKINANIM;		//eGameSetting_CustomSkinAnim - on
			// TU9
			pGameSettings->uiBitmaskValues|=GAMESETTING_DEATHMESSAGES;		//eGameSetting_DeathMessages - on
			pGameSettings->uiBitmaskValues|=(GAMESETTING_UISIZE&0x00000800);				// uisize 2
			pGameSettings->uiBitmaskValues|=(GAMESETTING_UISIZE_SPLITSCREEN&0x00004000);	// splitscreen ui size 3
			pGameSettings->uiBitmaskValues|=GAMESETTING_ANIMATEDCHARACTER;		//eGameSetting_AnimatedCharacter - on
			// TU12
			// favorite skins added, but only set in TU12 - set to FFs
			for(int i=0;i<MAX_FAVORITE_SKINS;i++)
			{
				pGameSettings->uiFavoriteSkinA[i]=0xFFFFFFFF;
			}
			pGameSettings->ucCurrentFavoriteSkinPos=0;
			// Added a bitmask in TU13 to enable/disable display of the Mash-up pack worlds in the saves list
			pGameSettings->uiMashUpPackWorldsDisplay = 0xFFFFFFFF;

			// PS3 1.05 - added Greek
			pGameSettings->ucLanguage = MINECRAFT_LANGUAGE_DEFAULT; // use the system language

		}
		break;
	case PROFILE_VERSION_4:
		{
			GAME_SETTINGS *pGameSettings=(GAME_SETTINGS *)pucData;

			pGameSettings->uiBitmaskValues&=~GAMESETTING_BEDROCKFOG;	//eGameSetting_BedrockFog - off
			pGameSettings->uiBitmaskValues|=GAMESETTING_DISPLAYHUD;		//eGameSetting_DisplayHUD - on
			pGameSettings->uiBitmaskValues|=GAMESETTING_DISPLAYHAND;		//eGameSetting_DisplayHand - on
			// TU7
			pGameSettings->uiBitmaskValues|=GAMESETTING_CUSTOMSKINANIM;		//eGameSetting_CustomSkinAnim - on
			// TU9
			pGameSettings->uiBitmaskValues|=GAMESETTING_DEATHMESSAGES;		//eGameSetting_DeathMessages - on
			pGameSettings->uiBitmaskValues|=(GAMESETTING_UISIZE&0x00000800);				// uisize 2
			pGameSettings->uiBitmaskValues|=(GAMESETTING_UISIZE_SPLITSCREEN&0x00004000);	// splitscreen ui size 3
			pGameSettings->uiBitmaskValues|=GAMESETTING_ANIMATEDCHARACTER;		//eGameSetting_AnimatedCharacter - on

			// Set the online flag to on, so it's not saved if a game starts offline when the user didn't change it to be offline (xbox disconnected from LIVE)
			pGameSettings->uiBitmaskValues|=GAMESETTING_ONLINE;					//eGameSetting_GameSetting_Online - on
			// TU12
			// favorite skins added, but only set in TU12 - set to FFs
			for(int i=0;i<MAX_FAVORITE_SKINS;i++)
			{
				pGameSettings->uiFavoriteSkinA[i]=0xFFFFFFFF;
			}
			pGameSettings->ucCurrentFavoriteSkinPos=0;
			// Added a bitmask in TU13 to enable/disable display of the Mash-up pack worlds in the saves list
			pGameSettings->uiMashUpPackWorldsDisplay = 0xFFFFFFFF;

			// PS3 1.05 - added Greek
			pGameSettings->ucLanguage = MINECRAFT_LANGUAGE_DEFAULT; // use the system language

		}

		break;
	case PROFILE_VERSION_5:
		{
			GAME_SETTINGS *pGameSettings=(GAME_SETTINGS *)pucData;

			// reset the display new message counter
			pGameSettings->uiBitmaskValues|=GAMESETTING_DISPLAYUPDATEMSG;		//eGameSetting_DisplayUpdateMessage (counter)
			// TU7
			pGameSettings->uiBitmaskValues|=GAMESETTING_CUSTOMSKINANIM;		//eGameSetting_CustomSkinAnim - on
			// TU9
			pGameSettings->uiBitmaskValues|=GAMESETTING_DEATHMESSAGES;		//eGameSetting_DeathMessages - on
			pGameSettings->uiBitmaskValues|=(GAMESETTING_UISIZE&0x00000800);				// uisize 2
			pGameSettings->uiBitmaskValues|=(GAMESETTING_UISIZE_SPLITSCREEN&0x00004000);	// splitscreen ui size 3
			pGameSettings->uiBitmaskValues|=GAMESETTING_ANIMATEDCHARACTER;		//eGameSetting_AnimatedCharacter - on
			// Set the online flag to on, so it's not saved if a game starts offline when the user didn't change it to be offline (xbox disconnected from LIVE)
			pGameSettings->uiBitmaskValues|=GAMESETTING_ONLINE;					//eGameSetting_GameSetting_Online - on
			// TU12
			// favorite skins added, but only set in TU12 - set to FFs
			for(int i=0;i<MAX_FAVORITE_SKINS;i++)
			{
				pGameSettings->uiFavoriteSkinA[i]=0xFFFFFFFF;
			}
			pGameSettings->ucCurrentFavoriteSkinPos=0;
			// Added a bitmask in TU13 to enable/disable display of the Mash-up pack worlds in the saves list
			pGameSettings->uiMashUpPackWorldsDisplay = 0xFFFFFFFF;

			// PS3 1.05 - added Greek
			pGameSettings->ucLanguage = MINECRAFT_LANGUAGE_DEFAULT; // use the system language


		}

		break;
	case PROFILE_VERSION_6:
		{
			GAME_SETTINGS *pGameSettings=(GAME_SETTINGS *)pucData;

			// Added gui size for splitscreen and fullscreen
			// Added death messages toggle

			// reset the display new message counter
			pGameSettings->uiBitmaskValues|=GAMESETTING_DISPLAYUPDATEMSG;		//eGameSetting_DisplayUpdateMessage (counter)
			// TU9
			pGameSettings->uiBitmaskValues|=GAMESETTING_DEATHMESSAGES;		//eGameSetting_DeathMessages - on
			pGameSettings->uiBitmaskValues|=(GAMESETTING_UISIZE&0x00000800);				// uisize 2
			pGameSettings->uiBitmaskValues|=(GAMESETTING_UISIZE_SPLITSCREEN&0x00004000);	// splitscreen ui size 3
			pGameSettings->uiBitmaskValues|=GAMESETTING_ANIMATEDCHARACTER;		//eGameSetting_AnimatedCharacter - on
			// Set the online flag to on, so it's not saved if a game starts offline when the user didn't change it to be offline (xbox disconnected from LIVE)
			pGameSettings->uiBitmaskValues|=GAMESETTING_ONLINE;					//eGameSetting_GameSetting_Online - on
			// TU12
			// favorite skins added, but only set in TU12 - set to FFs
			for(int i=0;i<MAX_FAVORITE_SKINS;i++)
			{
				pGameSettings->uiFavoriteSkinA[i]=0xFFFFFFFF;
			}
			pGameSettings->ucCurrentFavoriteSkinPos=0;
			// Added a bitmask in TU13 to enable/disable display of the Mash-up pack worlds in the saves list
			pGameSettings->uiMashUpPackWorldsDisplay = 0xFFFFFFFF;

			// PS3 1.05 - added Greek
			pGameSettings->ucLanguage = MINECRAFT_LANGUAGE_DEFAULT; // use the system language


		}

		break;

	case PROFILE_VERSION_7:
		{
			GAME_SETTINGS *pGameSettings=(GAME_SETTINGS *)pucData;
			// reset the display new message counter
			pGameSettings->uiBitmaskValues|=GAMESETTING_DISPLAYUPDATEMSG;		//eGameSetting_DisplayUpdateMessage (counter)

			// TU12
			// favorite skins added, but only set in TU12 - set to FFs
			for(int i=0;i<MAX_FAVORITE_SKINS;i++)
			{
				pGameSettings->uiFavoriteSkinA[i]=0xFFFFFFFF;
			}
			pGameSettings->ucCurrentFavoriteSkinPos=0;
			// Added a bitmask in TU13 to enable/disable display of the Mash-up pack worlds in the saves list
			pGameSettings->uiMashUpPackWorldsDisplay = 0xFFFFFFFF;

			// PS3 1.05 - added Greek
			pGameSettings->ucLanguage = MINECRAFT_LANGUAGE_DEFAULT; // use the system language


		}
		break;
#endif
	case PROFILE_VERSION_8:
		{
			GAME_SETTINGS *pGameSettings=(GAME_SETTINGS *)pucData;
			// reset the display new message counter
			pGameSettings->uiBitmaskValues|=GAMESETTING_DISPLAYUPDATEMSG;		//eGameSetting_DisplayUpdateMessage (counter)

			// Added a bitmask in TU13 to enable/disable display of the Mash-up pack worlds in the saves list
			pGameSettings->uiMashUpPackWorldsDisplay = 0xFFFFFFFF;

			// PS3DEC13
			pGameSettings->uiBitmaskValues&=~GAMESETTING_PS3EULAREAD;		//eGameSetting_PS3_EULA_Read - off

			// PS3 1.05 - added Greek
			pGameSettings->ucLanguage = MINECRAFT_LANGUAGE_DEFAULT; // use the system language

		}
		break;
	case PROFILE_VERSION_9:
		// PS3DEC13
		{
			GAME_SETTINGS *pGameSettings=(GAME_SETTINGS *)pucData;
			pGameSettings->uiBitmaskValues|=GAMESETTING_DISPLAYUPDATEMSG;		//eGameSetting_DisplayUpdateMessage (counter)
			pGameSettings->uiBitmaskValues&=~GAMESETTING_PS3EULAREAD;		//eGameSetting_PS3_EULA_Read - off

			// PS3 1.05 - added Greek
			pGameSettings->ucLanguage = MINECRAFT_LANGUAGE_DEFAULT; // use the system language

		}
		break;
	case PROFILE_VERSION_10:
		{
			GAME_SETTINGS *pGameSettings=(GAME_SETTINGS *)pucData;
			pGameSettings->uiBitmaskValues|=GAMESETTING_DISPLAYUPDATEMSG;		//eGameSetting_DisplayUpdateMessage (counter)
			pGameSettings->ucLanguage = MINECRAFT_LANGUAGE_DEFAULT; // use the system language
		}
		break;
	case PROFILE_VERSION_11:
		{
			GAME_SETTINGS *pGameSettings=(GAME_SETTINGS *)pucData;
			pGameSettings->uiBitmaskValues|=GAMESETTING_DISPLAYUPDATEMSG;		//eGameSetting_DisplayUpdateMessage (counter)
		}
		break;
	case PROFILE_VERSION_12:
		{
			GAME_SETTINGS *pGameSettings=(GAME_SETTINGS *)pucData;
			pGameSettings->uiBitmaskValues|=GAMESETTING_DISPLAYUPDATEMSG;		//eGameSetting_DisplayUpdateMessage (counter)
		}
		break;
	default:
		{
			// This might be from a version during testing of new profile updates
			app.DebugPrintf("Don't know what to do with this profile version!\n");
#ifndef _CONTENT_PACKAGE
			//		__debugbreak();
#endif

			GAME_SETTINGS *pGameSettings=(GAME_SETTINGS *)pucData;
			pGameSettings->ucMenuSensitivity=100; //eGameSetting_Sensitivity_InMenu
			pGameSettings->ucInterfaceOpacity=80; //eGameSetting_Sensitivity_InMenu
			pGameSettings->usBitmaskValues|=0x0200; //eGameSetting_DisplaySplitscreenGamertags - on
			pGameSettings->usBitmaskValues|=0x0400; //eGameSetting_Hints - on
			pGameSettings->usBitmaskValues|=0x1000; //eGameSetting_Autosave - 2
			pGameSettings->usBitmaskValues|=0x8000; //eGameSetting_Tooltips - on

			pGameSettings->uiBitmaskValues=0L; // reset
			pGameSettings->uiBitmaskValues|=GAMESETTING_CLOUDS;					//eGameSetting_Clouds - on
			pGameSettings->uiBitmaskValues|=GAMESETTING_ONLINE;					//eGameSetting_GameSetting_Online - on
			//eGameSetting_GameSetting_Invite - off
			pGameSettings->uiBitmaskValues|=GAMESETTING_FRIENDSOFFRIENDS;		//eGameSetting_GameSetting_FriendsOfFriends - on
			pGameSettings->uiBitmaskValues|=GAMESETTING_DISPLAYUPDATEMSG;		//eGameSetting_DisplayUpdateMessage (counter)
			pGameSettings->uiBitmaskValues&=~GAMESETTING_BEDROCKFOG;			//eGameSetting_BedrockFog - off
			pGameSettings->uiBitmaskValues|=GAMESETTING_DISPLAYHUD;				//eGameSetting_DisplayHUD - on
			pGameSettings->uiBitmaskValues|=GAMESETTING_DISPLAYHAND;			//eGameSetting_DisplayHand - on
			pGameSettings->uiBitmaskValues|=GAMESETTING_CUSTOMSKINANIM;			//eGameSetting_CustomSkinAnim - on
			pGameSettings->uiBitmaskValues|=GAMESETTING_DEATHMESSAGES;			//eGameSetting_DeathMessages - on
			pGameSettings->uiBitmaskValues|=(GAMESETTING_UISIZE&0x00000800);				// uisize 2
			pGameSettings->uiBitmaskValues|=(GAMESETTING_UISIZE_SPLITSCREEN&0x00004000);	// splitscreen ui size 3
			pGameSettings->uiBitmaskValues|=GAMESETTING_ANIMATEDCHARACTER;		//eGameSetting_AnimatedCharacter - on
			// TU12
			// favorite skins added, but only set in TU12 - set to FFs
			for(int i=0;i<MAX_FAVORITE_SKINS;i++)
			{
				pGameSettings->uiFavoriteSkinA[i]=0xFFFFFFFF;
			}
			pGameSettings->ucCurrentFavoriteSkinPos=0;
			// Added a bitmask in TU13 to enable/disable display of the Mash-up pack worlds in the saves list
			pGameSettings->uiMashUpPackWorldsDisplay = 0xFFFFFFFF;

			// PS3DEC13
			pGameSettings->uiBitmaskValues&=~GAMESETTING_PS3EULAREAD;		//eGameSetting_PS3_EULA_Read - off

			// PS3 1.05 - added Greek
			pGameSettings->ucLanguage = MINECRAFT_LANGUAGE_DEFAULT; // use the system language

		}
		break;
	}

	return 0;
}

void CMinecraftApp::ApplyGameSettingsChanged(int iPad)
{
	ActionGameSettings(iPad,eGameSetting_MusicVolume	);
	ActionGameSettings(iPad,eGameSetting_SoundFXVolume	);
	ActionGameSettings(iPad,eGameSetting_RenderDistance	);
	ActionGameSettings(iPad,eGameSetting_Gamma			);
	ActionGameSettings(iPad,eGameSetting_FOV			);
	ActionGameSettings(iPad,eGameSetting_Difficulty		);
	ActionGameSettings(iPad,eGameSetting_Sensitivity_InGame	);
	ActionGameSettings(iPad,eGameSetting_ViewBob		);
	ActionGameSettings(iPad,eGameSetting_ControlScheme	);
	ActionGameSettings(iPad,eGameSetting_ControlInvertLook);
	ActionGameSettings(iPad,eGameSetting_ControlSouthPaw);
	ActionGameSettings(iPad,eGameSetting_SplitScreenVertical);
	ActionGameSettings(iPad,eGameSetting_GamertagsVisible);

	// Interim TU 1.6.6
	ActionGameSettings(iPad,eGameSetting_Sensitivity_InMenu	);
	ActionGameSettings(iPad,eGameSetting_DisplaySplitscreenGamertags);
	ActionGameSettings(iPad,eGameSetting_Hints);
	ActionGameSettings(iPad,eGameSetting_InterfaceOpacity);
	ActionGameSettings(iPad,eGameSetting_Tooltips);

	ActionGameSettings(iPad,eGameSetting_Clouds);
	ActionGameSettings(iPad,eGameSetting_BedrockFog);
	ActionGameSettings(iPad,eGameSetting_DisplayHUD);
	ActionGameSettings(iPad,eGameSetting_DisplayHand);
	ActionGameSettings(iPad,eGameSetting_CustomSkinAnim);
	ActionGameSettings(iPad,eGameSetting_DeathMessages);
	ActionGameSettings(iPad,eGameSetting_UISize);
	ActionGameSettings(iPad,eGameSetting_UISizeSplitscreen);
	ActionGameSettings(iPad,eGameSetting_AnimatedCharacter);

	ActionGameSettings(iPad,eGameSetting_PS3_EULA_Read);

}

void CMinecraftApp::ActionGameSettings(int iPad,eGameSetting eVal)
{
	Minecraft *pMinecraft=Minecraft::GetInstance();
	switch(eVal)
	{
	case eGameSetting_MusicVolume:
		if(iPad==ProfileManager.GetPrimaryPad())
		{
			pMinecraft->options->set(Options::Option::MUSIC,static_cast<float>(GameSettingsA[iPad]->ucMusicVolume)/100.0f);
		}
		break;
    case eGameSetting_SoundFXVolume:
        if (iPad == ProfileManager.GetPrimaryPad())
        {
            pMinecraft->options->set(Options::Option::SOUND, static_cast<float>(GameSettingsA[iPad]->ucSoundFXVolume) / 100.0f);
        }
		break;
	case eGameSetting_RenderDistance:
        if (iPad == ProfileManager.GetPrimaryPad())
        {
            int dist = (GameSettingsA[iPad]->uiBitmaskValues >> 16) & 0xFF;

            int level = UIScene_SettingsGraphicsMenu::DistanceToLevel(dist);
            pMinecraft->options->set(Options::Option::RENDER_DISTANCE, 3 - level);
        };
		break;
	case eGameSetting_Gamma:
		if(iPad==ProfileManager.GetPrimaryPad())
		{
#if defined(_WIN64) || defined(_WINDOWS64)
			pMinecraft->options->set(Options::Option::GAMMA, static_cast<float>(GameSettingsA[iPad]->ucGamma) / 100.0f);
#else
			// ucGamma range is 0-100, UpdateGamma is 0 - 32768
			float fVal=((float)GameSettingsA[iPad]->ucGamma)*327.68f;
			RenderManager.UpdateGamma((unsigned short)fVal);
#endif
		}

		break;
	case eGameSetting_FOV:
		if(iPad==ProfileManager.GetPrimaryPad())
		{
			float fovDeg = 70.0f + (float)GameSettingsA[iPad]->ucFov * 40.0f / 100.0f;
			pMinecraft->gameRenderer->SetFovVal(fovDeg);
			pMinecraft->options->set(Options::Option::FOV, (float)GameSettingsA[iPad]->ucFov / 100.0f);
		}
		break;
	case eGameSetting_Difficulty:		
		if(iPad==ProfileManager.GetPrimaryPad())
		{
			pMinecraft->options->toggle(Options::Option::DIFFICULTY,GameSettingsA[iPad]->usBitmaskValues&0x03);
			app.DebugPrintf("Difficulty toggle to %d\n",GameSettingsA[iPad]->usBitmaskValues&0x03);

			// Update the Game Host setting
			app.SetGameHostOption(eGameHostOption_Difficulty,pMinecraft->options->difficulty);

			// send this to the other players if we are in-game
			bool bInGame=pMinecraft->level!=nullptr;

			// Game Host only (and for now we can't change the diff while in game, so this shouldn't happen)
			if(bInGame && g_NetworkManager.IsHost() && (iPad==ProfileManager.GetPrimaryPad()))
			{
				app.SetXuiServerAction(iPad,eXuiServerAction_ServerSettingChanged_Difficulty);
			}
		}
		else
		{
			app.DebugPrintf("NOT ACTIONING DIFFICULTY - Primary pad is %d, This pad is %d\n",ProfileManager.GetPrimaryPad(),iPad);
		}

		break;
	case eGameSetting_Sensitivity_InGame:
		// 4J-PB - we don't use the options value
		// tell the input that we've changed the sensitivity - range of the slider is 0 to 200, default is 100
		pMinecraft->options->set(Options::Option::SENSITIVITY,static_cast<float>(GameSettingsA[iPad]->ucSensitivity)/100.0f);
		//InputManager.SetJoypadSensitivity(iPad,((float)GameSettingsA[iPad]->ucSensitivity)/100.0f);

		break;
	case eGameSetting_ViewBob:
		// 4J-PB - not handled here any more - it's read from the gamesettings per player
		//pMinecraft->options->toggle(Options::Option::VIEW_BOBBING,GameSettingsA[iPad]->usBitmaskValues&0x04);
		break;
	case eGameSetting_ControlScheme:
		InputManager.SetJoypadMapVal(iPad,(GameSettingsA[iPad]->usBitmaskValues&0x30)>>4);
		break;

	case eGameSetting_ControlInvertLook:
		// Nothing specific to do for this setting.
		break;

	case eGameSetting_ControlSouthPaw:
		// What is the setting?
		if ( GameSettingsA[iPad]->usBitmaskValues & 0x80 )
		{
			// Southpaw.
			InputManager.SetJoypadStickAxisMap( iPad, AXIS_MAP_LX, AXIS_MAP_RX );
			InputManager.SetJoypadStickAxisMap( iPad, AXIS_MAP_LY, AXIS_MAP_RY );
			InputManager.SetJoypadStickAxisMap( iPad, AXIS_MAP_RX, AXIS_MAP_LX );
			InputManager.SetJoypadStickAxisMap( iPad, AXIS_MAP_RY, AXIS_MAP_LY );
			InputManager.SetJoypadStickTriggerMap( iPad, TRIGGER_MAP_0, TRIGGER_MAP_1 );
			InputManager.SetJoypadStickTriggerMap( iPad, TRIGGER_MAP_1, TRIGGER_MAP_0 );
		}
		else
		{
			// Right handed.
			InputManager.SetJoypadStickAxisMap( iPad, AXIS_MAP_LX, AXIS_MAP_LX );
			InputManager.SetJoypadStickAxisMap( iPad, AXIS_MAP_LY, AXIS_MAP_LY );
			InputManager.SetJoypadStickAxisMap( iPad, AXIS_MAP_RX, AXIS_MAP_RX );
			InputManager.SetJoypadStickAxisMap( iPad, AXIS_MAP_RY, AXIS_MAP_RY );
			InputManager.SetJoypadStickTriggerMap( iPad, TRIGGER_MAP_0, TRIGGER_MAP_0 );
			InputManager.SetJoypadStickTriggerMap( iPad, TRIGGER_MAP_1, TRIGGER_MAP_1 );
		}
		break;
	case eGameSetting_SplitScreenVertical:
		if(iPad==ProfileManager.GetPrimaryPad())
		{
			pMinecraft->updatePlayerViewportAssignments();
		}
		break;
	case eGameSetting_GamertagsVisible:
		{
			bool bInGame=pMinecraft->level!=nullptr;

			// Game Host only
			if(bInGame && g_NetworkManager.IsHost() && (iPad==ProfileManager.GetPrimaryPad()))
			{
				// Update the Game Host setting if you are the host and you are in-game
				app.SetGameHostOption(eGameHostOption_Gamertags,((GameSettingsA[iPad]->usBitmaskValues&0x0008)!=0)?1:0);
				app.SetXuiServerAction(iPad,eXuiServerAction_ServerSettingChanged_Gamertags);

				PlayerList *players = MinecraftServer::getInstance()->getPlayerList();
				for( auto& decorationPlayer : players->players )
				{
					decorationPlayer->setShowOnMaps((app.GetGameHostOption(eGameHostOption_Gamertags)!=0)?true:false);
				}
			}
		}
		break;
		// Interim TU 1.6.6
	case eGameSetting_Sensitivity_InMenu:
		// 4J-PB - we don't use the options value
		// tell the input that we've changed the sensitivity - range of the slider is 0 to 200, default is 100
		//pMinecraft->options->set(Options::Option::SENSITIVITY,((float)GameSettingsA[iPad]->ucSensitivity)/100.0f);
		//InputManager.SetJoypadSensitivity(iPad,((float)GameSettingsA[iPad]->ucSensitivity)/100.0f);

		break;

	case eGameSetting_DisplaySplitscreenGamertags:
		for( BYTE idx = 0; idx < XUSER_MAX_COUNT; ++idx)
		{
			if(pMinecraft->localplayers[idx] != nullptr)
			{
				if(pMinecraft->localplayers[idx]->m_iScreenSection==C4JRender::VIEWPORT_TYPE_FULLSCREEN)
				{
					ui.DisplayGamertag(idx,false);
				}
				else
				{
					ui.DisplayGamertag(idx,true);
				}
			}
		}

		break;
	case eGameSetting_InterfaceOpacity:
		// update the tooltips display
		ui.RefreshTooltips( iPad);

		break;
	case eGameSetting_Hints:
		//nothing to do here
		break;
	case eGameSetting_Tooltips:
		if((GameSettingsA[iPad]->usBitmaskValues&0x8000)!=0)
		{
			ui.SetEnableTooltips(iPad,TRUE);
		}
		else
		{
			ui.SetEnableTooltips(iPad,FALSE);
		}
		break;
	case eGameSetting_Clouds:
		//nothing to do here
		break;
	case eGameSetting_Online:
		//nothing to do here
		break;
	case eGameSetting_InviteOnly:
		//nothing to do here
		break;
	case eGameSetting_FriendsOfFriends:
		//nothing to do here
		break;
    case eGameSetting_BedrockFog:
        {
            bool bInGame = pMinecraft->level != nullptr;

            if (bInGame && g_NetworkManager.IsHost() && (iPad == ProfileManager.GetPrimaryPad()))
            {
                app.SetGameHostOption(eGameHostOption_BedrockFog, GetGameSettings(iPad, eGameSetting_BedrockFog) ? 1 : 0);
                app.SetXuiServerAction(iPad, eXuiServerAction_ServerSettingChanged_BedrockFog);
            }
        }
        break;
	case eGameSetting_DisplayHUD:
		//nothing to do here
		break;
	case eGameSetting_DisplayHand:
		//nothing to do here
		break;
	case eGameSetting_CustomSkinAnim:
		//nothing to do here
		break;
	case eGameSetting_DeathMessages:
		//nothing to do here
		break;
	case eGameSetting_UISize:
		//nothing to do here
		break;
	case eGameSetting_UISizeSplitscreen:
		//nothing to do here
		break;
	case eGameSetting_AnimatedCharacter:
		//nothing to do here
		break;
	case eGameSetting_PS3_EULA_Read:
		//nothing to do here
		break;
	case eGameSetting_PSVita_NetworkModeAdhoc:
		//nothing to do here
		break;
	}
}

void CMinecraftApp::SetPlayerSkin(int iPad,const wstring &name)
{
	DWORD skinId = app.getSkinIdFromPath(name);

	SetPlayerSkin(iPad,skinId);
}

void CMinecraftApp::SetPlayerSkin(int iPad,DWORD dwSkinId)
{
	DebugPrintf("Setting skin for %d to %08X\n", iPad, dwSkinId);

	GameSettingsA[iPad]->dwSelectedSkin = dwSkinId;
	GameSettingsA[iPad]->bSettingsChanged = true;

	TelemetryManager->RecordSkinChanged(iPad, GameSettingsA[iPad]->dwSelectedSkin);

	if(Minecraft::GetInstance()->localplayers[iPad]!=nullptr) Minecraft::GetInstance()->localplayers[iPad]->setAndBroadcastCustomSkin(dwSkinId);
}


wstring CMinecraftApp::GetPlayerSkinName(int iPad)
{
	return app.getSkinPathFromId(GameSettingsA[iPad]->dwSelectedSkin);
}

DWORD CMinecraftApp::GetPlayerSkinId(int iPad)
{
	// 4J-PB -check the user has rights to use this skin - they may have had at some point but the entitlement has been removed.
	DLCPack *Pack=nullptr;
	DLCSkinFile *skinFile=nullptr;
	DWORD dwSkin=GameSettingsA[iPad]->dwSelectedSkin;
	wchar_t chars[256];

	if( GET_IS_DLC_SKIN_FROM_BITMASK(dwSkin) )
	{
		// 4J Stu - DLC skins are numbered using decimal rather than hex to make it easier to number manually
		swprintf(chars, 256, L"dlcskin%08d.png", GET_DLC_SKIN_ID_FROM_BITMASK(dwSkin));

		Pack=app.m_dlcManager.getPackContainingSkin(chars);

		if(Pack)
		{
			skinFile = Pack->getSkinFile(chars);

			bool bSkinIsFree = skinFile->getParameterAsBool( DLCManager::e_DLCParamType_Free );
			bool bLicensed = Pack->hasPurchasedFile( DLCManager::e_DLCType_Skin, skinFile->getPath() );

			if(bSkinIsFree || bLicensed)
			{
				return dwSkin;
			}
			else
			{
				return 0;
			}
		}
	}


	return dwSkin;
}

DWORD CMinecraftApp::GetAdditionalModelParts(int iPad)
{
	return m_dwAdditionalModelParts[iPad];
}


void CMinecraftApp::SetPlayerCape(int iPad,const wstring &name)
{
	DWORD capeId = Player::getCapeIdFromPath(name);

	SetPlayerCape(iPad,capeId);
}

void CMinecraftApp::SetPlayerCape(int iPad,DWORD dwCapeId)
{
	DebugPrintf("Setting cape for %d to %08X\n", iPad, dwCapeId);

	GameSettingsA[iPad]->dwSelectedCape = dwCapeId;
	GameSettingsA[iPad]->bSettingsChanged = true;

	//SentientManager.RecordSkinChanged(iPad, GameSettingsA[iPad]->dwSelectedSkin);

	if(Minecraft::GetInstance()->localplayers[iPad]!=nullptr) Minecraft::GetInstance()->localplayers[iPad]->setAndBroadcastCustomCape(dwCapeId);
}

wstring CMinecraftApp::GetPlayerCapeName(int iPad)
{
	return Player::getCapePathFromId(GameSettingsA[iPad]->dwSelectedCape);
}

DWORD CMinecraftApp::GetPlayerCapeId(int iPad)
{
	return GameSettingsA[iPad]->dwSelectedCape;
}

void CMinecraftApp::SetPlayerFavoriteSkin(int iPad, int iIndex,unsigned int uiSkinID)
{
	DebugPrintf("Setting favorite skin for %d to %08X\n", iPad, uiSkinID);

	GameSettingsA[iPad]->uiFavoriteSkinA[iIndex] = uiSkinID;
	GameSettingsA[iPad]->bSettingsChanged = true;
}

unsigned int CMinecraftApp::GetPlayerFavoriteSkin(int iPad,int iIndex)
{
	return GameSettingsA[iPad]->uiFavoriteSkinA[iIndex];
}

unsigned char CMinecraftApp::GetPlayerFavoriteSkinsPos(int iPad)
{
	return GameSettingsA[iPad]->ucCurrentFavoriteSkinPos;
}

void CMinecraftApp::SetPlayerFavoriteSkinsPos(int iPad, int iPos)
{
	GameSettingsA[iPad]->ucCurrentFavoriteSkinPos=static_cast<unsigned char>(iPos);
	GameSettingsA[iPad]->bSettingsChanged = true;
}

unsigned int CMinecraftApp::GetPlayerFavoriteSkinsCount(int iPad)
{
	unsigned int uiCount=0;
	for(int i=0;i<MAX_FAVORITE_SKINS;i++)
	{
		if(GameSettingsA[iPad]->uiFavoriteSkinA[i]!=0xFFFFFFFF)
		{
			uiCount++;
		}
		else
		{
			break;
		}
	}
	return uiCount;
}

void CMinecraftApp::ValidateFavoriteSkins(int iPad)
{
	unsigned int uiCount=GetPlayerFavoriteSkinsCount(iPad);

	// remove invalid skins
	unsigned int uiValidSkin=0;
	wchar_t chars[256];

	for(unsigned int i=0;i<uiCount;i++)
	{
		// get the pack number from the skin id
		swprintf(chars, 256, L"dlcskin%08d.png", app.GetPlayerFavoriteSkin(iPad,i));

		// Also check they haven't reverted to a trial pack
		DLCPack *pDLCPack=app.m_dlcManager.getPackContainingSkin(chars);

		if(pDLCPack!=nullptr)
		{
			// 4J-PB - We should let players add the free skins to their favourites as well!
			//DLCFile *pDLCFile=pDLCPack->getFile(DLCManager::e_DLCType_Skin,chars);
			DLCSkinFile *pSkinFile = pDLCPack->getSkinFile(chars);

			if( pDLCPack->hasPurchasedFile(DLCManager::e_DLCType_Skin, L"") || (pSkinFile && pSkinFile->isFree()))
			{
				GameSettingsA[iPad]->uiFavoriteSkinA[uiValidSkin++]=GameSettingsA[iPad]->uiFavoriteSkinA[i];
			}
		}
	}

	for(unsigned int i=uiValidSkin;i<MAX_FAVORITE_SKINS;i++)
	{
		GameSettingsA[iPad]->uiFavoriteSkinA[i]=0xFFFFFFFF;
	}
}

// Mash-up pack worlds
void CMinecraftApp::HideMashupPackWorld(int iPad, unsigned int iMashupPackID)
{
	unsigned int uiPackID=iMashupPackID - 1024; // mash-up ids start at 1024
	GameSettingsA[iPad]->uiMashUpPackWorldsDisplay&=~(1<<uiPackID);
	GameSettingsA[iPad]->bSettingsChanged = true;
}

void CMinecraftApp::EnableMashupPackWorlds(int iPad)
{
	GameSettingsA[iPad]->uiMashUpPackWorldsDisplay=0xFFFFFFFF;
	GameSettingsA[iPad]->bSettingsChanged = true;
}

unsigned int CMinecraftApp::GetMashupPackWorlds(int iPad)
{
	return GameSettingsA[iPad]->uiMashUpPackWorldsDisplay;
}

void CMinecraftApp::SetMinecraftLanguage(int iPad, unsigned char ucLanguage)
{
	GameSettingsA[iPad]->ucLanguage = ucLanguage;
	GameSettingsA[iPad]->bSettingsChanged = true;
}

unsigned char CMinecraftApp::GetMinecraftLanguage(int iPad)
{
	// if there are no game settings read yet, return the default language
	if(GameSettingsA[iPad]==nullptr)
	{
		return 0;
	}
	else
	{
		return GameSettingsA[iPad]->ucLanguage;
	}
}

void CMinecraftApp::SetMinecraftLocale(int iPad, unsigned char ucLocale)
{
	GameSettingsA[iPad]->ucLocale = ucLocale;
	GameSettingsA[iPad]->bSettingsChanged = true;
}

unsigned char CMinecraftApp::GetMinecraftLocale(int iPad)
{
	// if there are no game settings read yet, return the default language
	if(GameSettingsA[iPad]==nullptr)
	{
		return 0;
	}
	else
	{
		return GameSettingsA[iPad]->ucLocale;
	}
}

void CMinecraftApp::SetGameSettings(int iPad,eGameSetting eVal,unsigned char ucVal)
{
	//Minecraft *pMinecraft=Minecraft::GetInstance();

	switch(eVal)
	{
	case eGameSetting_MusicVolume:
		if(GameSettingsA[iPad]->ucMusicVolume!=ucVal)
		{
			GameSettingsA[iPad]->ucMusicVolume=ucVal;
			if(iPad==ProfileManager.GetPrimaryPad())
			{
				ActionGameSettings(iPad,eVal);
			}
			GameSettingsA[iPad]->bSettingsChanged=true;
		}
		break;
	case eGameSetting_SoundFXVolume:
		if(GameSettingsA[iPad]->ucSoundFXVolume!=ucVal)
		{
			GameSettingsA[iPad]->ucSoundFXVolume=ucVal;
			if(iPad==ProfileManager.GetPrimaryPad())
			{
				ActionGameSettings(iPad,eVal);
			}
			GameSettingsA[iPad]->bSettingsChanged=true;
		}
		break;
	case eGameSetting_RenderDistance:
		{
			unsigned int val = ucVal & 0xFF;

			GameSettingsA[iPad]->uiBitmaskValues &= ~(0xFF << 16);
			GameSettingsA[iPad]->uiBitmaskValues |= val << 16;
			if(iPad == ProfileManager.GetPrimaryPad())
				ActionGameSettings(iPad,eVal);
			GameSettingsA[iPad]->bSettingsChanged = true;
		}
		break;
	case eGameSetting_Gamma:
		if(GameSettingsA[iPad]->ucGamma!=ucVal)
		{
			GameSettingsA[iPad]->ucGamma=ucVal;
			if(iPad==ProfileManager.GetPrimaryPad())
			{
				ActionGameSettings(iPad,eVal);
			}
			GameSettingsA[iPad]->bSettingsChanged=true;
		}
		break;
	case eGameSetting_FOV:
		if(GameSettingsA[iPad]->ucFov!=ucVal)
		{
			GameSettingsA[iPad]->ucFov=ucVal;
			if(iPad==ProfileManager.GetPrimaryPad())
			{
				ActionGameSettings(iPad,eVal);
			}
			GameSettingsA[iPad]->bSettingsChanged=true;
		}
		break;
	case eGameSetting_Difficulty:		
		if((GameSettingsA[iPad]->usBitmaskValues&0x03)!=(ucVal&0x03))
		{
			GameSettingsA[iPad]->usBitmaskValues&=~0x03;
			GameSettingsA[iPad]->usBitmaskValues|=ucVal&0x03;
			if(iPad==ProfileManager.GetPrimaryPad())
			{
				ActionGameSettings(iPad,eVal);
			}
			GameSettingsA[iPad]->bSettingsChanged=true;
		}
		break;
	case eGameSetting_Sensitivity_InGame:
		if(GameSettingsA[iPad]->ucSensitivity!=ucVal)
		{
			GameSettingsA[iPad]->ucSensitivity=ucVal;
			ActionGameSettings(iPad,eVal);
			GameSettingsA[iPad]->bSettingsChanged=true;
		}
		break;
	case eGameSetting_ViewBob:
		if((GameSettingsA[iPad]->usBitmaskValues&0x0004)!=((ucVal&0x01)<<2))
		{
			if(ucVal!=0)
			{
				GameSettingsA[iPad]->usBitmaskValues|=0x0004;
			}
			else
			{
				GameSettingsA[iPad]->usBitmaskValues&=~0x0004;
			}
			ActionGameSettings(iPad,eVal);
			GameSettingsA[iPad]->bSettingsChanged=true;
		}
		break;
	case eGameSetting_ControlScheme: // bits 5 and 6
		if((GameSettingsA[iPad]->usBitmaskValues&0x30)!=((ucVal&0x03)<<4))
		{
			GameSettingsA[iPad]->usBitmaskValues&=~0x0030;
			if(ucVal!=0)
			{
				GameSettingsA[iPad]->usBitmaskValues|=(ucVal&0x03)<<4;
			}
			ActionGameSettings(iPad,eVal);
			GameSettingsA[iPad]->bSettingsChanged=true;
		}
		break;

	case eGameSetting_ControlInvertLook:
		if((GameSettingsA[iPad]->usBitmaskValues&0x0040)!=((ucVal&0x01)<<6))
		{
			if(ucVal!=0)
			{
				GameSettingsA[iPad]->usBitmaskValues|=0x0040;
			}
			else
			{
				GameSettingsA[iPad]->usBitmaskValues&=~0x0040;
			}
			ActionGameSettings(iPad,eVal);
			GameSettingsA[iPad]->bSettingsChanged=true;
		}
		break;

	case eGameSetting_ControlSouthPaw:
		if((GameSettingsA[iPad]->usBitmaskValues&0x0080)!=((ucVal&0x01)<<7))
		{
			if(ucVal!=0)
			{
				GameSettingsA[iPad]->usBitmaskValues|=0x0080;
			}
			else
			{
				GameSettingsA[iPad]->usBitmaskValues&=~0x0080;
			}
			ActionGameSettings(iPad,eVal);
			GameSettingsA[iPad]->bSettingsChanged=true;
		}
		break;
	case eGameSetting_SplitScreenVertical:
		if((GameSettingsA[iPad]->usBitmaskValues&0x0100)!=((ucVal&0x01)<<8))
		{
			if(ucVal!=0)
			{
				GameSettingsA[iPad]->usBitmaskValues|=0x0100;
			}
			else
			{
				GameSettingsA[iPad]->usBitmaskValues&=~0x0100;
			}
			ActionGameSettings(iPad,eVal);
			GameSettingsA[iPad]->bSettingsChanged=true;
		}
		break;
	case eGameSetting_GamertagsVisible:
		if((GameSettingsA[iPad]->usBitmaskValues&0x0008)!=((ucVal&0x01)<<3))
		{
			if(ucVal!=0)
			{
				GameSettingsA[iPad]->usBitmaskValues|=0x0008;
			}
			else
			{
				GameSettingsA[iPad]->usBitmaskValues&=~0x0008;
			}
			ActionGameSettings(iPad,eVal);
			GameSettingsA[iPad]->bSettingsChanged=true;
		}
		break;

		// 4J-PB - Added for Interim TU for 1.6.6
	case eGameSetting_Sensitivity_InMenu:
		if(GameSettingsA[iPad]->ucMenuSensitivity!=ucVal)
		{
			GameSettingsA[iPad]->ucMenuSensitivity=ucVal;
			ActionGameSettings(iPad,eVal);
			GameSettingsA[iPad]->bSettingsChanged=true;
		}
		break;
	case eGameSetting_DisplaySplitscreenGamertags:
		if((GameSettingsA[iPad]->usBitmaskValues&0x0200)!=((ucVal&0x01)<<9))
		{
			if(ucVal!=0)
			{
				GameSettingsA[iPad]->usBitmaskValues|=0x0200;
			}
			else
			{
				GameSettingsA[iPad]->usBitmaskValues&=~0x0200;
			}
			ActionGameSettings(iPad,eVal);
			GameSettingsA[iPad]->bSettingsChanged=true;
		}
		break;
	case eGameSetting_Hints:
		if((GameSettingsA[iPad]->usBitmaskValues&0x0400)!=((ucVal&0x01)<<10))
		{
			if(ucVal!=0)
			{
				GameSettingsA[iPad]->usBitmaskValues|=0x0400;
			}
			else
			{
				GameSettingsA[iPad]->usBitmaskValues&=~0x0400;
			}
			ActionGameSettings(iPad,eVal);
			GameSettingsA[iPad]->bSettingsChanged=true;
		}
		break;
	case eGameSetting_Autosave:
		if((GameSettingsA[iPad]->usBitmaskValues&0x7800)!=((ucVal&0x0F)<<11))
		{
			GameSettingsA[iPad]->usBitmaskValues&=~0x7800;
			if(ucVal!=0)
			{
				GameSettingsA[iPad]->usBitmaskValues|=(ucVal&0x0F)<<11;
			}
			ActionGameSettings(iPad,eVal);
			GameSettingsA[iPad]->bSettingsChanged=true;
		}
		break;

	case eGameSetting_Tooltips:
		if((GameSettingsA[iPad]->usBitmaskValues&0x8000)!=((ucVal&0x01)<<15))
		{
			if(ucVal!=0)
			{
				GameSettingsA[iPad]->usBitmaskValues|=0x8000;
			}
			else
			{
				GameSettingsA[iPad]->usBitmaskValues&=~0x8000;
			}
			ActionGameSettings(iPad,eVal);
			GameSettingsA[iPad]->bSettingsChanged=true;
		}
		break;
	case eGameSetting_InterfaceOpacity:
		if(GameSettingsA[iPad]->ucInterfaceOpacity!=ucVal)
		{
			GameSettingsA[iPad]->ucInterfaceOpacity=ucVal;
			ActionGameSettings(iPad,eVal);
			GameSettingsA[iPad]->bSettingsChanged=true;
		}

		break;
	case eGameSetting_Clouds:
		if((GameSettingsA[iPad]->uiBitmaskValues&GAMESETTING_CLOUDS)!=(ucVal&0x01))
		{
			if(ucVal==1)
			{
				GameSettingsA[iPad]->uiBitmaskValues|=GAMESETTING_CLOUDS;
			}
			else
			{
				GameSettingsA[iPad]->uiBitmaskValues&=~GAMESETTING_CLOUDS;
			}
			ActionGameSettings(iPad,eVal);
			GameSettingsA[iPad]->bSettingsChanged=true;
		}

		break;

	case eGameSetting_Online:
		if((GameSettingsA[iPad]->uiBitmaskValues&GAMESETTING_ONLINE)!=(ucVal&0x01)<<1)
		{
			if(ucVal==1)
			{
				GameSettingsA[iPad]->uiBitmaskValues|=GAMESETTING_ONLINE;
			}
			else
			{
				GameSettingsA[iPad]->uiBitmaskValues&=~GAMESETTING_ONLINE;
			}
			ActionGameSettings(iPad,eVal);
			GameSettingsA[iPad]->bSettingsChanged=true;
		}

		break;
	case eGameSetting_InviteOnly:
		if((GameSettingsA[iPad]->uiBitmaskValues&GAMESETTING_INVITEONLY)!=(ucVal&0x01)<<2)
		{
			if(ucVal==1)
			{
				GameSettingsA[iPad]->uiBitmaskValues|=GAMESETTING_INVITEONLY;
			}
			else
			{
				GameSettingsA[iPad]->uiBitmaskValues&=~GAMESETTING_INVITEONLY;
			}
			ActionGameSettings(iPad,eVal);
			GameSettingsA[iPad]->bSettingsChanged=true;
		}

		break;
	case eGameSetting_FriendsOfFriends:
		if((GameSettingsA[iPad]->uiBitmaskValues&GAMESETTING_FRIENDSOFFRIENDS)!=(ucVal&0x01)<<3)
		{
			if(ucVal==1)
			{
				GameSettingsA[iPad]->uiBitmaskValues|=GAMESETTING_FRIENDSOFFRIENDS;
			}
			else
			{
				GameSettingsA[iPad]->uiBitmaskValues&=~GAMESETTING_FRIENDSOFFRIENDS;
			}
			ActionGameSettings(iPad,eVal);
			GameSettingsA[iPad]->bSettingsChanged=true;
		}

		break;
	case eGameSetting_DisplayUpdateMessage:
		if((GameSettingsA[iPad]->uiBitmaskValues&GAMESETTING_DISPLAYUPDATEMSG)!=(ucVal&0x03)<<4)
		{
			GameSettingsA[iPad]->uiBitmaskValues&=~GAMESETTING_DISPLAYUPDATEMSG;
			if(ucVal>0)
			{
				GameSettingsA[iPad]->uiBitmaskValues|=(ucVal&0x03)<<4;
			}

			ActionGameSettings(iPad,eVal);
			GameSettingsA[iPad]->bSettingsChanged=true;
		}

		break;

	case eGameSetting_BedrockFog:
		if((GameSettingsA[iPad]->uiBitmaskValues&GAMESETTING_BEDROCKFOG)!=(ucVal&0x01)<<6)
		{
			if(ucVal==1)
			{
				GameSettingsA[iPad]->uiBitmaskValues|=GAMESETTING_BEDROCKFOG;
			}
			else
			{
				GameSettingsA[iPad]->uiBitmaskValues&=~GAMESETTING_BEDROCKFOG;
			}
			ActionGameSettings(iPad,eVal);
			GameSettingsA[iPad]->bSettingsChanged=true;
		}

		break;
	case eGameSetting_DisplayHUD:
		if((GameSettingsA[iPad]->uiBitmaskValues&GAMESETTING_DISPLAYHUD)!=(ucVal&0x01)<<7)
		{
			if(ucVal==1)
			{
				GameSettingsA[iPad]->uiBitmaskValues|=GAMESETTING_DISPLAYHUD;
			}
			else
			{
				GameSettingsA[iPad]->uiBitmaskValues&=~GAMESETTING_DISPLAYHUD;
			}
			ActionGameSettings(iPad,eVal);
			GameSettingsA[iPad]->bSettingsChanged=true;
		}

		break;
	case eGameSetting_DisplayHand:
		if((GameSettingsA[iPad]->uiBitmaskValues&GAMESETTING_DISPLAYHAND)!=(ucVal&0x01)<<8)
		{
			if(ucVal==1)
			{
				GameSettingsA[iPad]->uiBitmaskValues|=GAMESETTING_DISPLAYHAND;
			}
			else
			{
				GameSettingsA[iPad]->uiBitmaskValues&=~GAMESETTING_DISPLAYHAND;
			}
			ActionGameSettings(iPad,eVal);
			GameSettingsA[iPad]->bSettingsChanged=true;
		}

		break;

	case eGameSetting_CustomSkinAnim:
		if((GameSettingsA[iPad]->uiBitmaskValues&GAMESETTING_CUSTOMSKINANIM)!=(ucVal&0x01)<<9)
		{
			if(ucVal==1)
			{
				GameSettingsA[iPad]->uiBitmaskValues|=GAMESETTING_CUSTOMSKINANIM;
			}
			else
			{
				GameSettingsA[iPad]->uiBitmaskValues&=~GAMESETTING_CUSTOMSKINANIM;
			}
			ActionGameSettings(iPad,eVal);
			GameSettingsA[iPad]->bSettingsChanged=true;
		}

		break;
		// TU9
	case eGameSetting_DeathMessages:
		if((GameSettingsA[iPad]->uiBitmaskValues&GAMESETTING_DEATHMESSAGES)!=(ucVal&0x01)<<10)
		{
			if(ucVal==1)
			{
				GameSettingsA[iPad]->uiBitmaskValues|=GAMESETTING_DEATHMESSAGES;
			}
			else
			{
				GameSettingsA[iPad]->uiBitmaskValues&=~GAMESETTING_DEATHMESSAGES;
			}
			ActionGameSettings(iPad,eVal);
			GameSettingsA[iPad]->bSettingsChanged=true;
		}
		break;
	case eGameSetting_UISize:
		if((GameSettingsA[iPad]->uiBitmaskValues&GAMESETTING_UISIZE)!=((ucVal&0x03)<<11))
		{
			GameSettingsA[iPad]->uiBitmaskValues&=~GAMESETTING_UISIZE;
			if(ucVal!=0)
			{
				GameSettingsA[iPad]->uiBitmaskValues|=(ucVal&0x03)<<11;
			}
			ActionGameSettings(iPad,eVal);
			GameSettingsA[iPad]->bSettingsChanged=true;
		}
		break;
	case eGameSetting_UISizeSplitscreen:
		if((GameSettingsA[iPad]->uiBitmaskValues&GAMESETTING_UISIZE_SPLITSCREEN)!=((ucVal&0x03)<<13))
		{
			GameSettingsA[iPad]->uiBitmaskValues&=~GAMESETTING_UISIZE_SPLITSCREEN;
			if(ucVal!=0)
			{
				GameSettingsA[iPad]->uiBitmaskValues|=(ucVal&0x03)<<13;
			}
			ActionGameSettings(iPad,eVal);
			GameSettingsA[iPad]->bSettingsChanged=true;
		}
		break;
	case eGameSetting_AnimatedCharacter:
		if((GameSettingsA[iPad]->uiBitmaskValues&GAMESETTING_ANIMATEDCHARACTER)!=(ucVal&0x01)<<15)
		{
			if(ucVal==1)
			{
				GameSettingsA[iPad]->uiBitmaskValues|=GAMESETTING_ANIMATEDCHARACTER;
			}
			else
			{
				GameSettingsA[iPad]->uiBitmaskValues&=~GAMESETTING_ANIMATEDCHARACTER;
			}
			ActionGameSettings(iPad,eVal);
			GameSettingsA[iPad]->bSettingsChanged=true;
		}
		break;
	case eGameSetting_PS3_EULA_Read:
		if((GameSettingsA[iPad]->uiBitmaskValues&GAMESETTING_PS3EULAREAD)!=(ucVal&0x01)<<16)
		{
			if(ucVal==1)
			{
				GameSettingsA[iPad]->uiBitmaskValues|=GAMESETTING_PS3EULAREAD;
			}
			else
			{
				GameSettingsA[iPad]->uiBitmaskValues&=~GAMESETTING_PS3EULAREAD;
			}
			ActionGameSettings(iPad,eVal);
			GameSettingsA[iPad]->bSettingsChanged=true;
		}
		break;
	case eGameSetting_PSVita_NetworkModeAdhoc:
		if((GameSettingsA[iPad]->uiBitmaskValues&GAMESETTING_PSVITANETWORKMODEADHOC)!=(ucVal&0x01)<<17)
		{
			if(ucVal==1)
			{
				GameSettingsA[iPad]->uiBitmaskValues|=GAMESETTING_PSVITANETWORKMODEADHOC;
			}
			else
			{
				GameSettingsA[iPad]->uiBitmaskValues&=~GAMESETTING_PSVITANETWORKMODEADHOC;
			}
			ActionGameSettings(iPad,eVal);
			GameSettingsA[iPad]->bSettingsChanged=true;
		}
		break;

	}
}

unsigned char CMinecraftApp::GetGameSettings(eGameSetting eVal)
{
	int iPad=ProfileManager.GetPrimaryPad();

	return GetGameSettings(iPad,eVal);
}

unsigned char CMinecraftApp::GetGameSettings(int iPad,eGameSetting eVal)
{
	switch(eVal)
	{
	case eGameSetting_MusicVolume:
		return GameSettingsA[iPad]->ucMusicVolume;
		break;
	case eGameSetting_SoundFXVolume:
		return GameSettingsA[iPad]->ucSoundFXVolume;
		break;
	case eGameSetting_RenderDistance:
		{
		int val = (GameSettingsA[iPad]->uiBitmaskValues >> 16) & 0xFF;
		if(val == 0) return val = 16; //brain
		return val;
		break;
		}
	case eGameSetting_Gamma:
		return GameSettingsA[iPad]->ucGamma;
		break;
	case eGameSetting_FOV:
		return GameSettingsA[iPad]->ucFov;
		break;
	case eGameSetting_Difficulty:		
		return GameSettingsA[iPad]->usBitmaskValues&0x0003;
		break;
	case eGameSetting_Sensitivity_InGame:
		return GameSettingsA[iPad]->ucSensitivity;
		break;
	case eGameSetting_ViewBob:
		return ((GameSettingsA[iPad]->usBitmaskValues&0x0004)>>2);
		break;
	case eGameSetting_GamertagsVisible:
		return ((GameSettingsA[iPad]->usBitmaskValues&0x0008)>>3);
		break;
	case eGameSetting_ControlScheme:
		return ((GameSettingsA[iPad]->usBitmaskValues&0x0030)>>4); // 2 bits
		break;
	case eGameSetting_ControlInvertLook:
		return ((GameSettingsA[iPad]->usBitmaskValues&0x0040)>>6);
		break;
	case eGameSetting_ControlSouthPaw:
		return ((GameSettingsA[iPad]->usBitmaskValues&0x0080)>>7);
		break;
	case eGameSetting_SplitScreenVertical:
		return ((GameSettingsA[iPad]->usBitmaskValues&0x0100)>>8);
		break;
		// 4J-PB - Added for Interim TU for 1.6.6
	case eGameSetting_Sensitivity_InMenu:
		return GameSettingsA[iPad]->ucMenuSensitivity;
		break;

	case eGameSetting_DisplaySplitscreenGamertags:
		return ((GameSettingsA[iPad]->usBitmaskValues&0x0200)>>9);
		break;

	case eGameSetting_Hints:
		return ((GameSettingsA[iPad]->usBitmaskValues&0x0400)>>10);
		break;
	case eGameSetting_Autosave:
		{
			unsigned char ucVal=(GameSettingsA[iPad]->usBitmaskValues&0x7800)>>11;
			return ucVal;
		}
		break;
	case eGameSetting_Tooltips:
		return ((GameSettingsA[iPad]->usBitmaskValues&0x8000)>>15);
		break;

	case eGameSetting_InterfaceOpacity:
		return GameSettingsA[iPad]->ucInterfaceOpacity;
		break;

	case eGameSetting_Clouds:
		return (GameSettingsA[iPad]->uiBitmaskValues&GAMESETTING_CLOUDS);
		break;
	case eGameSetting_Online:
		return (GameSettingsA[iPad]->uiBitmaskValues&GAMESETTING_ONLINE)>>1;
		break;
	case eGameSetting_InviteOnly:
		return (GameSettingsA[iPad]->uiBitmaskValues&GAMESETTING_INVITEONLY)>>2;
		break;
	case eGameSetting_FriendsOfFriends:
		return (GameSettingsA[iPad]->uiBitmaskValues&GAMESETTING_FRIENDSOFFRIENDS)>>3;
		break;
	case eGameSetting_DisplayUpdateMessage:
		return (GameSettingsA[iPad]->uiBitmaskValues&GAMESETTING_DISPLAYUPDATEMSG)>>4;
		break;
	case eGameSetting_BedrockFog:
		return (GameSettingsA[iPad]->uiBitmaskValues&GAMESETTING_BEDROCKFOG)>>6;
		break;
	case eGameSetting_DisplayHUD:
		return (GameSettingsA[iPad]->uiBitmaskValues&GAMESETTING_DISPLAYHUD)>>7;
		break;
	case eGameSetting_DisplayHand:
		return (GameSettingsA[iPad]->uiBitmaskValues&GAMESETTING_DISPLAYHAND)>>8;
		break;
	case eGameSetting_CustomSkinAnim:
		return (GameSettingsA[iPad]->uiBitmaskValues&GAMESETTING_CUSTOMSKINANIM)>>9;
		break;
		// TU9
	case eGameSetting_DeathMessages:
		return (GameSettingsA[iPad]->uiBitmaskValues&GAMESETTING_DEATHMESSAGES)>>10;
		break;
	case eGameSetting_UISize:
		{
			unsigned char ucVal=(GameSettingsA[iPad]->uiBitmaskValues&GAMESETTING_UISIZE)>>11;
			return ucVal;
		}
		break;
	case eGameSetting_UISizeSplitscreen:
		{
			unsigned char ucVal=(GameSettingsA[iPad]->uiBitmaskValues&GAMESETTING_UISIZE_SPLITSCREEN)>>13;
			return ucVal;
		}
		break;
	case eGameSetting_AnimatedCharacter:
		return (GameSettingsA[iPad]->uiBitmaskValues&GAMESETTING_ANIMATEDCHARACTER)>>15;

	case eGameSetting_PS3_EULA_Read:
		return (GameSettingsA[iPad]->uiBitmaskValues&GAMESETTING_PS3EULAREAD)>>16;

	case eGameSetting_PSVita_NetworkModeAdhoc:
		return (GameSettingsA[iPad]->uiBitmaskValues&GAMESETTING_PSVITANETWORKMODEADHOC)>>17;

	}
	return 0;
}

void CMinecraftApp::CheckGameSettingsChanged(bool bOverride5MinuteTimer, int iPad)
{
	// If the settings have changed, write them to the profile

	if(iPad==XUSER_INDEX_ANY)
	{
		for(int i=0;i<XUSER_MAX_COUNT;i++)
		{
			if(GameSettingsA[i]->bSettingsChanged)
			{
#if ( defined __PS3__ || defined __ORBIS__ || defined _DURANGO || defined __PSVITA__ )
				StorageManager.WriteToProfile(i,true, bOverride5MinuteTimer);
#else
				ProfileManager.WriteToProfile(i,true, bOverride5MinuteTimer);
#ifdef _WINDOWS64
				Win64_SaveSettings(GameSettingsA[i]);
#endif
#endif
				GameSettingsA[i]->bSettingsChanged=false;
			}
		}
	}
	else
	{
		if(GameSettingsA[iPad]->bSettingsChanged)
		{
#if ( defined  __PS3__ || defined __ORBIS__ || defined _DURANGO || defined __PSVITA__)
			StorageManager.WriteToProfile(iPad,true, bOverride5MinuteTimer);
#else
			ProfileManager.WriteToProfile(iPad,true, bOverride5MinuteTimer);
#ifdef _WINDOWS64
			Win64_SaveSettings(GameSettingsA[iPad]);
#endif
#endif
			GameSettingsA[iPad]->bSettingsChanged=false;
		}
	}
}

void CMinecraftApp::ClearGameSettingsChangedFlag(int iPad)
{
	GameSettingsA[iPad]->bSettingsChanged=false;
}

///////////////////////////
//
// Remove the debug settings in the release build
//
////////////////////////////
#ifndef _DEBUG
unsigned int CMinecraftApp::GetGameSettingsDebugMask(int iPad,bool bOverridePlayer) //bOverridePlayer is to force the send for the server to get the read options
{
	return 0;
}

void CMinecraftApp::SetGameSettingsDebugMask(int iPad, unsigned int uiVal)
{
}

void CMinecraftApp::ActionDebugMask(int iPad,bool bSetAllClear)
{
}

#else

unsigned int CMinecraftApp::GetGameSettingsDebugMask(int iPad,bool bOverridePlayer) //bOverridePlayer is to force the send for the server to get the read options
{
	if(iPad==-1)
	{
		iPad=ProfileManager.GetPrimaryPad();
	}
	if(iPad < 0) iPad = 0;

	shared_ptr<Player> player = Minecraft::GetInstance()->localplayers[iPad];

	if(bOverridePlayer || player==nullptr)
	{
		return  GameSettingsA[iPad]->uiDebugBitmask;
	}
	else
	{
		return player->GetDebugOptions();
	}
}


void CMinecraftApp::SetGameSettingsDebugMask(int iPad, unsigned int uiVal)
{
#ifndef _CONTENT_PACKAGE
	GameSettingsA[iPad]->bSettingsChanged=true;
	GameSettingsA[iPad]->uiDebugBitmask=uiVal;

	// update the value so the network server can use it
	shared_ptr<Player> player = Minecraft::GetInstance()->localplayers[iPad];

	if(player)
	{
		Minecraft::GetInstance()->localgameModes[iPad]->handleDebugOptions(uiVal,player);
	}
#endif
}

void CMinecraftApp::ActionDebugMask(int iPad,bool bSetAllClear)
{
	unsigned int ulBitmask=app.GetGameSettingsDebugMask(iPad);

	if(bSetAllClear) ulBitmask=0L;



	// these settings should only be actioned for the primary player
	if(ProfileManager.GetPrimaryPad()!=iPad) return;

	for(int i=0;i<eDebugSetting_Max;i++)
	{
		switch(i)
		{
		case eDebugSetting_LoadSavesFromDisk:
			if(ulBitmask&(1<<i))
			{
				app.SetLoadSavesFromFolderEnabled(true);
			}
			else
			{
				app.SetLoadSavesFromFolderEnabled(false);
			}
			break;

		case eDebugSetting_WriteSavesToDisk:
			if(ulBitmask&(1<<i))
			{
				app.SetWriteSavesToFolderEnabled(true);
			}
			else
			{
				app.SetWriteSavesToFolderEnabled(false);
			}
			break;

		case eDebugSetting_FreezePlayers:	//eDebugSetting_InterfaceOff:
			if(ulBitmask&(1<<i))
			{
				app.SetFreezePlayers(true);

				// Turn off interface rendering.
				//app.SetInterfaceRenderingOff( true );
			}
			else
			{
				app.SetFreezePlayers(false);

				// Turn on interface rendering.
				//app.SetInterfaceRenderingOff( false );
			}
			break;
		case eDebugSetting_Safearea:
			if(ulBitmask&(1<<i))
			{
				app.ShowSafeArea( TRUE );
			}
			else
			{
				app.ShowSafeArea( FALSE );
			}
			break;

			//case eDebugSetting_HandRenderingOff:
			//	if(ulBitmask&(1<<i))
			//	{
			//		// Turn off hand rendering.
			//		//app.SetHandRenderingOff( true );
			//	}
			//	else
			//	{
			//		// Turn on hand rendering.
			//		//app.SetHandRenderingOff( false );
			//	}
			//	break;

		case eDebugSetting_ShowUIConsole:
			if(ulBitmask&(1<<i))
			{
				ui.ShowUIDebugConsole(true);
			}
			else
			{
				ui.ShowUIDebugConsole(false);
			}
			break;

		case eDebugSetting_ShowUIMarketingGuide:
			if(ulBitmask&(1<<i))
			{
				ui.ShowUIDebugMarketingGuide(true);
			}
			else
			{
				ui.ShowUIDebugMarketingGuide(false);
			}
			break;

		case eDebugSetting_MobsDontAttack:
			if(ulBitmask&(1<<i))
			{
				app.SetMobsDontAttackEnabled(true);
			}
			else
			{
				app.SetMobsDontAttackEnabled(false);
			}
			break;

		case eDebugSetting_UseDpadForDebug:
			if(ulBitmask&(1<<i))
			{
				app.SetUseDPadForDebug(true);
			}
			else
			{
				app.SetUseDPadForDebug(false);
			}
			break;
		case eDebugSetting_MobsDontTick:
			if(ulBitmask&(1<<i))
			{
				app.SetMobsDontTickEnabled(true);
			}
			else
			{
				app.SetMobsDontTickEnabled(false);
			}
			break;

		}
	}
}
#endif

int CMinecraftApp::DisplaySavingMessage(void *pParam, C4JStorage::ESavingMessage eVal, int iPad)
{
	//CMinecraftApp* pClass = (CMinecraftApp*)pParam;

	ui.ShowSavingMessage(iPad, eVal);

	return 0;
}

void CMinecraftApp::SetActionConfirmed(LPVOID param)
{
	XuiActionParam *actionInfo = static_cast<XuiActionParam *>(param);
	app.SetAction(actionInfo->iPad, actionInfo->action);
}


void CMinecraftApp::HandleXuiActions(void)
{
	eXuiAction eAction;
	eTMSAction eTMS;
	LPVOID param;
	Minecraft *pMinecraft=Minecraft::GetInstance();
	shared_ptr<MultiplayerLocalPlayer> player;

	// are there any global actions to deal with?
	eAction = app.GetGlobalXuiAction();
	if(eAction!=eAppAction_Idle)
	{
		switch(eAction)
		{
		case eAppAction_DisplayLavaMessage:
			// Display a warning about placing lava in the spawn area
			{
				UINT uiIDA[1];
				uiIDA[0]=IDS_CONFIRM_OK;
				C4JStorage::EMessageResult result = ui.RequestErrorMessage( IDS_CANT_PLACE_NEAR_SPAWN_TITLE, IDS_CANT_PLACE_NEAR_SPAWN_TEXT, uiIDA,1,XUSER_INDEX_ANY);
				if(result != C4JStorage::EMessage_Busy) SetGlobalXuiAction(eAppAction_Idle);

			}
			break;
		default:
			break;
		}
	}

	// are there any app actions to deal with?
	for(int i=0;i<XUSER_MAX_COUNT;i++)
	{
		eAction = app.GetXuiAction(i);
		param = m_eXuiActionParam[i];

		if(eAction!=eAppAction_Idle)
		{
			switch(eAction)
			{
				// the renderer will capture a screenshot
			case eAppAction_SocialPost:
				if(ProfileManager.IsFullVersion())
				{
					// Facebook Share
					if( CSocialManager::Instance()->IsTitleAllowedToPostImages() && CSocialManager::Instance()->AreAllUsersAllowedToPostImages() )
					{
						// disable character name tags for the shot
						//m_bwasHidingGui = pMinecraft->options->hideGui; // 4J Stu - Removed 1.8.2 bug fix (TU6) as don't need this
						pMinecraft->options->hideGui = true;

						SetAction(i,eAppAction_SocialPostScreenshot);
					}
					else
					{
						SetAction(i,eAppAction_Idle);
					}
				}
				else
				{
					SetAction(i,eAppAction_Idle);
				}
				break;
			case eAppAction_SocialPostScreenshot:
				{
					SetAction(i,eAppAction_Idle);
					bool bKeepHiding = false;
					for(int j=0; j < XUSER_MAX_COUNT;++j)
					{
						if(app.GetXuiAction(j) == eAppAction_SocialPostScreenshot)
						{
							bKeepHiding = true;
							break;
						}
					}
					pMinecraft->options->hideGui=bKeepHiding;

					// Facebook Share

					if(app.GetLocalPlayerCount()>1)
					{
						ui.NavigateToScene(i,eUIScene_SocialPost);
					}
					else
					{
						ui.NavigateToScene(i,eUIScene_SocialPost);
					}
				}
				break;
			case eAppAction_SaveGame:
				SetAction(i,eAppAction_Idle);
				if(!GetChangingSessionType())
				{
					// If this is the trial game, do an upsell
					if(ProfileManager.IsFullVersion())
					{

						// flag the render to capture the screenshot for the save
						SetAction(i,eAppAction_SaveGameCapturedThumbnail);
					}
					else
					{
						// ask the player if they would like to upgrade, or they'll lose the level

						UINT uiIDA[2];
						uiIDA[0]=IDS_CONFIRM_OK;
						uiIDA[1]=IDS_CONFIRM_CANCEL;
						ui.RequestErrorMessage(IDS_UNLOCK_TITLE, IDS_UNLOCK_TOSAVE_TEXT, uiIDA, 2,i,&CMinecraftApp::UnlockFullSaveReturned,this);
					}
				}

				break;
			case eAppAction_AutosaveSaveGame:
				{
					// Need to run a check to see if the save exists in order to stop the dialog asking if we want to overwrite it coming up on an autosave
					bool bSaveExists;
					StorageManager.DoesSaveExist(&bSaveExists);

					SetAction(i,eAppAction_Idle);
					if(!GetChangingSessionType())
					{

						// flag the render to capture the screenshot for the save
						SetAction(i,eAppAction_AutosaveSaveGameCapturedThumbnail);
					}
				}

				break;

			case eAppAction_SaveGameCapturedThumbnail:
				// reset the autosave timer
				app.SetAutosaveTimerTime();
				SetAction(i,eAppAction_Idle);
				// Check that there is a name for the save - if we're saving from the tutorial and this is the first save from the tutorial, we'll not have a name
				/*if(StorageManager.GetSaveName()==nullptr)
				{
				app.NavigateToScene(i,eUIScene_SaveWorld);
				}
				else*/
				{
					// turn off the gamertags in splitscreen for the primary player, since they are about to be made fullscreen
					ui.HideAllGameUIElements();

					// Hide the other players scenes
					ui.ShowOtherPlayersBaseScene(ProfileManager.GetPrimaryPad(), false);

					//INT saveOrCheckpointId = 0;
					//bool validSave = StorageManager.GetSaveUniqueNumber(&saveOrCheckpointId);
					//SentientManager.RecordLevelSaveOrCheckpoint(ProfileManager.GetPrimaryPad(), saveOrCheckpointId);

					LoadingInputParams *loadingParams = new LoadingInputParams();
					loadingParams->func = &UIScene_PauseMenu::SaveWorldThreadProc;
					loadingParams->lpParam = static_cast<LPVOID>(false);

					// 4J-JEV - PS4: Fix for #5708 - [ONLINE] - If the user pulls their network cable out while saving the title will hang.
					loadingParams->waitForThreadToDelete = true;

					UIFullscreenProgressCompletionData *completionData = new UIFullscreenProgressCompletionData();
					completionData->bShowBackground=TRUE;
					completionData->bShowLogo=TRUE;
					completionData->type = e_ProgressCompletion_NavigateBackToScene;
					completionData->iPad = ProfileManager.GetPrimaryPad();

					if( ui.IsSceneInStack( ProfileManager.GetPrimaryPad(), eUIScene_EndPoem ) )
					{
						completionData->scene = eUIScene_EndPoem;
					}
					else
					{
						completionData->scene = eUIScene_PauseMenu;
					}

					loadingParams->completionData = completionData;

					// 4J Stu - Xbox only
#ifdef _XBOX
					// Temporarily make this scene fullscreen
					CXuiSceneBase::SetPlayerBaseScenePosition( ProfileManager.GetPrimaryPad(), CXuiSceneBase::e_BaseScene_Fullscreen );
#endif

					ui.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_FullscreenProgress, loadingParams , eUILayer_Fullscreen, eUIGroup_Fullscreen);

				}
				break;
			case eAppAction_AutosaveSaveGameCapturedThumbnail:

				{
					app.SetAutosaveTimerTime();
					SetAction(i,eAppAction_Idle);

#if defined(_XBOX_ONE) || defined(__ORBIS__)
					app.SetXuiServerAction(ProfileManager.GetPrimaryPad(),eXuiServerAction_AutoSaveGame);

					if(app.GetGameHostOption(eGameHostOption_DisableSaving)) StorageManager.SetSaveDisabled(true);
#else
					// turn off the gamertags in splitscreen for the primary player, since they are about to be made fullscreen
					ui.HideAllGameUIElements();

					//app.CloseAllPlayersXuiScenes();
					// Hide the other players scenes
					ui.ShowOtherPlayersBaseScene(ProfileManager.GetPrimaryPad(), false);

					// This just allows it to be shown
					if(pMinecraft->localgameModes[ProfileManager.GetPrimaryPad()] != nullptr) pMinecraft->localgameModes[ProfileManager.GetPrimaryPad()]->getTutorial()->showTutorialPopup(false);

					//INT saveOrCheckpointId = 0;
					//bool validSave = StorageManager.GetSaveUniqueNumber(&saveOrCheckpointId);
					//SentientManager.RecordLevelSaveOrCheckpoint(ProfileManager.GetPrimaryPad(), saveOrCheckpointId);


					LoadingInputParams *loadingParams = new LoadingInputParams();
					loadingParams->func = &UIScene_PauseMenu::SaveWorldThreadProc;

					loadingParams->lpParam = (LPVOID)(true);

					UIFullscreenProgressCompletionData *completionData = new UIFullscreenProgressCompletionData();
					completionData->bShowBackground=TRUE;
					completionData->bShowLogo=TRUE;
					completionData->type = e_ProgressCompletion_AutosaveNavigateBack;
					completionData->iPad = ProfileManager.GetPrimaryPad();
					//completionData->bAutosaveWasMenuDisplayed=ui.GetMenuDisplayed(ProfileManager.GetPrimaryPad());
					loadingParams->completionData = completionData;

					// 4J Stu - Xbox only
#ifdef _XBOX
					// Temporarily make this scene fullscreen
					CXuiSceneBase::SetPlayerBaseScenePosition( ProfileManager.GetPrimaryPad(), CXuiSceneBase::e_BaseScene_Fullscreen );
#endif

					ui.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_FullscreenProgress, loadingParams , eUILayer_Fullscreen, eUIGroup_Fullscreen);
#endif
				}
				break;
			case eAppAction_ExitPlayer:
				// a secondary player has chosen to quit
				{
					int iPlayerC=g_NetworkManager.GetPlayerCount();

					// Since the player is exiting, let's flush any profile writes for them, and hope we're not breaking TCR 136...
#if (defined __PS3__ || defined __ORBIS__ || defined _DURANGO  || defined __PSVITA__)
					StorageManager.ForceQueuedProfileWrites(i);
					LeaderboardManager::Instance()->OpenSession();
					for (int j = 0; j < XUSER_MAX_COUNT; j++)
					{
						if( ProfileManager.IsSignedIn(j) )
						{
							app.DebugPrintf("Stats save for an offline game for the player at index %d\n", 0);
							Minecraft::GetInstance()->forceStatsSave(j);
						}
					}
					LeaderboardManager::Instance()->CloseSession();
#else
					ProfileManager.ForceQueuedProfileWrites(i);
#endif

					// not required - it's done within the removeLocalPlayerIdx
					// 				if(pMinecraft->level->isClientSide)
					// 				{
					// 					// we need to remove the qnetplayer, or this player won't be able to get back into the game until qnet times out and removes them
					// 					g_NetworkManager.NotifyPlayerLeaving(g_NetworkManager.GetLocalPlayerByUserIndex(i));
					// 				}

					// if there are any tips showing, we need to close them

					pMinecraft->gui->clearMessages(i);

					// Make sure we've not got this player selected as current - this shouldn't be the case anyway
					pMinecraft->setLocalPlayerIdx(ProfileManager.GetPrimaryPad());
					pMinecraft->removeLocalPlayerIdx(i);

#ifdef _XBOX
					// tell the xui scenes a splitscreen player left - has to come after removeLocalPlayerIdx which calls updatePlayerViewportAssignments
					XUIMessage xuiMsg;
					CustomMessage_Splitscreenplayer_Struct myMsgData;
					CustomMessage_Splitscreenplayer( &xuiMsg, &myMsgData, false);

					// send the message
					for(int idx=0;idx<XUSER_MAX_COUNT;idx++)
					{
						if((i!=idx) && (pMinecraft->localplayers[idx]!=nullptr))
						{
							XuiBroadcastMessage( CXuiSceneBase::GetPlayerBaseScene(idx), &xuiMsg );
						}
					}
#endif

#ifndef _XBOX
					// Wipe out the tooltips
					ui.SetTooltips(i, -1);
#endif

					// Change the presence info
					// Are we offline or online, and how many players are there
					if(iPlayerC>2) // one player is about to leave here - they'll be set to idle in the qnet manager player leave
					{
						for(int iPlayer=0;iPlayer<XUSER_MAX_COUNT;iPlayer++)
						{
							if((iPlayer!=i) && pMinecraft->localplayers[iPlayer])
							{
								if(g_NetworkManager.IsLocalGame())
								{
									ProfileManager.SetCurrentGameActivity(iPlayer,CONTEXT_PRESENCE_MULTIPLAYEROFFLINE,false);
								}
								else
								{
									ProfileManager.SetCurrentGameActivity(iPlayer,CONTEXT_PRESENCE_MULTIPLAYER,false);
								}
							}
						}
					}
					else
					{
						for(int iPlayer=0;iPlayer<XUSER_MAX_COUNT;iPlayer++)
						{
							if((iPlayer!=i) && pMinecraft->localplayers[iPlayer])
							{
								if(g_NetworkManager.IsLocalGame())
								{
									ProfileManager.SetCurrentGameActivity(iPlayer,CONTEXT_PRESENCE_MULTIPLAYER_1POFFLINE,false);
								}
								else
								{
									ProfileManager.SetCurrentGameActivity(iPlayer,CONTEXT_PRESENCE_MULTIPLAYER_1P,false);
								}
							}
						}
					}

#ifdef _DURANGO
					ProfileManager.RemoveGamepadFromGame(i);
#endif

					SetAction(i,eAppAction_Idle);
				}
				break;
			case eAppAction_ExitPlayerPreLogin:
				{
					int iPlayerC=g_NetworkManager.GetPlayerCount();
					// Since the player is exiting, let's flush any profile writes for them, and hope we're not breaking TCR 136...
#if (defined __PS3__ || defined __ORBIS__ || defined _DURANGO  || defined __PSVITA__)
					StorageManager.ForceQueuedProfileWrites(i);
#else
					ProfileManager.ForceQueuedProfileWrites(i);
#endif
					// if there are any tips showing, we need to close them

					pMinecraft->gui->clearMessages(i);

					// Make sure we've not got this player selected as current - this shouldn't be the case anyway
					pMinecraft->setLocalPlayerIdx(ProfileManager.GetPrimaryPad());
					pMinecraft->removeLocalPlayerIdx(i);

#ifdef _XBOX
					// tell the xui scenes a splitscreen player left - has to come after removeLocalPlayerIdx which calls updatePlayerViewportAssignments
					XUIMessage xuiMsg;
					CustomMessage_Splitscreenplayer_Struct myMsgData;
					CustomMessage_Splitscreenplayer( &xuiMsg, &myMsgData, false);

					// send the message
					for(int idx=0;idx<XUSER_MAX_COUNT;idx++)
					{
						if((i!=idx) && (pMinecraft->localplayers[idx]!=nullptr))
						{
							XuiBroadcastMessage( CXuiSceneBase::GetPlayerBaseScene(idx), &xuiMsg );
						}
					}
#endif

#ifndef _XBOX
					// Wipe out the tooltips
					ui.SetTooltips(i, -1);
#endif

					// Change the presence info
					// Are we offline or online, and how many players are there
					if(iPlayerC>2) // one player is about to leave here - they'll be set to idle in the qnet manager player leave
					{
						for(int iPlayer=0;iPlayer<XUSER_MAX_COUNT;iPlayer++)
						{
							if((iPlayer!=i) && pMinecraft->localplayers[iPlayer])
							{
								if(g_NetworkManager.IsLocalGame())
								{
									ProfileManager.SetCurrentGameActivity(iPlayer,CONTEXT_PRESENCE_MULTIPLAYEROFFLINE,false);
								}
								else
								{
									ProfileManager.SetCurrentGameActivity(iPlayer,CONTEXT_PRESENCE_MULTIPLAYER,false);
								}
							}
						}
					}
					else
					{
						for(int iPlayer=0;iPlayer<XUSER_MAX_COUNT;iPlayer++)
						{
							if((iPlayer!=i) && pMinecraft->localplayers[iPlayer])
							{
								if(g_NetworkManager.IsLocalGame())
								{
									ProfileManager.SetCurrentGameActivity(iPlayer,CONTEXT_PRESENCE_MULTIPLAYER_1POFFLINE,false);
								}
								else
								{
									ProfileManager.SetCurrentGameActivity(iPlayer,CONTEXT_PRESENCE_MULTIPLAYER_1P,false);
								}
							}
						}
					}
					SetAction(i,eAppAction_Idle);
				}
				break;

#ifdef __ORBIS__
			case eAppAction_OptionsSaveNoSpace:
				{
					SetAction(i,eAppAction_Idle);

					SceSaveDataDialogParam param;
					SceSaveDataDialogSystemMessageParam sysParam;
					SceSaveDataDialogItems items;
					SceSaveDataDirName dirName;

					sceSaveDataDialogParamInitialize(&param);
					param.mode = SCE_SAVE_DATA_DIALOG_MODE_SYSTEM_MSG;
					param.dispType = SCE_SAVE_DATA_DIALOG_TYPE_SAVE;
					memset(&sysParam,0,sizeof(sysParam));
					param.sysMsgParam = &sysParam;
					param.sysMsgParam->sysMsgType = SCE_SAVE_DATA_DIALOG_SYSMSG_TYPE_NOSPACE_CONTINUABLE;
					param.sysMsgParam->value = app.GetOptionsBlocksRequired(i);
					memset(&items, 0, sizeof(items));
					param.items = &items;
					param.items->userId = ProfileManager.getUserID(i);

					int ret = sceSaveDataDialogInitialize();
					ret = sceSaveDataDialogOpen(&param);

					app.SetOptionsSaveDataDialogRunning(true);//m_bOptionsSaveDataDialogRunning = true;
					//pClass->m_eSaveIncompleteType = saveIncompleteType;

					//StorageManager.SetSaveDisabled(true);
					//pClass->EnterSaveNotificationSection();

				}
				break;
#endif

			case eAppAction_ExitWorld:

				SetAction(i,eAppAction_Idle);

				// If we're already leaving don't exit
				if (g_NetworkManager.IsLeavingGame())
				{
					break;
				}

				pMinecraft->gui->clearMessages();

				// turn off the gamertags in splitscreen for the primary player, since they are about to be made fullscreen
				ui.HideAllGameUIElements();

				// reset the flag stopping new dlc message being shown if you've seen the message before
				DisplayNewDLCTipAgain();

				// clear the autosave timer that might be on screen
				ui.ShowAutosaveCountdownTimer(false);

				// Hide the selected item text
				ui.HideAllGameUIElements();

				// Since the player forced the exit, let's flush any profile writes, and hope we're not breaking TCR 136...
#if (defined __PS3__ || defined __ORBIS__  || defined _DURANGO  || defined __PSVITA__)
				StorageManager.ForceQueuedProfileWrites();
				LeaderboardManager::Instance()->OpenSession();
				for (int j = 0; j < XUSER_MAX_COUNT; j++)
				{
					if( ProfileManager.IsSignedIn(j) )
					{
						app.DebugPrintf("Stats save for an offline game for the player at index %d\n", 0);
						Minecraft::GetInstance()->forceStatsSave(j);
					}
				}
				LeaderboardManager::Instance()->CloseSession();
#elif (defined _XBOX)
				ProfileManager.ForceQueuedProfileWrites();
#endif

				// 4J-PB - cancel any possible string verifications queued with LIVE
				//InputManager.CancelAllVerifyInProgress();

				if(ProfileManager.IsFullVersion())
				{

					// In a split screen, only the primary player actually quits the game, others just remove their players
					if( i != ProfileManager.GetPrimaryPad() )
					{
						// Make sure we've not got this player selected as current - this shouldn't be the case anyway
						pMinecraft->setLocalPlayerIdx(ProfileManager.GetPrimaryPad());
						pMinecraft->removeLocalPlayerIdx(i);

#ifdef _DURANGO
						ProfileManager.RemoveGamepadFromGame(i);
#endif
						SetAction(i,eAppAction_Idle);
						return;
					}
					// flag to capture the save thumbnail
					SetAction(i,eAppAction_ExitWorldCapturedThumbnail, param);
				}
				else
				{
					// ask the player if they would like to upgrade, or they'll lose the level
					UINT uiIDA[2];
					uiIDA[0]=IDS_CONFIRM_OK;
					uiIDA[1]=IDS_CONFIRM_CANCEL;
					ui.RequestErrorMessage(IDS_UNLOCK_TITLE, IDS_UNLOCK_TOSAVE_TEXT, uiIDA, 2, i,&CMinecraftApp::UnlockFullExitReturned,this);
				}

				// Change the presence info
				// Are we offline or online, and how many players are there

				if(g_NetworkManager.GetPlayerCount()>1)
				{
					for(int j=0;j<XUSER_MAX_COUNT;j++)
					{
						if(pMinecraft->localplayers[j])
						{
							if(g_NetworkManager.IsLocalGame())
							{
								app.SetRichPresenceContext(j,CONTEXT_GAME_STATE_BLANK);
								ProfileManager.SetCurrentGameActivity(j,CONTEXT_PRESENCE_MULTIPLAYEROFFLINE,false);
							}
							else
							{
								app.SetRichPresenceContext(j,CONTEXT_GAME_STATE_BLANK);
								ProfileManager.SetCurrentGameActivity(j,CONTEXT_PRESENCE_MULTIPLAYER,false);
							}
							TelemetryManager->RecordLevelExit(j, eSen_LevelExitStatus_Exited);
						}
					}
				}
				else
				{
					app.SetRichPresenceContext(i,CONTEXT_GAME_STATE_BLANK);
					if(g_NetworkManager.IsLocalGame())
					{
						ProfileManager.SetCurrentGameActivity(i,CONTEXT_PRESENCE_MULTIPLAYER_1POFFLINE,false);
					}
					else
					{
						ProfileManager.SetCurrentGameActivity(i,CONTEXT_PRESENCE_MULTIPLAYER_1P,false);
					}
					TelemetryManager->RecordLevelExit(i, eSen_LevelExitStatus_Exited);
				}
				break;
			case eAppAction_ExitWorldCapturedThumbnail:
				{
					SetAction(i,eAppAction_Idle);
					// Stop app running
					SetGameStarted(false);
					SetChangingSessionType(true); // Added to stop handling ethernet disconnects

					ui.CloseAllPlayersScenes();

					// turn off the gamertags in splitscreen for the primary player, since they are about to be made fullscreen
					ui.HideAllGameUIElements();

					// 4J Stu - Fix for #12368 - Crash: Game crashes when saving then exiting and selecting to save
					for(unsigned int idx = 0; idx < XUSER_MAX_COUNT; ++idx)
					{
#ifdef _XBOX
						app.TutorialSceneNavigateBack(idx,true);
#endif

						// 4J Stu - Fix for #13257 - CRASH: Gameplay: Title crashed after exiting the tutorial
						// It doesn't matter if they were in the tutorial already
						pMinecraft->playerLeftTutorial( idx );
					}

					LoadingInputParams *loadingParams = new LoadingInputParams();
					loadingParams->func = &UIScene_PauseMenu::ExitWorldThreadProc;
					loadingParams->lpParam = param;

					UIFullscreenProgressCompletionData *completionData = new UIFullscreenProgressCompletionData();
					// If param is non-null then this is a forced exit by the server, so make sure the player knows why
					// 4J Stu - Changed - Don't use the FullScreenProgressScreen for action, use a dialog instead
					completionData->bRequiresUserAction = FALSE;//(param != nullptr) ? TRUE : FALSE;
					completionData->bShowTips = (param != nullptr) ? FALSE : TRUE;
					completionData->bShowBackground=TRUE;
					completionData->bShowLogo=TRUE;
					completionData->type = e_ProgressCompletion_NavigateToHomeMenu;
					completionData->iPad = DEFAULT_XUI_MENU_USER;
					loadingParams->completionData = completionData;

					ui.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_FullscreenProgress, loadingParams);
				}
				break;
			case eAppAction_ExitWorldTrial:
				{
					SetAction(i,eAppAction_Idle);

					pMinecraft->gui->clearMessages();

					// turn off the gamertags in splitscreen for the primary player, since they are about to be made fullscreen
					ui.HideAllGameUIElements();

					// Stop app running
					SetGameStarted(false);

					ui.CloseAllPlayersScenes();

					// 4J Stu - Fix for #12368 - Crash: Game crashes when saving then exiting and selecting to save
					for(unsigned int idx = 0; idx < XUSER_MAX_COUNT; ++idx)
					{
#ifdef _XBOX
						app.TutorialSceneNavigateBack(idx,true);
#endif

						// 4J Stu - Fix for #13257 - CRASH: Gameplay: Title crashed after exiting the tutorial
						// It doesn't matter if they were in the tutorial already
						pMinecraft->playerLeftTutorial( idx );
					}

					LoadingInputParams *loadingParams = new LoadingInputParams();
					loadingParams->func = &UIScene_PauseMenu::ExitWorldThreadProc;
					loadingParams->lpParam = param;

					UIFullscreenProgressCompletionData *completionData = new UIFullscreenProgressCompletionData();
					completionData->bShowBackground=TRUE;
					completionData->bShowLogo=TRUE;
					completionData->type = e_ProgressCompletion_NavigateToHomeMenu;
					completionData->iPad = DEFAULT_XUI_MENU_USER;
					loadingParams->completionData = completionData;

					ui.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_FullscreenProgress, loadingParams);
				}

				break;
			case eAppAction_ExitTrial:
				//XLaunchNewImage(XLAUNCH_KEYWORD_DASH_ARCADE, 0);
				ExitGame();
				break;

			case eAppAction_Respawn:
				{
					ConnectionProgressParams *param = new ConnectionProgressParams();
					param->iPad = i;
					param->stringId = IDS_PROGRESS_RESPAWNING;
					param->showTooltips = false;
					param->setFailTimer = false;
					ui.NavigateToScene(i,eUIScene_ConnectingProgress, param);

					// Need to reset this incase the player has already died and respawned
					pMinecraft->localplayers[i]->SetPlayerRespawned(false);

					SetAction(i,eAppAction_WaitForRespawnComplete);
					if( app.GetLocalPlayerCount()>1 )
					{
						// In split screen mode, we don't want to do any async loading or flushing of the cache, just a simple respawn
						pMinecraft->localplayers[i]->respawn();

						// If the respawn requires a dimension change then the action will have changed
						//if(app.GetXuiAction(i) == eAppAction_Respawn)
						//{
						//	SetAction(i,eAppAction_Idle);
						//	CloseXuiScenes(i);
						//}
					}
					else
					{
						//SetAction(i,eAppAction_WaitForRespawnComplete);

						//LoadingInputParams *loadingParams = new LoadingInputParams();
						//loadingParams->func = &CScene_Death::RespawnThreadProc;
						//loadingParams->lpParam = (LPVOID)i;

						// Disable game & update thread whilst we do any of this
						//app.SetGameStarted(false);
						pMinecraft->gameRenderer->DisableUpdateThread();

						// 4J Stu - We don't need this on a thread in multiplayer as respawning is asynchronous.
						pMinecraft->localplayers[i]->respawn();

						//app.SetGameStarted(true);
						pMinecraft->gameRenderer->EnableUpdateThread();

						//UIFullscreenProgressCompletionData *completionData = new UIFullscreenProgressCompletionData();
						//completionData->bShowBackground=TRUE;
						//completionData->bShowLogo=TRUE;
						//completionData->type = e_ProgressCompletion_CloseUIScenes;
						//completionData->iPad = i;
						//loadingParams->completionData = completionData;

						//app.NavigateToScene(i,eUIScene_FullscreenProgress, loadingParams, true);
					}
				}
				break;
			case eAppAction_WaitForRespawnComplete:
				player = pMinecraft->localplayers[i];
				if(player != nullptr && player->GetPlayerRespawned())
				{
					SetAction(i,eAppAction_Idle);

					if(ui.IsSceneInStack(i, eUIScene_EndPoem))
					{
						ui.NavigateBack(i,false,eUIScene_EndPoem);
					}
					else
					{
						ui.CloseUIScenes(i);
					}

					// clear the progress messages

					// 					pMinecraft->progressRenderer->progressStart(-1);
					// 					pMinecraft->progressRenderer->progressStage(-1);
				}
				else if(!g_NetworkManager.IsInGameplay())
				{
					SetAction(i,eAppAction_Idle);
				}
				break;
			case eAppAction_WaitForDimensionChangeComplete:
				player = pMinecraft->localplayers[i];
				if(player != nullptr && player->connection && player->connection->isStarted())
				{
					SetAction(i,eAppAction_Idle);
					ui.CloseUIScenes(i);
				}
				else if(!g_NetworkManager.IsInGameplay())
				{
					SetAction(i,eAppAction_Idle);
				}
				break;
			case eAppAction_PrimaryPlayerSignedOut:
				{
					//SetAction(i,eAppAction_Idle);

					// clear the autosavetimer that might be displayed
					ui.ShowAutosaveCountdownTimer(false);

					// If the player signs out before the game started the server can be killed a bit earlier to stop
					// the loading or saving of a new game continuing running while the UI/Guide is up
					if(!app.GetGameStarted()) MinecraftServer::HaltServer(true);

					// inform the player they are being returned to the menus because they signed out
					StorageManager.SetSaveDeviceSelected(i,false);
					// need to clear the player stats - can't assume it'll be done in setlevel - we may not be in the game
					StatsCounter* pStats = Minecraft::GetInstance()->stats[ i ];
					pStats->clear();

					// 4J-PB - the libs will display the Returned to Title screen
					// 					UINT uiIDA[1];
					// 					uiIDA[0]=IDS_CONFIRM_OK;
					//
					// 					ui.RequestMessageBox(IDS_RETURNEDTOMENU_TITLE, IDS_RETURNEDTOTITLESCREEN_TEXT, uiIDA, 1, i,&CMinecraftApp::PrimaryPlayerSignedOutReturned,this,app.GetStringTable());
					if( g_NetworkManager.IsInSession() )
					{
						app.SetAction(i,eAppAction_PrimaryPlayerSignedOutReturned);
					}
					else
					{
						app.SetAction(i,eAppAction_PrimaryPlayerSignedOutReturned_Menus);
						MinecraftServer::resetFlags();
					}
				}
				break;
			case eAppAction_EthernetDisconnected:
				{
					app.DebugPrintf("Handling eAppAction_EthernetDisconnected\n");
					SetAction(i,eAppAction_Idle);

					// 4J Stu - Fix for #12530 -TCR 001 BAS Game Stability: Title will crash if the player disconnects while starting a new world and then opts to play the tutorial once they have been returned to the Main Menu.
					if(!g_NetworkManager.IsLeavingGame())
					{
						app.DebugPrintf("Handling eAppAction_EthernetDisconnected - Not leaving game\n");
						// 4J-PB - not the same as a signout. We should only leave the game if this machine is not the host. We shouldn't get rid of the save device either.
						if( g_NetworkManager.IsHost() )
						{
							app.DebugPrintf("Handling eAppAction_EthernetDisconnected - Is Host\n");
							// If it's already a local game, then an ethernet disconnect should have no effect
							if( !g_NetworkManager.IsLocalGame() && g_NetworkManager.IsInGameplay() )
							{
								// Change the session to an offline session
								SetAction(i,eAppAction_ChangeSessionType);
							}
							else if(!g_NetworkManager.IsLocalGame() && !g_NetworkManager.IsInGameplay() )
							{
								// There are two cases here, either:
								//	 1. We're early enough in the create/load game that we can do a really minimal shutdown or
								//   2. We're far enough in (game has started but the actual game started flag hasn't been set) that we should just wait until we're in the game and switch to offline mode

								// If there's a non-null level then, for our purposes, the game has started
								bool gameStarted = false;
								for(int j = 0; j < pMinecraft->levels.length; j++)
								{
									if (pMinecraft->levels.data[i] != nullptr)
									{
										gameStarted = true;
										break;
									}
								}

								if (!gameStarted)
								{
									// 1. Exit
									MinecraftServer::HaltServer();

									// Fix for #12530 - TCR 001 BAS Game Stability: Title will crash if the player disconnects while starting a new world and then opts to play the tutorial once they have been returned to the Main Menu.
									// 4J Stu - Leave the session
									g_NetworkManager.LeaveGame(FALSE);

									// need to clear the player stats - can't assume it'll be done in setlevel - we may not be in the game
									StatsCounter* pStats = Minecraft::GetInstance()->stats[ i ];
									pStats->clear();
									UINT uiIDA[1];
									uiIDA[0]=IDS_CONFIRM_OK;

									ui.RequestErrorMessage(g_NetworkManager.CorrectErrorIDS(IDS_CONNECTION_LOST), g_NetworkManager.CorrectErrorIDS(IDS_CONNECTION_LOST_LIVE), uiIDA, 1, i,&CMinecraftApp::EthernetDisconnectReturned,this);
								}
								else
								{
									// 2. Switch to offline
									SetAction(i,eAppAction_ChangeSessionType);
								}
							}
						}
						else
						{
#if defined __PS3__ || defined __ORBIS__ || defined __PSVITA__
							if(UIScene_LoadOrJoinMenu::isSaveTransferRunning())
							{
								// the save transfer is still in progress, delay jumping back to the main menu until we've cleaned up
								SetAction(i,eAppAction_EthernetDisconnected);
							}
							else
#endif
							{
							app.DebugPrintf("Handling eAppAction_EthernetDisconnected - Not host\n");
							// need to clear the player stats - can't assume it'll be done in setlevel - we may not be in the game
							StatsCounter* pStats = Minecraft::GetInstance()->stats[ i ];
							pStats->clear();
							UINT uiIDA[1];
							uiIDA[0]=IDS_CONFIRM_OK;

							ui.RequestErrorMessage(g_NetworkManager.CorrectErrorIDS(IDS_CONNECTION_LOST), g_NetworkManager.CorrectErrorIDS(IDS_CONNECTION_LOST_LIVE), uiIDA, 1, i,&CMinecraftApp::EthernetDisconnectReturned,this);

							}
						}
					}
				}
				break;
				// We currently handle both these returns the same way.
			case eAppAction_EthernetDisconnectedReturned:
			case eAppAction_PrimaryPlayerSignedOutReturned:
				{
					SetAction(i,eAppAction_Idle);

					pMinecraft->gui->clearMessages();

					// turn off the gamertags in splitscreen for the primary player, since they are about to be made fullscreen
					ui.HideAllGameUIElements();

					// set the state back to pre-game
					ProfileManager.ResetProfileProcessState();


					if( g_NetworkManager.IsLeavingGame() )
					{
						// 4J Stu - If we are already leaving the game, then we just need to signal that the player signed out to stop saves
						pMinecraft->progressRenderer->progressStartNoAbort( IDS_EXITING_GAME );
						pMinecraft->progressRenderer->progressStage(-1);
						// This has no effect on client machines
						MinecraftServer::HaltServer(true);
					}
					else
					{
						// Stop app running
						SetGameStarted(false);

						// turn off the gamertags in splitscreen for the primary player, since they are about to be made fullscreen
						ui.HideAllGameUIElements();

						ui.CloseAllPlayersScenes();

						// 4J Stu - Fix for #12368 - Crash: Game crashes when saving then exiting and selecting to save
						for(unsigned int idx = 0; idx < XUSER_MAX_COUNT; ++idx)
						{
#ifdef _XBOX
							app.TutorialSceneNavigateBack(idx,true);
#endif

							// 4J Stu - Fix for #13257 - CRASH: Gameplay: Title crashed after exiting the tutorial
							// It doesn't matter if they were in the tutorial already
							pMinecraft->playerLeftTutorial( idx );
						}

						LoadingInputParams *loadingParams = new LoadingInputParams();
						loadingParams->func = &CMinecraftApp::SignoutExitWorldThreadProc;

						UIFullscreenProgressCompletionData *completionData = new UIFullscreenProgressCompletionData();
						completionData->bShowBackground=TRUE;
						completionData->bShowLogo=TRUE;
						completionData->iPad=DEFAULT_XUI_MENU_USER;
						completionData->type = e_ProgressCompletion_NavigateToHomeMenu;
						loadingParams->completionData = completionData;

						ui.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_FullscreenProgress, loadingParams);
					}
				}
				break;
			case eAppAction_PrimaryPlayerSignedOutReturned_Menus:
				SetAction(i,eAppAction_Idle);
				// set the state back to pre-game
				ProfileManager.ResetProfileProcessState();
				// clear the save device
				StorageManager.SetSaveDeviceSelected(i,false);

				ui.UpdatePlayerBasePositions();
				// there are multiple layers in the help menu, so a navigate back isn't enough
				ui.NavigateToHomeMenu();

				break;
			case eAppAction_EthernetDisconnectedReturned_Menus:
				SetAction(i,eAppAction_Idle);
				// set the state back to pre-game
				ProfileManager.ResetProfileProcessState();

				ui.UpdatePlayerBasePositions();

				// there are multiple layers in the help menu, so a navigate back isn't enough
				ui.NavigateToHomeMenu();

				break;

			case eAppAction_TrialOver:
				{
					SetAction(i,eAppAction_Idle);
					UINT uiIDA[2];
					uiIDA[0]=IDS_UNLOCK_TITLE;
					uiIDA[1]=IDS_EXIT_GAME;

					ui.RequestErrorMessage(IDS_TRIALOVER_TITLE, IDS_TRIALOVER_TEXT, uiIDA, 2, i,&CMinecraftApp::TrialOverReturned,this);
				}
				break;

				// INVITES
			case eAppAction_DashboardTrialJoinFromInvite:
				{
					TelemetryManager->RecordUpsellPresented(i, eSen_UpsellID_Full_Version_Of_Game, app.m_dwOfferID);

					SetAction(i,eAppAction_Idle);
					UINT uiIDA[2];
					uiIDA[0]=IDS_CONFIRM_OK;
					uiIDA[1]=IDS_CONFIRM_CANCEL;

					ui.RequestErrorMessage(IDS_UNLOCK_TITLE, IDS_UNLOCK_ACCEPT_INVITE, uiIDA, 2, i,&CMinecraftApp::UnlockFullInviteReturned,this);
				}
				break;
			case eAppAction_ExitAndJoinFromInvite:
				{
					UINT uiIDA[3];

					SetAction(i,eAppAction_Idle);
					// Check the player really wants to do this

#if defined(_XBOX_ONE) || defined(__ORBIS__)
					// Show save option is saves ARE disabled
					if(ProfileManager.IsFullVersion() && StorageManager.GetSaveDisabled() && i==ProfileManager.GetPrimaryPad() && g_NetworkManager.IsHost() && GetGameStarted() )
					{
						uiIDA[0]=IDS_CONFIRM_CANCEL;
						uiIDA[1]=IDS_EXIT_GAME_SAVE;
						uiIDA[2]=IDS_EXIT_GAME_NO_SAVE;

						ui.RequestAlertMessage(IDS_EXIT_GAME, IDS_CONFIRM_LEAVE_VIA_INVITE, uiIDA, 3, i,&CMinecraftApp::ExitAndJoinFromInviteSaveDialogReturned,this);
					}
					else
#else
					if(ProfileManager.IsFullVersion() && !StorageManager.GetSaveDisabled() && i==ProfileManager.GetPrimaryPad() && g_NetworkManager.IsHost() && GetGameStarted() )
					{
						uiIDA[0]=IDS_CONFIRM_CANCEL;
						uiIDA[1]=IDS_EXIT_GAME_SAVE;
						uiIDA[2]=IDS_EXIT_GAME_NO_SAVE;

						ui.RequestAlertMessage(IDS_EXIT_GAME, IDS_CONFIRM_LEAVE_VIA_INVITE, uiIDA, 3, i,&CMinecraftApp::ExitAndJoinFromInviteSaveDialogReturned,this);
					}
					else
#endif
					{
						if(!ProfileManager.IsFullVersion())
						{
							TelemetryManager->RecordUpsellPresented(i, eSen_UpsellID_Full_Version_Of_Game, app.m_dwOfferID);

							// upsell
							uiIDA[0]=IDS_CONFIRM_OK;
							uiIDA[1]=IDS_CONFIRM_CANCEL;
							ui.RequestErrorMessage(IDS_UNLOCK_TITLE, IDS_UNLOCK_ACCEPT_INVITE, uiIDA, 2, i,&CMinecraftApp::UnlockFullInviteReturned,this);
						}
						else
						{
							uiIDA[0]=IDS_CONFIRM_CANCEL;
							uiIDA[1]=IDS_CONFIRM_OK;
							ui.RequestAlertMessage(IDS_EXIT_GAME, IDS_CONFIRM_LEAVE_VIA_INVITE, uiIDA, 2,i,&CMinecraftApp::ExitAndJoinFromInvite,this);
						}
					}
				}
				break;
			case eAppAction_ExitAndJoinFromInviteConfirmed:
				{
					SetAction(i,eAppAction_Idle);

					pMinecraft->gui->clearMessages();

					// turn off the gamertags in splitscreen for the primary player, since they are about to be made fullscreen
					ui.HideAllGameUIElements();

					// Stop app running
					SetGameStarted(false);

					ui.CloseAllPlayersScenes();

					// 4J Stu - Fix for #12368 - Crash: Game crashes when saving then exiting and selecting to save
					for(unsigned int idx = 0; idx < XUSER_MAX_COUNT; ++idx)
					{
#ifdef _XBOX
						app.TutorialSceneNavigateBack(idx,true);
#endif

						// 4J Stu - Fix for #13257 - CRASH: Gameplay: Title crashed after exiting the tutorial
						// It doesn't matter if they were in the tutorial already
						pMinecraft->playerLeftTutorial( idx );
					}

					// 4J-PB - may have been using a texture pack with audio , so clean up anything texture pack related here

					// unload any texture pack audio
					// if there is audio in use, clear out the audio, and unmount the pack
					TexturePack *pTexPack=Minecraft::GetInstance()->skins->getSelected();
					DLCTexturePack *pDLCTexPack=nullptr;

					if(pTexPack->hasAudio())
					{
						// get the dlc texture pack, and store it
						pDLCTexPack=static_cast<DLCTexturePack *>(pTexPack);
					}

					// change to the default texture pack
					pMinecraft->skins->selectTexturePackById(TexturePackRepository::DEFAULT_TEXTURE_PACK_ID);

					if(pTexPack->hasAudio())
					{
						// need to stop the streaming audio - by playing streaming audio from the default texture pack now
						// reset the streaming sounds back to the normal ones
#ifndef _XBOX
						pMinecraft->soundEngine->SetStreamingSounds(eStream_Overworld_Calm1,eStream_Overworld_piano3,
							eStream_Nether1,eStream_Nether4,
							eStream_end_dragon,eStream_end_end,
							eStream_CD_1, eStream_Overworld_Creative1, eStream_Overworld_Creative6, eStream_Overworld_Menu1, eStream_Overworld_Menu4);
#endif
						pMinecraft->soundEngine->playStreaming(L"", 0, 0, 0, 1, 1);

#ifdef _XBOX
						if(pDLCTexPack->m_pStreamedWaveBank!=nullptr)
						{
							pDLCTexPack->m_pStreamedWaveBank->Destroy();
						}
						if(pDLCTexPack->m_pSoundBank!=nullptr)
						{
							pDLCTexPack->m_pSoundBank->Destroy();
						}
#endif
#ifdef _DURANGO
						DWORD result = StorageManager.UnmountInstalledDLC(L"TPACK");
#else
						DWORD result = StorageManager.UnmountInstalledDLC("TPACK");
#endif
						app.DebugPrintf("Unmount result is %d\n",result);
					}

#ifdef _XBOX_ONE
					// 4J Stu - It's possible that we can sign in/remove players between the mask initially being set and this point
					m_InviteData.dwLocalUsersMask = 0;
					for(unsigned int index = 0; index < XUSER_MAX_COUNT; ++index)
					{
						if(ProfileManager.IsSignedIn(index) )
                        {
                            if (index == i || pMinecraft->localplayers[index] != nullptr)
                            {
                                m_InviteData.dwLocalUsersMask |= g_NetworkManager.GetLocalPlayerMask(index);
                            }
						}
					}
#endif

                    LoadingInputParams *loadingParams = new LoadingInputParams();
                    loadingParams->func = &CGameNetworkManager::ExitAndJoinFromInviteThreadProc;
                    loadingParams->lpParam = static_cast<LPVOID>(&m_InviteData);

					UIFullscreenProgressCompletionData *completionData = new UIFullscreenProgressCompletionData();
					completionData->bShowBackground=TRUE;
					completionData->bShowLogo=TRUE;
					completionData->iPad=DEFAULT_XUI_MENU_USER;
					completionData->type = e_ProgressCompletion_NoAction;
					loadingParams->completionData = completionData;

					ui.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_FullscreenProgress, loadingParams);
				}

				break;
			case eAppAction_JoinFromInvite:
				{
					SetAction(i,eAppAction_Idle);

					// 4J Stu - Move this state block from CPlatformNetworkManager::ExitAndJoinFromInviteThreadProc, as g_NetworkManager.JoinGameFromInviteInfo ultimately can call NavigateToScene,
					/// and we should only be calling that from the main thread
					app.SetTutorialMode( false );

					g_NetworkManager.SetLocalGame(false);

					JoinFromInviteData *inviteData = static_cast<JoinFromInviteData *>(param);
					// 4J-PB - clear any previous connection errors
					Minecraft::GetInstance()->clearConnectionFailed();

					app.DebugPrintf( "Changing Primary Pad on an invite accept - pad was %d, and is now %d\n", ProfileManager.GetPrimaryPad(), inviteData->dwUserIndex );
					ProfileManager.SetLockedProfile(inviteData->dwUserIndex);
					ProfileManager.SetPrimaryPad(inviteData->dwUserIndex);

#ifdef _XBOX_ONE
					// 4J Stu - If a player is signed in (i.e. locked) but not in the mask, unlock them
					for(unsigned int index = 0; index < XUSER_MAX_COUNT; ++index)
					{
						if( index != inviteData->dwUserIndex && ProfileManager.IsSignedIn(index) )
						{
							if( (m_InviteData.dwLocalUsersMask & g_NetworkManager.GetLocalPlayerMask( index ) ) == 0 )
							{
								ProfileManager.RemoveGamepadFromGame(index);
							}
						}
					}
#endif

					// change the minecraft player name
					Minecraft::GetInstance()->user->name = convStringToWstring( ProfileManager.GetGamertag(ProfileManager.GetPrimaryPad()));

					bool success = g_NetworkManager.JoinGameFromInviteInfo(
						inviteData->dwUserIndex, // dwUserIndex
						inviteData->dwLocalUsersMask,   // dwUserMask
						inviteData->pInviteInfo );      // pInviteInfo

					if( !success )
					{
						app.DebugPrintf( "Failed joining game from invite\n" );
						//return hr;

						// 4J Stu - Copied this from XUI_FullScreenProgress to properly handle the fail case, as the thread will no longer be failing
						UINT uiIDA[1];
						uiIDA[0]=IDS_CONFIRM_OK;
						ui.RequestErrorMessage( IDS_CONNECTION_FAILED, IDS_CONNECTION_LOST_SERVER, uiIDA,1,ProfileManager.GetPrimaryPad());

						ui.NavigateToHomeMenu();
						ui.UpdatePlayerBasePositions();
					}
				}
				break;
			case eAppAction_ChangeSessionType:
				{
					// If we are not in gameplay yet, then wait until the server is setup before changing the session type
					if( g_NetworkManager.IsInGameplay() )
					{
						// This kicks off a thread that waits for the server to end, then closes the current session, starts a new one and joins the local players into it

						SetAction(i,eAppAction_Idle);

						if( !GetChangingSessionType() && !g_NetworkManager.IsLocalGame() )
						{
							SetGameStarted(false);
							SetChangingSessionType(true);
							SetReallyChangingSessionType(true);

							// turn off the gamertags in splitscreen for the primary player, since they are about to be made fullscreen
							ui.HideAllGameUIElements();

							if( !ui.IsSceneInStack( ProfileManager.GetPrimaryPad(), eUIScene_EndPoem ) )
							{
								ui.CloseAllPlayersScenes();
							}
							ui.ShowOtherPlayersBaseScene(ProfileManager.GetPrimaryPad(), true);

							// Remove this line to fix:
							// #49084 - TU5: Code: Gameplay: The title crashes every time client navigates to 'Play game' menu and loads/creates new game after a "Connection to Xbox LIVE was lost" message has appeared.
							//app.NavigateToScene(0,eUIScene_Main);

							LoadingInputParams *loadingParams = new LoadingInputParams();
							loadingParams->func = &CGameNetworkManager::ChangeSessionTypeThreadProc;
							loadingParams->lpParam = nullptr;

							UIFullscreenProgressCompletionData *completionData = new UIFullscreenProgressCompletionData();
#ifdef __PS3__
							completionData->bRequiresUserAction=FALSE;
#else
							completionData->bRequiresUserAction=TRUE;
#endif
							completionData->bShowBackground=TRUE;
							completionData->bShowLogo=TRUE;
							completionData->iPad=DEFAULT_XUI_MENU_USER;
							if( ui.IsSceneInStack( ProfileManager.GetPrimaryPad(), eUIScene_EndPoem ) )
							{
								completionData->type = e_ProgressCompletion_NavigateBackToScene;
								completionData->scene = eUIScene_EndPoem;
							}
							else
							{
								completionData->type = e_ProgressCompletion_CloseAllPlayersUIScenes;
							}
							loadingParams->completionData = completionData;

							ui.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_FullscreenProgress, loadingParams);
						}
					}
					else if( g_NetworkManager.IsLeavingGame() )
					{
						// If we are leaving the game, then ignore the state change
						SetAction(i,eAppAction_Idle);
					}
#if 0
					// 4J-HG - Took this out since ChangeSessionType is only set in two places (both in EthernetDisconnected) and this case is handled there, plus this breaks
					// this if statements original purpose (to allow us to wait for IsInGameplay before actioning switching to offline

					// QNet must do this kind of thing automatically by itself, but on PS3 at least, we need the disconnection to definitely end up with us out of the game one way or another,
					// and the other two cases above don't catch the case where we are just starting the game and get a disconnection during the loading/creation
					else
					{
						if( g_NetworkManager.IsInSession() )
						{
							g_NetworkManager._LeaveGame();
						}
					}
#endif
				}
				break;
			case eAppAction_SetDefaultOptions:
				SetAction(i,eAppAction_Idle);
#if ( defined __PS3__ || defined __ORBIS__ || defined _DURANGO || defined __PSVITA__)
				SetDefaultOptions((C4JStorage::PROFILESETTINGS *)param,i);
#else
                SetDefaultOptions(static_cast<C_4JProfile::PROFILESETTINGS *>(param), i);
#endif

				// if the profile data has been changed, then force a profile write
				// It seems we're allowed to break the 5 minute rule if it's the result of a user action
				CheckGameSettingsChanged(true,i);

				break;

			case eAppAction_RemoteServerSave:
				{
					// If the remote server save has already finished, don't complete the action
					if (GetGameStarted())
					{
						SetAction(ProfileManager.GetPrimaryPad(), eAppAction_Idle);
						break;
					}

					SetAction(i,eAppAction_WaitRemoteServerSaveComplete);

					for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
					{
						ui.CloseUIScenes(i, true);
					}

					// turn off the gamertags in splitscreen for the primary player, since they are about to be made fullscreen
					ui.HideAllGameUIElements();

					LoadingInputParams *loadingParams = new LoadingInputParams();
					loadingParams->func = &CMinecraftApp::RemoteSaveThreadProc;
					loadingParams->lpParam = nullptr;			

					UIFullscreenProgressCompletionData *completionData = new UIFullscreenProgressCompletionData();
					completionData->bRequiresUserAction=FALSE;
					completionData->bShowBackground=TRUE;
					completionData->bShowLogo=TRUE;
					completionData->iPad=DEFAULT_XUI_MENU_USER;
					if( ui.IsSceneInStack( ProfileManager.GetPrimaryPad(), eUIScene_EndPoem ) )
					{
						completionData->type = e_ProgressCompletion_NavigateBackToScene;
						completionData->scene = eUIScene_EndPoem;
					}
					else
					{
						completionData->type = e_ProgressCompletion_CloseAllPlayersUIScenes;
					}
					loadingParams->completionData = completionData;

					loadingParams->cancelFunc = &CMinecraftApp::ExitGameFromRemoteSave;
					loadingParams->cancelText = IDS_TOOLTIPS_EXIT;

					ui.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_FullscreenProgress, loadingParams);
				}
				break;
			case eAppAction_WaitRemoteServerSaveComplete:
				// Do nothing
				break;
			case eAppAction_FailedToJoinNoPrivileges:
				{
					UINT uiIDA[1];
					uiIDA[0]=IDS_CONFIRM_OK;
					C4JStorage::EMessageResult result = ui.RequestErrorMessage( IDS_NO_MULTIPLAYER_PRIVILEGE_TITLE, IDS_NO_MULTIPLAYER_PRIVILEGE_JOIN_TEXT, uiIDA,1,ProfileManager.GetPrimaryPad());
					if(result != C4JStorage::EMessage_Busy) SetAction(i,eAppAction_Idle);
				}
				break;
			case eAppAction_ProfileReadError:
				// Return player to the main menu - code largely copied from that for handling
				// eAppAction_PrimaryPlayerSignedOut, although I don't think we should have got as
				// far as needing to halt the server, or running the game, before returning to the menu
				if(!app.GetGameStarted()) MinecraftServer::HaltServer(true);

				if( g_NetworkManager.IsInSession() )
				{
					app.SetAction(i,eAppAction_PrimaryPlayerSignedOutReturned);
				}
				else
				{
					app.SetAction(i,eAppAction_PrimaryPlayerSignedOutReturned_Menus);
					MinecraftServer::resetFlags();
				}
				break;

			case eAppAction_BanLevel:
				{
					// It's possible that this state can get set after the game has been exited (e.g. by network disconnection) so we can't ban the level at that point
					if(g_NetworkManager.IsInGameplay() && !g_NetworkManager.IsLeavingGame())
					{
						TelemetryManager->RecordBanLevel(i);

#if defined _XBOX
						INetworkPlayer *pHost=g_NetworkManager.GetHostPlayer();
						// write the level to the banned level list, and exit the world
						AddLevelToBannedLevelList(i,((NetworkPlayerXbox *)pHost)->GetUID(),GetUniqueMapName(),true);
#elif defined _XBOX_ONE
						INetworkPlayer *pHost=g_NetworkManager.GetHostPlayer();
						AddLevelToBannedLevelList(i,pHost->GetUID(),GetUniqueMapName(),true);
#endif
						// primary player would exit the world, secondary would exit the player
						if(ProfileManager.GetPrimaryPad()==i)
						{
							SetAction(i,eAppAction_ExitWorld);
						}
						else
						{
							SetAction(i,eAppAction_ExitPlayer);
						}
					}
				}
				break;
			case eAppAction_LevelInBanLevelList:
				{
					UINT uiIDA[2];
					uiIDA[0]=IDS_BUTTON_REMOVE_FROM_BAN_LIST;
					uiIDA[1]=IDS_EXIT_GAME;

					// pass in the gamertag format string
					WCHAR wchFormat[40];
					INetworkPlayer *player = g_NetworkManager.GetLocalPlayerByUserIndex(i);

					// If not the primary player, but the primary player has banned this level and decided not to unban
					// then we may have left the game by now
					if(player)
					{
						swprintf(wchFormat, 40, L"%ls\n\n%%ls",player->GetOnlineName());

						C4JStorage::EMessageResult result = ui.RequestErrorMessage( IDS_BANNED_LEVEL_TITLE, IDS_PLAYER_BANNED_LEVEL, uiIDA,2,i,&CMinecraftApp::BannedLevelDialogReturned,this, wchFormat);
						if(result != C4JStorage::EMessage_Busy) SetAction(i,eAppAction_Idle);
					}
					else
					{
						SetAction(i,eAppAction_Idle);
					}
				}
				break;
			case eAppAction_DebugText:
				// launch the xui for text entry
				{
#ifdef _XBOX
					CScene_TextEntry::XuiTextInputParams *pDebugTextParams= new CScene_TextEntry::XuiTextInputParams;
					pDebugTextParams->iPad=i;
					pDebugTextParams->wch=(WCHAR)param;

					app.NavigateToScene(i,eUIScene_TextEntry,pDebugTextParams);
#endif
					SetAction(i,eAppAction_Idle);
				}
				break;

			case eAppAction_ReloadTexturePack:
				{
					SetAction(i,eAppAction_Idle);
					Minecraft *pMinecraft = Minecraft::GetInstance();
					pMinecraft->textures->reloadAll();
					pMinecraft->skins->updateUI();

					if(!pMinecraft->skins->isUsingDefaultSkin())
					{
						TexturePack *pTexturePack = pMinecraft->skins->getSelected();

						DLCPack *pDLCPack=pTexturePack->getDLCPack();

						bool purchased = false;
						// do we have a license?
						if(pDLCPack && pDLCPack->hasPurchasedFile( DLCManager::e_DLCType_Texture, L"" ))
						{
							purchased = true;
						}
#ifdef _XBOX
						TelemetryManager->RecordTexturePackLoaded(i, pTexturePack->getId(), purchased?1:0);
#endif
					}

					// 4J-PB  - If the texture pack has audio, we need to switch to this
					if(pMinecraft->skins->getSelected()->hasAudio())
					{
						Minecraft::GetInstance()->soundEngine->playStreaming(L"", 0, 0, 0, 1, 1);
					}
				}
				break;

			case eAppAction_ReloadFont:
				{
#ifndef _XBOX
					app.DebugPrintf(
						"[Consoles_App] eAppAction_ReloadFont, ingame='%s'.\n",
						app.GetGameStarted() ? "Yes" : "No" );

					SetAction(i,eAppAction_Idle);

					ui.SetTooltips(i, -1);

					ui.ReloadSkin();
					ui.StartReloadSkinThread();

					ui.setCleanupOnReload();
#endif
				}
				break;

			case eAppAction_TexturePackRequired:
				{
#if defined __PS3__ || defined __ORBIS__ || defined __PSVITA__
					UINT uiIDA[2];
					uiIDA[0]=IDS_TEXTUREPACK_FULLVERSION;
					uiIDA[1]=IDS_CONFIRM_CANCEL;			// let them continue without the texture pack here (as this is only really for r
					// Give the player a warning about the texture pack missing
					ui.RequestErrorMessage(IDS_DLC_TEXTUREPACK_NOT_PRESENT_TITLE, IDS_DLC_TEXTUREPACK_NOT_PRESENT, uiIDA, 2, ProfileManager.GetPrimaryPad(),&CMinecraftApp::TexturePackDialogReturned,this);
					SetAction(i,eAppAction_Idle);
#else
#ifdef _XBOX
					ULONGLONG ullOfferID_Full;
					app.GetDLCFullOfferIDForPackID(app.GetRequiredTexturePackID(),&ullOfferID_Full);

					TelemetryManager->RecordUpsellPresented(ProfileManager.GetPrimaryPad(), eSet_UpsellID_Texture_DLC, ullOfferID_Full & 0xFFFFFFFF);
#endif
					UINT uiIDA[2];

					uiIDA[0]=IDS_TEXTUREPACK_FULLVERSION;
					uiIDA[1]=IDS_TEXTURE_PACK_TRIALVERSION;

					// Give the player a warning about the texture pack missing
					ui.RequestErrorMessage(IDS_DLC_TEXTUREPACK_NOT_PRESENT_TITLE, IDS_DLC_TEXTUREPACK_NOT_PRESENT, uiIDA, 2, ProfileManager.GetPrimaryPad(),&CMinecraftApp::TexturePackDialogReturned,this);
					SetAction(i,eAppAction_Idle);
#endif
				}

				break;
			}
		}

		// Any TMS actions?

		eTMS = app.GetTMSAction(i);

		if(eTMS!=eTMSAction_Idle)
		{
			switch(eTMS)
			{
				// TMS++ actions
			case eTMSAction_TMSPP_RetrieveFiles_CreateLoad_SignInReturned:
			case eTMSAction_TMSPP_RetrieveFiles_RunPlayGame:
#ifdef _XBOX
				app.TMSPP_SetTitleGroupID(GROUP_ID);
				SetTMSAction(i,eTMSAction_TMSPP_GlobalFileList);
#elif defined _XBOX_ONE
				SetTMSAction(i,eTMSAction_TMSPP_GlobalFileList_Waiting);
				app.TMSPP_RetrieveFileList(i,C4JStorage::eGlobalStorage_Title,eTMSAction_TMSPP_UserFileList);
#else
				SetTMSAction(i,eTMSAction_TMSPP_UserFileList);
#endif
				break;

#ifdef _XBOX
			case eTMSAction_TMSPP_GlobalFileList:
				SetTMSAction(i,eTMSAction_TMSPP_GlobalFileList_Waiting);
				app.TMSPP_RetrieveFileList(i,C4JStorage::eGlobalStorage_Title,"\\",eTMSAction_TMSPP_UserFileList);
				break;
#endif
			case eTMSAction_TMSPP_UserFileList:
				// retrieve the file list first
#if defined _XBOX
				SetTMSAction(i,eTMSAction_TMSPP_UserFileList_Waiting);
				app.TMSPP_RetrieveFileList(i,C4JStorage::eGlobalStorage_TitleUser,"\\",eTMSAction_TMSPP_XUIDSFile);
#elif defined _XBOX_ONE
				SetTMSAction(i,eTMSAction_TMSPP_UserFileList_Waiting);
				app.TMSPP_RetrieveFileList(i,C4JStorage::eGlobalStorage_TitleUser,eTMSAction_TMSPP_DLCFile);
#else
				SetTMSAction(i,eTMSAction_TMSPP_XUIDSFile);
#endif
				break;
			case eTMSAction_TMSPP_XUIDSFile:
#ifdef _XBOX
				SetTMSAction(i,eTMSAction_TMSPP_XUIDSFile_Waiting);
				// pass in the next app action on the call or callback completing
				app.TMSPP_ReadXuidsFile(i,eTMSAction_TMSPP_DLCFile);
#else
				SetTMSAction(i,eTMSAction_TMSPP_DLCFile);
#endif

				break;
			case eTMSAction_TMSPP_DLCFile:
#if defined _XBOX || defined _XBOX_ONE
				SetTMSAction(i,eTMSAction_TMSPP_DLCFile_Waiting);
				// pass in the next app action on the call or callback completing
				app.TMSPP_ReadDLCFile(i,eTMSAction_TMSPP_BannedListFile);
#else
				SetTMSAction(i,eTMSAction_TMSPP_BannedListFile);
#endif
				break;
			case eTMSAction_TMSPP_BannedListFile:
				// If we have one in TMSPP, then we can assume we can ignore TMS
#if defined _XBOX
				SetTMSAction(i,eTMSAction_TMSPP_BannedListFile_Waiting);
				// pass in the next app action on the call or callback completing
				if(app.TMSPP_ReadBannedList(i,eTMSAction_TMS_RetrieveFiles_Complete)==false)
				{
					// we don't have a banned list in TMSPP, so we should check TMS
					app.ReadBannedList(i, eTMSAction_TMS_RetrieveFiles_Complete,true);
				}
#elif defined _XBOX_ONE
				SetTMSAction(i,eTMSAction_TMSPP_BannedListFile_Waiting);
				// pass in the next app action on the call or callback completing
				app.TMSPP_ReadBannedList(i,eTMSAction_TMS_RetrieveFiles_Complete);

#else
				SetTMSAction(i,eTMSAction_TMS_RetrieveFiles_Complete);
#endif
				break;

				// SPECIAL CASE - where the user goes directly in to Help & Options from the main menu
			case eTMSAction_TMSPP_RetrieveFiles_HelpAndOptions:
			case eTMSAction_TMSPP_RetrieveFiles_DLCMain:
				// retrieve the file list first
#if defined _XBOX
				// pass in the next app action on the call or callback completing
				SetTMSAction(i,eTMSAction_TMSPP_XUIDSFile_Waiting);
				app.TMSPP_RetrieveFileList(i,C4JStorage::eGlobalStorage_Title,"\\",eTMSAction_TMSPP_DLCFileOnly);
#elif defined _XBOX_ONE
				SetTMSAction(i,eTMSAction_TMSPP_GlobalFileList_Waiting);
				app.TMSPP_RetrieveFileList(i,C4JStorage::eGlobalStorage_Title,eTMSAction_TMSPP_RetrieveUserFilelist_DLCFileOnly);
#else
				SetTMSAction(i,eTMSAction_TMSPP_DLCFileOnly);
#endif
				break;
			case eTMSAction_TMSPP_RetrieveUserFilelist_DLCFileOnly:
#if defined _XBOX
				SetTMSAction(i,eTMSAction_TMSPP_UserFileList_Waiting);
				app.TMSPP_RetrieveFileList(i,C4JStorage::eGlobalStorage_TitleUser,"\\",eTMSAction_TMSPP_XUIDSFile);
#elif defined _XBOX_ONE
				//StorageManager.TMSPP_DeleteFile(i,C4JStorage::eGlobalStorage_TitleUser,C4JStorage::TMS_FILETYPE_BINARY,L"TP06.png",nullptr,nullptr, 0);
				SetTMSAction(i,eTMSAction_TMSPP_UserFileList_Waiting);
				app.TMSPP_RetrieveFileList(i,C4JStorage::eGlobalStorage_TitleUser,eTMSAction_TMSPP_DLCFileOnly);
#else
				SetTMSAction(i,eTMSAction_TMSPP_DLCFileOnly);
#endif

				break;

			case eTMSAction_TMSPP_DLCFileOnly:
#if defined _XBOX || defined _XBOX_ONE
				SetTMSAction(i,eTMSAction_TMSPP_DLCFile_Waiting);
				// pass in the next app action on the call or callback completing
				app.TMSPP_ReadDLCFile(i,eTMSAction_TMSPP_RetrieveFiles_Complete);
#else
				SetTMSAction(i,eTMSAction_TMSPP_RetrieveFiles_Complete);
#endif
				break;


			case eTMSAction_TMSPP_RetrieveFiles_Complete:
				SetTMSAction(i,eTMSAction_Idle);
				break;


				// TMS files
				/*			case eTMSAction_TMS_RetrieveFiles_CreateLoad_SignInReturned:
				case eTMSAction_TMS_RetrieveFiles_RunPlayGame:
				#ifdef _XBOX
				SetTMSAction(i,eTMSAction_TMS_XUIDSFile_Waiting);
				// pass in the next app action on the call or callback completing
				app.ReadXuidsFileFromTMS(i,eTMSAction_TMS_DLCFile,true);
				#else
				SetTMSAction(i,eTMSAction_TMS_DLCFile);
				#endif
				break;

				case eTMSAction_TMS_DLCFile:
				#ifdef _XBOX
				SetTMSAction(i,eTMSAction_TMS_DLCFile_Waiting);
				// pass in the next app action on the call or callback completing
				app.ReadDLCFileFromTMS(i,eTMSAction_TMS_BannedListFile,true);
				#else
				SetTMSAction(i,eTMSAction_TMS_BannedListFile);
				#endif

				break;

				case eTMSAction_TMS_RetrieveFiles_HelpAndOptions:
				case eTMSAction_TMS_RetrieveFiles_DLCMain:
				#ifdef _XBOX
				SetTMSAction(i,eTMSAction_TMS_DLCFile_Waiting);
				// pass in the next app action on the call or callback completing
				app.ReadDLCFileFromTMS(i,eTMSAction_Idle,true);
				#else
				SetTMSAction(i,eTMSAction_Idle);
				#endif

				break;
				case eTMSAction_TMS_BannedListFile:
				#ifdef _XBOX
				SetTMSAction(i,eTMSAction_TMS_BannedListFile_Waiting);
				// pass in the next app action on the call or callback completing
				app.ReadBannedList(i, eTMSAction_TMS_RetrieveFiles_Complete,true);
				#else
				SetTMSAction(i,eTMSAction_TMS_RetrieveFiles_Complete);
				#endif

				break;

				*/
			case eTMSAction_TMS_RetrieveFiles_Complete:
				SetTMSAction(i,eTMSAction_Idle);
				// 				if(StorageManager.SetSaveDevice(&CScene_Main::DeviceSelectReturned,pClass))
				// 				{
				// 					// save device already selected
				// 					// ensure we've applied this player's settings
				// 					app.ApplyGameSettingsChanged(ProfileManager.GetPrimaryPad());
				// 					app.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_MultiGameJoinLoad);
				// 				}
				break;
			}
		}

	}
}

int CMinecraftApp::BannedLevelDialogReturned(void *pParam,int iPad,const C4JStorage::EMessageResult result)
{
	CMinecraftApp* pApp = static_cast<CMinecraftApp *>(pParam);
	//Minecraft *pMinecraft=Minecraft::GetInstance();

	if(result==C4JStorage::EMessage_ResultAccept)
	{
#if defined _XBOX || defined _XBOX_ONE
		INetworkPlayer *pHost = g_NetworkManager.GetHostPlayer();
		// unban the level
		if (pHost != nullptr)
		{
#if defined _XBOX
			pApp->RemoveLevelFromBannedLevelList(iPad,((NetworkPlayerXbox *)pHost)->GetUID(),pApp->GetUniqueMapName());
#else
			pApp->RemoveLevelFromBannedLevelList(iPad,pHost->GetUID(),pApp->GetUniqueMapName());
#endif
		}
#endif
	}
	else
	{
		if( iPad == ProfileManager.GetPrimaryPad() )
		{
			pApp->SetAction(iPad,eAppAction_ExitWorld);
		}
		else
		{
			pApp->SetAction(iPad,eAppAction_ExitPlayer);
		}
	}

	return 0;
}

void CMinecraftApp::loadMediaArchive()
{
	wstring mediapath = L"";

#ifdef __PS3__
	mediapath = L"Common\\Media\\MediaPS3.arc";
#elif _WINDOWS64
	mediapath = L"Common\\Media\\MediaWindows64.arc";
#elif __ORBIS__
	mediapath = L"Common\\Media\\MediaOrbis.arc";
#elif _DURANGO
	mediapath = L"Common\\Media\\MediaDurango.arc";
#elif __PSVITA__
	mediapath = L"Common\\Media\\MediaPSVita.arc";
#endif

	if (!mediapath.empty())
	{
		m_mediaArchive = new ArchiveFile( File(mediapath) );
	}
#if 0
	string path = "Common\\media.arc";
	HANDLE hFile = CreateFile(	path.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ,
		nullptr,
		OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN,
		nullptr );

	if( hFile != INVALID_HANDLE_VALUE )
	{
		File fileHelper(convStringToWstring(path));
		DWORD dwFileSize = fileHelper.length();

		// Initialize memory.
		PBYTE m_fBody = new BYTE[ dwFileSize ];
		ZeroMemory(m_fBody, dwFileSize);

		DWORD m_fSize = 0;
		BOOL hr = ReadFile(	hFile,
			m_fBody,
			dwFileSize,
			&m_fSize,
			nullptr	);

		assert( m_fSize == dwFileSize );

		CloseHandle( hFile );

		m_mediaArchive = new ArchiveFile(m_fBody, m_fSize);
	}
	else
	{
		assert( false );
		// AHHHHHHHHHHHH
		m_mediaArchive = nullptr;
	}
#endif
}

void CMinecraftApp::loadStringTable()
{
#ifndef _XBOX

	if(m_stringTable!=nullptr)
	{
		// we need to unload the current string table, this is a reload
		delete m_stringTable;
	}
	wstring localisationFile = L"languages.loc";
	if (m_mediaArchive->hasFile(localisationFile))
	{
		byteArray locFile = m_mediaArchive->getFile(localisationFile);
		m_stringTable = new StringTable(locFile.data, locFile.length);
		delete locFile.data;
	}
	else
	{
		m_stringTable = nullptr;
		assert(false);
		// AHHHHHHHHH.
	}
#endif
}

int CMinecraftApp::PrimaryPlayerSignedOutReturned(void *pParam,int iPad,const C4JStorage::EMessageResult)
{
	//CMinecraftApp* pApp = (CMinecraftApp*)pParam;
	//Minecraft *pMinecraft=Minecraft::GetInstance();

	// if the player is null, we're in the menus
	//if(Minecraft::GetInstance()->player!=nullptr)

	// We always create a session before kicking of any of the game code, so even though we may still be joining/creating a game
	// at this point we want to handle it differently from just being in a menu
	if( g_NetworkManager.IsInSession() )
	{
		app.SetAction(iPad,eAppAction_PrimaryPlayerSignedOutReturned);
	}
	else
	{
		app.SetAction(iPad,eAppAction_PrimaryPlayerSignedOutReturned_Menus);
	}
	return 0;
}

int CMinecraftApp::EthernetDisconnectReturned(void *pParam,int iPad,const C4JStorage::EMessageResult)
{
	//CMinecraftApp* pApp = (CMinecraftApp*)pParam;
	Minecraft *pMinecraft=Minecraft::GetInstance();

	// if the player is null, we're in the menus
    if (Minecraft::GetInstance()->player != nullptr)
    {
        app.SetAction(pMinecraft->player->GetXboxPad(), eAppAction_EthernetDisconnectedReturned);
    }
	else
	{
		//  4J-PB - turn off the PSN store icon just in case this happened when we were in one of the DLC menus
#if defined __ORBIS__ || defined __PSVITA__
		app.GetCommerce()->HidePsStoreIcon();
#endif
		app.SetAction(iPad,eAppAction_EthernetDisconnectedReturned_Menus);
	}
	return 0;
}

int CMinecraftApp::SignoutExitWorldThreadProc( void* lpParameter )
{

	// Share AABB & Vec3 pools with default (main thread) - should be ok as long as we don't tick the main thread whilst this thread is running
	AABB::UseDefaultThreadStorage();
	Vec3::UseDefaultThreadStorage();
	Compression::UseDefaultThreadStorage();

	//app.SetGameStarted(false);

	Minecraft *pMinecraft=Minecraft::GetInstance();

	int exitReasonStringId = -1;

	bool saveStats = false;
	if (pMinecraft->isClientSide() || g_NetworkManager.IsInSession() )
	{
		if(lpParameter != nullptr )
		{
			switch( app.GetDisconnectReason() )
			{
			case DisconnectPacket::eDisconnect_Kicked:
				exitReasonStringId = IDS_DISCONNECTED_KICKED;
				break;
			case DisconnectPacket::eDisconnect_NoUGC_AllLocal:
				exitReasonStringId = IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_ALL_LOCAL;
				break;
			case DisconnectPacket::eDisconnect_NoUGC_Single_Local:
				exitReasonStringId = IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_SINGLE_LOCAL;
				break;
#ifdef _XBOX
			case DisconnectPacket::eDisconnect_NoUGC_Remote:
				exitReasonStringId = IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_REMOTE;
				break;
#endif
			case DisconnectPacket::eDisconnect_NoFlying:
				exitReasonStringId = IDS_DISCONNECTED_FLYING;
				break;
			case DisconnectPacket::eDisconnect_OutdatedServer:
				exitReasonStringId = IDS_DISCONNECTED_SERVER_OLD;
				break;
			case DisconnectPacket::eDisconnect_OutdatedClient:
				exitReasonStringId = IDS_DISCONNECTED_CLIENT_OLD;
				break;
			default:
				exitReasonStringId = IDS_DISCONNECTED;
			}
			pMinecraft->progressRenderer->progressStartNoAbort( exitReasonStringId );
			// 4J - Force a disconnection, this handles the situation that the server has already disconnected
			if( pMinecraft->levels[0] != nullptr ) pMinecraft->levels[0]->disconnect(false);
			if( pMinecraft->levels[1] != nullptr ) pMinecraft->levels[1]->disconnect(false);
		}
		else
		{
			exitReasonStringId = IDS_EXITING_GAME;
			pMinecraft->progressRenderer->progressStartNoAbort( IDS_EXITING_GAME );

			if( pMinecraft->levels[0] != nullptr ) pMinecraft->levels[0]->disconnect();
			if( pMinecraft->levels[1] != nullptr ) pMinecraft->levels[1]->disconnect();
		}

		// 4J Stu - This only does something if we actually have a server, so don't need to do any other checks
		MinecraftServer::HaltServer(true);

		// We need to call the stats & leaderboards save before we exit the session
		//pMinecraft->forceStatsSave();
		saveStats = false;

		// 4J Stu - Leave the session once the disconnect packet has been sent
		g_NetworkManager.LeaveGame(FALSE);
	}
	else
	{
		if(lpParameter != nullptr )
		{
			switch( app.GetDisconnectReason() )
			{
			case DisconnectPacket::eDisconnect_Kicked:
				exitReasonStringId = IDS_DISCONNECTED_KICKED;
				break;
			case DisconnectPacket::eDisconnect_NoUGC_AllLocal:
				exitReasonStringId = IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_ALL_LOCAL;
				break;
			case DisconnectPacket::eDisconnect_NoUGC_Single_Local:
				exitReasonStringId = IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_SINGLE_LOCAL;
				break;
#if defined(__PS3__) || defined(__ORBIS__) || defined (__PSVITA__)
			case DisconnectPacket::eDisconnect_ContentRestricted_AllLocal:
				exitReasonStringId = IDS_CONTENT_RESTRICTION_MULTIPLAYER;
				break;
			case DisconnectPacket::eDisconnect_ContentRestricted_Single_Local:
				exitReasonStringId = IDS_CONTENT_RESTRICTION;
				break;
#endif
#ifdef _XBOX
			case DisconnectPacket::eDisconnect_NoUGC_Remote:
				exitReasonStringId = IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_REMOTE;
				break;
#endif
			case DisconnectPacket::eDisconnect_OutdatedServer:
				exitReasonStringId = IDS_DISCONNECTED_SERVER_OLD;
				break;
			case DisconnectPacket::eDisconnect_OutdatedClient:
				exitReasonStringId = IDS_DISCONNECTED_CLIENT_OLD;
			default:
				exitReasonStringId = IDS_DISCONNECTED;
			}
			pMinecraft->progressRenderer->progressStartNoAbort( exitReasonStringId );
		}
	}
	pMinecraft->setLevel(nullptr,exitReasonStringId,nullptr,saveStats,true);

	// 4J-JEV: Fix for #106402 - TCR #014 BAS Debug Output:
	// TU12: Mass Effect Mash-UP: Save file "Default_DisplayName" is created on all storage devices after signing out from a re-launched pre-generated world
	app.m_gameRules.unloadCurrentGameRules(); //

	MinecraftServer::resetFlags();

	// We can't start/join a new game until the session is destroyed, so wait for it to be idle again
	while( g_NetworkManager.IsInSession() )
	{
		Sleep(1);
	}

	return S_OK;
}

int CMinecraftApp::UnlockFullInviteReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	//CMinecraftApp* pApp = (CMinecraftApp*)pParam;
	Minecraft *pMinecraft=Minecraft::GetInstance();
	bool bNoPlayer;

	// bug 11285 - TCR 001: BAS Game Stability: CRASH - When trying to join a full version game with a trial version, the trial crashes
	// 4J-PB - we may be in the main menus here, and we don't have a pMinecraft->player

	if(pMinecraft->player==nullptr)
	{
		bNoPlayer=true;
	}

	if(result==C4JStorage::EMessage_ResultAccept)
	{
		if(ProfileManager.IsSignedInLive(iPad))
		{
			// 4J-PB - need to check this user can access the store
#if defined(__PS3__) || defined(__PSVITA__)
			bool bContentRestricted;
			ProfileManager.GetChatAndContentRestrictions(ProfileManager.GetPrimaryPad(),true,nullptr,&bContentRestricted,nullptr);
			if(bContentRestricted)
			{
				UINT uiIDA[1];
				uiIDA[0]=IDS_CONFIRM_OK;
				ui.RequestErrorMessage(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, ProfileManager.GetPrimaryPad());
			}
			else
#endif
			{
				ProfileManager.DisplayFullVersionPurchase(false,iPad,eSen_UpsellID_Full_Version_Of_Game);
			}
		}
#if defined(__PS3__)
		else
		{
			// you're not signed in to PSN!
			UINT uiIDA[2];
			uiIDA[0]=IDS_PRO_NOTONLINE_ACCEPT;
			uiIDA[1]=IDS_PRO_NOTONLINE_DECLINE;
			ui.RequestErrorMessage(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_NOTONLINE_TEXT, uiIDA, 2, ProfileManager.GetPrimaryPad(),&CMinecraftApp::MustSignInFullVersionPurchaseReturned,&app);

		}
#endif
	}
	else
	{
		TelemetryManager->RecordUpsellResponded(iPad, eSen_UpsellID_Full_Version_Of_Game, app.m_dwOfferID, eSen_UpsellOutcome_Declined);
	}

	return 0;
}

int CMinecraftApp::UnlockFullSaveReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	//CMinecraftApp* pApp = (CMinecraftApp*)pParam;
	Minecraft *pMinecraft=Minecraft::GetInstance();

	if(result==C4JStorage::EMessage_ResultAccept)
	{
		if(ProfileManager.IsSignedInLive(pMinecraft->player->GetXboxPad()))
		{
			// 4J-PB - need to check this user can access the store
#if defined(__PS3__) || defined(__PSVITA__)
			bool bContentRestricted;
			ProfileManager.GetChatAndContentRestrictions(ProfileManager.GetPrimaryPad(),true,nullptr,&bContentRestricted,nullptr);
			if(bContentRestricted)
			{
				UINT uiIDA[1];
				uiIDA[0]=IDS_CONFIRM_OK;
				ui.RequestErrorMessage(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, ProfileManager.GetPrimaryPad());
			}
			else
#endif
			{
				ProfileManager.DisplayFullVersionPurchase(false,pMinecraft->player->GetXboxPad(),eSen_UpsellID_Full_Version_Of_Game);
			}
		}
#if defined(__PS3__)
		else
		{
			// you're not signed in to PSN!
			UINT uiIDA[2];
			uiIDA[0]=IDS_PRO_NOTONLINE_ACCEPT;
			uiIDA[1]=IDS_PRO_NOTONLINE_DECLINE;
			ui.RequestErrorMessage(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_NOTONLINE_TEXT, uiIDA, 2, ProfileManager.GetPrimaryPad(),&CMinecraftApp::MustSignInFullVersionPurchaseReturned,&app);
		}
#elif defined(__ORBIS__)
		else
		{
			// Determine why they're not "signed in live"
			if (ProfileManager.isSignedInPSN(iPad))
			{
				// Signed in to PSN but not connected (no internet access)
				assert(!ProfileManager.isConnectedToPSN(iPad));

				UINT uiIDA[1];
				uiIDA[0] = IDS_OK;
				ui.RequestErrorMessage( IDS_ERROR_NETWORK_TITLE, IDS_ERROR_NETWORK, uiIDA, 1, iPad);
			}
			else
			{
				// Not signed in to PSN
				UINT uiIDA[1];
				uiIDA[0] = IDS_PRO_NOTONLINE_ACCEPT;
				ui.RequestAlertMessage( IDS_PRO_NOTONLINE_TITLE, IDS_PRO_NOTONLINE_TEXT, uiIDA, 1, iPad, &CMinecraftApp::MustSignInFullVersionPurchaseReturned,&app);
			}
		}
#endif
	}
	else
	{
		TelemetryManager->RecordUpsellResponded(iPad, eSen_UpsellID_Full_Version_Of_Game, app.m_dwOfferID, eSen_UpsellOutcome_Declined);
	}

	return 0;
}

int CMinecraftApp::UnlockFullExitReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	CMinecraftApp* pApp = static_cast<CMinecraftApp *>(pParam);
	Minecraft *pMinecraft=Minecraft::GetInstance();

	if(result==C4JStorage::EMessage_ResultAccept)
	{
		if(ProfileManager.IsSignedInLive(pMinecraft->player->GetXboxPad()))
		{
			// 4J-PB - need to check this user can access the store
#if defined(__PS3__) || defined(__PSVITA__)
			bool bContentRestricted;
			ProfileManager.GetChatAndContentRestrictions(ProfileManager.GetPrimaryPad(),true,nullptr,&bContentRestricted,nullptr);
			if(bContentRestricted)
			{
				UINT uiIDA[1];
				uiIDA[0]=IDS_CONFIRM_OK;
				ui.RequestErrorMessage(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, ProfileManager.GetPrimaryPad());
			}
			else
#endif
			{
				ProfileManager.DisplayFullVersionPurchase(false,pMinecraft->player->GetXboxPad(),eSen_UpsellID_Full_Version_Of_Game);
#if defined __ORBIS__ || defined __PS3__ || defined __PSVITA__
				// still need to exit the trial or we'll be in the Pause menu with input ignored
				pApp->SetAction(pMinecraft->player->GetXboxPad(),eAppAction_ExitWorldTrial);
#endif
			}
		}
#if defined(__PS3__) || defined __PSVITA__
		else
		{
			// you're not signed in to PSN!
			UINT uiIDA[2];
			uiIDA[0]=IDS_PRO_NOTONLINE_ACCEPT;
			uiIDA[1]=IDS_PRO_NOTONLINE_DECLINE;
			ui.RequestErrorMessage(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_NOTONLINE_TEXT, uiIDA, 2, ProfileManager.GetPrimaryPad(),&CMinecraftApp::MustSignInFullVersionPurchaseReturnedExitTrial,&app);
		}
#elif defined(__ORBIS__)
		else
		{
			// Determine why they're not "signed in live"
			if (ProfileManager.isSignedInPSN(iPad))
			{
				// Signed in to PSN but not connected (no internet access)
				assert(!ProfileManager.isConnectedToPSN(iPad));

				UINT uiIDA[1];
				uiIDA[0] = IDS_OK;
				ui.RequestErrorMessage( IDS_ERROR_NETWORK_TITLE, IDS_ERROR_NETWORK, uiIDA, 1, iPad);
				// still need to exit the trial or we'll be in the Pause menu with input ignored
				pApp->SetAction(pMinecraft->player->GetXboxPad(),eAppAction_ExitWorldTrial);
			}
			else
			{
				// Not signed in to PSN
				UINT uiIDA[1];
				uiIDA[0] = IDS_PRO_NOTONLINE_ACCEPT;
				ui.RequestAlertMessage( IDS_PRO_NOTONLINE_TITLE, IDS_PRO_NOTONLINE_TEXT, uiIDA, 1, iPad, &CMinecraftApp::MustSignInFullVersionPurchaseReturnedExitTrial,&app);
			}
		}
#endif
	}
	else
	{
		TelemetryManager->RecordUpsellResponded(iPad, eSen_UpsellID_Full_Version_Of_Game, app.m_dwOfferID, eSen_UpsellOutcome_Declined);
		pApp->SetAction(pMinecraft->player->GetXboxPad(),eAppAction_ExitWorldTrial);
	}

	return 0;
}

int CMinecraftApp::TrialOverReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	CMinecraftApp* pApp = static_cast<CMinecraftApp *>(pParam);
	Minecraft *pMinecraft=Minecraft::GetInstance();

	if(result==C4JStorage::EMessage_ResultAccept)
	{
		// we need a signed in user for the unlock
		if(ProfileManager.IsSignedInLive(pMinecraft->player->GetXboxPad()))
		{
			// 4J-PB - need to check this user can access the store
#if defined(__PS3__) || defined(__PSVITA__)
			bool bContentRestricted;
			ProfileManager.GetChatAndContentRestrictions(ProfileManager.GetPrimaryPad(),true,nullptr,&bContentRestricted,nullptr);
			if(bContentRestricted)
			{
				UINT uiIDA[1];
				uiIDA[0]=IDS_CONFIRM_OK;
				ui.RequestErrorMessage(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, ProfileManager.GetPrimaryPad());
			}
			else
#endif
			{
				ProfileManager.DisplayFullVersionPurchase(false,pMinecraft->player->GetXboxPad(),eSen_UpsellID_Full_Version_Of_Game);
			}
		}
		else
		{
#if defined(__PS3__)

			// you're not signed in to PSN!
			UINT uiIDA[2];
			uiIDA[0]=IDS_PRO_NOTONLINE_ACCEPT;
			uiIDA[1]=IDS_PRO_NOTONLINE_DECLINE;
			ui.RequestErrorMessage(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_NOTONLINE_TEXT, uiIDA, 2, ProfileManager.GetPrimaryPad(),&CMinecraftApp::MustSignInFullVersionPurchaseReturned,&app);

			// 4J Stu - We can't actually exit the game, so just exit back to the main menu
			//pApp->SetAction(pMinecraft->player->GetXboxPad(),eAppAction_ExitWorldTrial);
#else
			pApp->SetAction(pMinecraft->player->GetXboxPad(),eAppAction_ExitTrial);
#endif
		}
	}
	else
	{
		TelemetryManager->RecordUpsellResponded(iPad, eSen_UpsellID_Full_Version_Of_Game, app.m_dwOfferID, eSen_UpsellOutcome_Declined);

#if defined(__PS3__) || defined(__ORBIS__)
		// 4J Stu - We can't actually exit the game, so just exit back to the main menu
		pApp->SetAction(pMinecraft->player->GetXboxPad(),eAppAction_ExitWorldTrial);
#else
		pApp->SetAction(pMinecraft->player->GetXboxPad(),eAppAction_ExitTrial);
#endif
	}

	return 0;
}

void CMinecraftApp::ProfileReadErrorCallback(void *pParam)
{
	CMinecraftApp *pApp=static_cast<CMinecraftApp *>(pParam);
	int iPrimaryPlayer=ProfileManager.GetPrimaryPad();
	pApp->SetAction(iPrimaryPlayer, eAppAction_ProfileReadError);
}

void CMinecraftApp::ClearSignInChangeUsersMask()
{
	// 4J-PB - When in the main menu, the user is on pad 0, and any change they make to their profile will be to pad 0 data
	// If they then go in as a secondary player to a splitscreen game, their profile will not be read again on pad 1 if they were previously in a splitscreen game
	// This is because m_uiLastSignInData remembers they were in previously, and doesn't read the profile data for them again
	// Fix this by resetting the m_uiLastSignInData on pressing play game for secondary users. The Primary user does a read profile on play game anyway
	int iPrimaryPlayer=ProfileManager.GetPrimaryPad();

	if(m_uiLastSignInData!=0)
	{
		if(iPrimaryPlayer>=0)
		{
			m_uiLastSignInData=1<<iPrimaryPlayer;
		}
		else
		{
			m_uiLastSignInData=0;
		}
	}
}
void CMinecraftApp::SignInChangeCallback(LPVOID pParam,bool bPrimaryPlayerChanged,unsigned int uiSignInData)
{
#ifdef __PS3__
	// this is normally set in the main menu, but we can go online in the create world screens, and the primary player name isn't updated
	Minecraft::GetInstance()->user->name = convStringToWstring( ProfileManager.GetGamertag(ProfileManager.GetPrimaryPad()));
#endif

	CMinecraftApp *pApp=static_cast<CMinecraftApp *>(pParam);
	// check if the primary player signed out
	int iPrimaryPlayer=ProfileManager.GetPrimaryPad();

	if((ProfileManager.GetLockedProfile()!=-1) && iPrimaryPlayer!=-1)
	{
		if ( ((uiSignInData & (1<<iPrimaryPlayer)) == 0) || bPrimaryPlayerChanged )
		{
			// Primary Player gone or there's been a sign out and sign in of the primary player, so kick them out
			pApp->SetAction(iPrimaryPlayer,eAppAction_PrimaryPlayerSignedOut);

			// 4J-PB - invalidate their banned level list
			pApp->InvalidateBannedList(iPrimaryPlayer);

			// need to ditch any DLCOffers info
			StorageManager.ClearDLCOffers();
			pApp->ClearAndResetDLCDownloadQueue();
			pApp->ClearDLCInstalled();
		}
		else
		{
			unsigned int uiChangedPlayers = uiSignInData ^ m_uiLastSignInData;

			if( g_NetworkManager.IsInSession() )
			{
				bool hasGuestIdChanged = false;
				for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
				{
					DWORD guestNumber = 0;
					if(ProfileManager.IsSignedIn(i))
					{
						XUSER_SIGNIN_INFO info;
						XUserGetSigninInfo(i,XUSER_GET_SIGNIN_INFO_OFFLINE_XUID_ONLY ,&info);
						pApp->DebugPrintf("Player at index %d has guest number %d\n", i,info.dwGuestNumber );
						guestNumber = info.dwGuestNumber;
					}
					if( pApp->m_currentSigninInfo[i].dwGuestNumber != 0 && guestNumber != 0 && pApp->m_currentSigninInfo[i].dwGuestNumber != guestNumber )
					{
						hasGuestIdChanged = true;
					}
				}

				if( hasGuestIdChanged )
				{
					UINT uiIDA[1];
					uiIDA[0]=IDS_CONFIRM_OK;
					ui.RequestErrorMessage(IDS_GUEST_ORDER_CHANGED_TITLE, IDS_GUEST_ORDER_CHANGED_TEXT, uiIDA, 1, ProfileManager.GetPrimaryPad());
				}

				// 4J Stu - On PS4 we can also cause to exit players if they are signed out here, but we shouldn't do that if
				// we are going to switch to an offline game as it will likely crash due to incompatible parallel processes
				bool switchToOffline = false;
				// If it's an online game, and the primary profile is no longer signed into LIVE then we act as if disconnected
				if( !ProfileManager.IsSignedInLive( ProfileManager.GetLockedProfile() ) && !g_NetworkManager.IsLocalGame() )
				{
					switchToOffline = true;
				}

				//printf("Old: %x, New: %x, Changed: %x\n", m_ulLastSignInData, ulSignInData, changedPlayers);
				for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
				{
					// Primary player shouldn't be subjected to these checks, and shouldn't call ExitPlayer
					if(i == iPrimaryPlayer) continue;

					// A guest a signed in or out, out of order which invalidates all the guest players we have in the game
					if(hasGuestIdChanged && pApp->m_currentSigninInfo[i].dwGuestNumber != 0 && g_NetworkManager.GetLocalPlayerByUserIndex(i)!=nullptr)
					{
						pApp->DebugPrintf("Recommending removal of player at index %d because their guest id changed\n",i);
						pApp->SetAction(i, eAppAction_ExitPlayer);
					}
					else
					{
						XUSER_SIGNIN_INFO info;
						XUserGetSigninInfo(i,XUSER_GET_SIGNIN_INFO_OFFLINE_XUID_ONLY ,&info);
						// 4J Stu - Also need to detect the case where the sign in mask is the same, but the player has swapped users (eg still signed in but xuid different)
						// Fix for #48451 - TU5: Code: UI: Splitscreen: Title crashes when switching to a profile previously signed out via splitscreen profile selection

						// 4J-PB - compiler complained about if below ('&&' within '||') - making it easier to read
						bool bPlayerChanged=(uiChangedPlayers&(1<<i))==(1<<i);
						bool bPlayerSignedIn=((uiSignInData&(1<<i))!=0);

						if( bPlayerChanged && (!bPlayerSignedIn || (bPlayerSignedIn && !ProfileManager.AreXUIDSEqual(pApp->m_currentSigninInfo[i].xuid, info.xuid) ) ))
						{
							// 4J-PB - invalidate their banned level list
							pApp->DebugPrintf("Player at index %d Left - invalidating their banned list\n",i);
							pApp->InvalidateBannedList(i);

							// 4J-HG: If either the player is in the network manager or in the game, need to exit player
							// TODO: Do we need to check the network manager?
							if (g_NetworkManager.GetLocalPlayerByUserIndex(i) != nullptr || Minecraft::GetInstance()->localplayers[i] != nullptr)
							{
								pApp->DebugPrintf("Player %d signed out\n", i);
								pApp->SetAction(i, eAppAction_ExitPlayer);
							}
						}
					}
#ifdef __ORBIS__
					// check if any of the addition players have signed out of PSN (primary player is handled below)
					if(!switchToOffline && i != ProfileManager.GetLockedProfile() && !g_NetworkManager.IsLocalGame())
					{
						if(g_NetworkManager.GetLocalPlayerByUserIndex(i)!=nullptr)
						{
							if(ProfileManager.IsSignedInLive(i) == false)
							{
								pApp->DebugPrintf("Recommending removal of player at index %d because they're no longer signed into PSNd\n",i);
								pApp->SetAction(i,eAppAction_ExitPlayer);
							}
						}
					}
#endif
				}

				// If it's an online game, and the primary profile is no longer signed into LIVE then we act as if disconnected
				if( switchToOffline )
				{
					pApp->SetAction(iPrimaryPlayer,eAppAction_EthernetDisconnected);
				}


				g_NetworkManager.HandleSignInChange();
			}
			// Some menus require the player to be signed in to live, so if this callback happens and the primary player is
			// no longer signed in then nav back
			else if ( pApp->GetLiveLinkRequired() && !ProfileManager.IsSignedInLive( ProfileManager.GetLockedProfile() ) )
			{
#ifdef __PSVITA__
				if(!CGameNetworkManager::usingAdhocMode())	// if we're in adhoc mode, we can ignore this
#endif
				{
				pApp->SetAction(iPrimaryPlayer,eAppAction_EthernetDisconnected);
			}
			}

#if ( defined __PS3__ || defined __ORBIS__ || defined _DURANGO  || defined __PSVITA__ )
			// 4J-JEV: Need to kick of loading of profile data for sub-sign in players.
			for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
			{
				if( i != iPrimaryPlayer
					&& ( uiChangedPlayers & (1<<i) )
					&& ( uiSignInData & (1<<i) )
					)
				{
					StorageManager.ReadFromProfile(i);
				}
			}
#endif
		}
		m_uiLastSignInData = uiSignInData;
	}
	else if(iPrimaryPlayer!=-1)
	{
		// make sure the TMS banned list data is ditched - the player may have gone in to help & options, backed out, and signed out
		pApp->InvalidateBannedList(iPrimaryPlayer);

		// need to ditch any DLCOffers info
		StorageManager.ClearDLCOffers();
		pApp->ClearAndResetDLCDownloadQueue();
		pApp->ClearDLCInstalled();

	}

	// Update the guest numbers to the current state
	for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
	{
		if(FAILED(XUserGetSigninInfo(i,XUSER_GET_SIGNIN_INFO_OFFLINE_XUID_ONLY,&pApp->m_currentSigninInfo[i])))
		{
			pApp->m_currentSigninInfo[i].xuid = INVALID_XUID;
			pApp->m_currentSigninInfo[i].dwGuestNumber = 0;
		}
		app.DebugPrintf("Player at index %d has guest number %d\n", i,pApp->m_currentSigninInfo[i].dwGuestNumber );
	}
}

void CMinecraftApp::NotificationsCallback(LPVOID pParam,DWORD dwNotification, unsigned int uiParam)
{
	CMinecraftApp* pClass = static_cast<CMinecraftApp *>(pParam);

	// push these on to the notifications to be handled in qnet's dowork

	PNOTIFICATION pNotification = new NOTIFICATION;

	pNotification->dwNotification=dwNotification;
	pNotification->uiParam=uiParam;

	switch( dwNotification )
	{
	case XN_SYS_SIGNINCHANGED:
		{
			pClass->DebugPrintf("Signing changed - %d\n", uiParam );
		}
		break;
	case XN_SYS_INPUTDEVICESCHANGED:
		if(app.GetGameStarted() && g_NetworkManager.IsInSession())
		{
			for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
			{
				if(!InputManager.IsPadConnected(i) &&
					Minecraft::GetInstance()->localplayers[i] != nullptr &&
					!ui.IsPauseMenuDisplayed(i) && !ui.IsSceneInStack(i, eUIScene_EndPoem) )
				{
					ui.CloseUIScenes(i);
					ui.NavigateToScene(i,eUIScene_PauseMenu);
				}
			}
		}
		break;
	case XN_LIVE_CONTENT_INSTALLED:
		// Need to inform xuis that we've possibly had DLC installed
		{
			//app.m_dlcManager.SetNeedsUpdated(true);
			// Clear the DLC installed flag to cause a GetDLC to run if it's called
			app.ClearDLCInstalled();

			ui.HandleDLCInstalled(ProfileManager.GetPrimaryPad());
		}
		break;
	case XN_SYS_STORAGEDEVICESCHANGED:
		{
#ifdef _XBOX
			// If the devices have changed, and we've got a dlc pack with audio selected, and that pack's content device is no longer valid... then pull the plug on
			// audio streaming, as if we leave this until later xact gets locked up attempting to destroy the streamed wave bank.
			TexturePack *pTexPack=Minecraft::GetInstance()->skins->getSelected();
			if(pTexPack->hasAudio())
			{
				DLCTexturePack *pDLCTexPack=(DLCTexturePack *)pTexPack;
				XCONTENTDEVICEID deviceID = pDLCTexPack->GetDLCDeviceID();
				if( XContentGetDeviceState( deviceID, nullptr ) != ERROR_SUCCESS )
				{
					// Set texture pack flag so that it is now considered as not having audio - this is critical so that the next playStreaming does what it is meant to do,
					// and also so that we don't try and unmount this again, or play any sounds from it in the future
					pTexPack->setHasAudio(false);
					// need to stop the streaming audio - by playing streaming audio from the default texture pack now
					Minecraft::GetInstance()->soundEngine->playStreaming(L"", 0, 0, 0, 0, 0);

					if(pDLCTexPack->m_pStreamedWaveBank!=nullptr)
					{
						pDLCTexPack->m_pStreamedWaveBank->Destroy();
					}
					if(pDLCTexPack->m_pSoundBank!=nullptr)
					{
						pDLCTexPack->m_pSoundBank->Destroy();
					}
					DWORD result = StorageManager.UnmountInstalledDLC("TPACK");
					app.DebugPrintf("Unmount result is %d\n",result);
				}
			}
#endif
		}
		break;
	}

	pClass->m_vNotifications.push_back(pNotification);
}

#if defined __PS3__ || defined __PSVITA__ || defined __ORBIS__
int CMinecraftApp::MustSignInFullVersionPurchaseReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	if(result==C4JStorage::EMessage_ResultAccept)
	{
#ifdef __PS3__
		SQRNetworkManager_PS3::AttemptPSNSignIn(&CMinecraftApp::NowDisplayFullVersionPurchase, &app,true);
#elif defined __PSVITA__
		SQRNetworkManager_Vita::AttemptPSNSignIn(&CMinecraftApp::NowDisplayFullVersionPurchase, &app,true);
#else // __PS4__
		SQRNetworkManager_Orbis::AttemptPSNSignIn(&CMinecraftApp::NowDisplayFullVersionPurchase, &app,true);
#endif
	}

	return 0;
}

#if defined __PS3__ || defined __PSVITA__ || defined __ORBIS__
int CMinecraftApp::MustSignInFullVersionPurchaseReturnedExitTrial(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	if(result==C4JStorage::EMessage_ResultAccept)
	{
#ifdef __PS3__
		SQRNetworkManager_PS3::AttemptPSNSignIn(&CMinecraftApp::NowDisplayFullVersionPurchase, &app,true);
#elif defined __PSVITA__
		SQRNetworkManager_Vita::AttemptPSNSignIn(&CMinecraftApp::NowDisplayFullVersionPurchase, &app,true);
#else // __PS4__
		SQRNetworkManager_Orbis::AttemptPSNSignIn(&CMinecraftApp::NowDisplayFullVersionPurchase, &app,true);
#endif
	}

	//4J-PB - we need to exit the trial, or we'll be in the pause menu with ignore input true
	app.SetAction(iPad,eAppAction_ExitWorldTrial);

	return 0;
}
#endif

int CMinecraftApp::NowDisplayFullVersionPurchase(void *pParam, bool bContinue, int iPad)
{
	app.m_bDisplayFullVersionPurchase=true;
	return 0;
}
#endif
void CMinecraftApp::UpsellReturnedCallback(LPVOID pParam, eUpsellType type, eUpsellResponse result, int iUserData)
{
	ESen_UpsellID senType;
	ESen_UpsellOutcome senResponse;
#ifdef __PS3__
	UINT uiIDA[2];
#endif

	// Map the eUpsellResponse to the enum we use for sentient
	switch(result)
	{
	case eUpsellResponse_Accepted_NoPurchase:
		senResponse = eSen_UpsellOutcome_Went_To_Guide;
		break;
	case eUpsellResponse_Accepted_Purchase:
		senResponse = eSen_UpsellOutcome_Accepted;
		break;
#ifdef __PS3__
		// special case for people who are not signed in to the PSN while playing the trial game
	case eUpsellResponse_UserNotSignedInPSN:

		uiIDA[0]=IDS_PRO_NOTONLINE_ACCEPT;
		uiIDA[1]=IDS_PRO_NOTONLINE_DECLINE;
		ui.RequestErrorMessage(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_NOTONLINE_TEXT, uiIDA, 2, ProfileManager.GetPrimaryPad(),&CMinecraftApp::MustSignInFullVersionPurchaseReturned,&app);

		return;

	case eUpsellResponse_NotAllowedOnline: // On earning a trophy in the trial version, where the user is underage and can't go online to buy the game, but they selected to buy the game on the trophy upsell
		uiIDA[0]=IDS_CONFIRM_OK;
		ui.RequestErrorMessage(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, ProfileManager.GetPrimaryPad());
		break;
#endif
	case eUpsellResponse_Declined:
	default:
		senResponse = eSen_UpsellOutcome_Declined;
		break;
	};

	// Map the eUpsellType to the enum we use for sentient
	switch(type)
	{
	case eUpsellType_Custom:
		senType = eSen_UpsellID_Full_Version_Of_Game;
		break;
	default:
		senType = eSen_UpsellID_Undefined;
		break;
	};

	// Always the primary pad that gets an upsell
	TelemetryManager->RecordUpsellResponded(ProfileManager.GetPrimaryPad(), eSen_UpsellID_Full_Version_Of_Game, app.m_dwOfferID, senResponse);
}

#ifdef _DEBUG_MENUS_ENABLED
bool CMinecraftApp::DebugArtToolsOn()
{
	return DebugSettingsOn() && (GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad())&(1L<<eDebugSetting_ArtTools)) != 0;
}
#endif

void CMinecraftApp::SetDebugSequence(const char *pchSeq)
{
	InputManager.SetDebugSequence(pchSeq,&CMinecraftApp::DebugInputCallback,this);
}
int CMinecraftApp::DebugInputCallback(LPVOID pParam)
{
	CMinecraftApp* pClass = static_cast<CMinecraftApp *>(pParam);
	//printf("sequence matched\n");
	pClass->m_bDebugOptions=!pClass->m_bDebugOptions;

	for(int i=0;i<XUSER_MAX_COUNT;i++)
	{
		if(app.DebugSettingsOn())
		{
			app.ActionDebugMask(i);
		}
		else
		{
			// force debug mask off
			app.ActionDebugMask(i,true);
		}
	}

	return 0;
}

int CMinecraftApp::GetLocalPlayerCount(void)
{
	int iPlayerC=0;
	Minecraft *pMinecraft = Minecraft::GetInstance();
	for(int i=0;i<XUSER_MAX_COUNT;i++)
	{
		if(pMinecraft != nullptr && pMinecraft->localplayers[i] != nullptr)
		{
			iPlayerC++;
		}
	}

	return iPlayerC;
}

int CMinecraftApp::MarketplaceCountsCallback(LPVOID pParam,C4JStorage::DLC_TMS_DETAILS *pTMSDetails, int iPad)
{
	app.DebugPrintf("Marketplace Counts= New - %d Total - %d\n",pTMSDetails->dwNewOffers,pTMSDetails->dwTotalOffers);

	if(pTMSDetails->dwNewOffers>0)
	{
		app.m_bNewDLCAvailable=true;
		app.m_bSeenNewDLCTip=false;
	}
	else
	{
		app.m_bNewDLCAvailable=false;
		app.m_bSeenNewDLCTip=true;
	}

	return 0;
}

bool CMinecraftApp::StartInstallDLCProcess(int iPad)
{
	app.DebugPrintf("--- CMinecraftApp::StartInstallDLCProcess: pad=%i.\n", iPad);

	// If there is already a call to this in progress, then do nothing
	// If the app says dlc is installed, then there has been no new system message to tell us there's new DLC since the last call to StartInstallDLCProcess
	if((app.DLCInstallProcessCompleted()==false) && (m_bDLCInstallPending==false))
	{
		app.m_dlcManager.resetUnnamedCorruptCount();
		m_bDLCInstallPending = true;
		m_iTotalDLC = 0;
		m_iTotalDLCInstalled = 0;
		app.DebugPrintf("--- CMinecraftApp::StartInstallDLCProcess - StorageManager.GetInstalledDLC\n");

		StorageManager.GetInstalledDLC(iPad,&CMinecraftApp::DLCInstalledCallback,this);
		return true;
	}
	else
	{
		app.DebugPrintf("--- CMinecraftApp::StartInstallDLCProcess - nothing to do\n");

		return false;
	}

}

// Installed DLC callback
int CMinecraftApp::DLCInstalledCallback(LPVOID pParam,int iInstalledC,int iPad)
{
	app.DebugPrintf("--- CMinecraftApp::DLCInstalledCallback: totalDLC=%i, pad=%i.\n", iInstalledC, iPad);
	app.m_iTotalDLC = iInstalledC;
	app.MountNextDLC(iPad);
	return 0;
}

void CMinecraftApp::MountNextDLC(int iPad)
{
	app.DebugPrintf("--- CMinecraftApp::MountNextDLC: pad=%i.\n", iPad);
	if(m_iTotalDLCInstalled < m_iTotalDLC)
	{
		// Mount it
		// We also need to match the ones the user wants to mount with the installed DLC
		// We're supposed to use a generic save game as a cache of these to do this, with XUSER_ANY

		if(StorageManager.MountInstalledDLC(iPad,m_iTotalDLCInstalled,&CMinecraftApp::DLCMountedCallback,this)!=ERROR_IO_PENDING )
		{
			// corrupt DLC
			app.DebugPrintf("Failed to mount DLC %d for pad %d\n",m_iTotalDLCInstalled,iPad);
			++m_iTotalDLCInstalled;
			app.MountNextDLC(iPad);
		}
		else
		{
			app.DebugPrintf("StorageManager.MountInstalledDLC ok\n");
		}
	}
	else
	{
		/* Removed - now loading these on demand instead of as each pack is mounted
		if(m_iTotalDLCInstalled > 0)
		{
		Minecraft *pMinecraft=Minecraft::GetInstance();
		pMinecraft->levelRenderer->AddDLCSkinsToMemTextures();
		}
		*/

		m_bDLCInstallPending = false;
		m_bDLCInstallProcessCompleted=true;

		ui.HandleDLCMountingComplete();

#if defined(_XBOX_ONE) || defined(__ORBIS__)
		// Check if the current texture pack is now installed
		if(!Minecraft::GetInstance()->skins->isUsingDefaultSkin())
		{
			TexturePack *tPack = Minecraft::GetInstance()->skins->getSelected();
			DLCTexturePack *pDLCTexPack=(DLCTexturePack *)tPack;

			DLCPack *pParentPack=pDLCTexPack->getDLCInfoParentPack();//tPack->getDLCPack();

			if(pParentPack->hasPurchasedFile( DLCManager::e_DLCType_Texture, L"" ))
			{
				StorageManager.SetSaveDisabled(false);
			}
		}
#endif
#if defined __PS3__ || defined __ORBIS__ || defined __PSVITA__
		{
			TexturePack* currentTPack = Minecraft::GetInstance()->skins->getSelected();
			TexturePack* requiredTPack = Minecraft::GetInstance()->skins->getTexturePackById(app.GetRequiredTexturePackID());
			if(currentTPack != requiredTPack)
			{
				Minecraft::GetInstance()->skins->selectTexturePackById(app.GetRequiredTexturePackID());
			}
		}
#endif
	}
}

// 4J-JEV: For the sake of clarity in DLCMountedCallback.
#if defined(_XBOX) || defined(__PS3__) || defined(_WINDOWS64)
#define CONTENT_DATA_DISPLAY_NAME(a) (a.szDisplayName)
#else
#define CONTENT_DATA_DISPLAY_NAME(a) (a.wszDisplayName)
#endif

int CMinecraftApp::DLCMountedCallback(LPVOID pParam,int iPad,DWORD dwErr,DWORD dwLicenceMask)
{
#if defined(_XBOX) || defined(_DURANGO) || defined(__PS3__) || defined(__ORBIS__) || defined(_WINDOWS64)  || defined (__PSVITA__) //Chris TODO
	app.DebugPrintf("--- CMinecraftApp::DLCMountedCallback\n");

	if(dwErr!=ERROR_SUCCESS)
	{
		// corrupt DLC
		app.DebugPrintf("Failed to mount DLC for pad %d: %d\n",iPad,dwErr);
		app.m_dlcManager.incrementUnnamedCorruptCount();
	}
	else
	{
		XCONTENT_DATA ContentData = StorageManager.GetDLC(app.m_iTotalDLCInstalled);

		DLCPack *pack = app.m_dlcManager.getPack( CONTENT_DATA_DISPLAY_NAME(ContentData) );

		if( pack != nullptr && pack->IsCorrupt() )
		{
			app.DebugPrintf("Pack '%ls' is corrupt, removing it from the DLC Manager.\n", CONTENT_DATA_DISPLAY_NAME(ContentData));

			app.m_dlcManager.removePack(pack);
			pack = nullptr;
		}

		if(pack == nullptr)
		{
			app.DebugPrintf("Pack \"%ls\" is not installed, so adding it\n", CONTENT_DATA_DISPLAY_NAME(ContentData));

#if defined(_XBOX) || defined(__PS3__) || defined(_WINDOWS64)
			pack = new DLCPack(ContentData.szDisplayName,dwLicenceMask);
#elif defined _XBOX_ONE
			pack = new DLCPack(ContentData.wszDisplayName,ContentData.wszProductID,dwLicenceMask);
#else
			pack = new DLCPack(ContentData.wszDisplayName,dwLicenceMask);
#endif
			pack->SetDLCMountIndex(app.m_iTotalDLCInstalled);
			pack->SetDLCDeviceID(ContentData.DeviceID);
			app.m_dlcManager.addPack(pack);

			app.HandleDLC(pack);

			if(pack->getDLCItemsCount(DLCManager::e_DLCType_Texture) > 0)
			{
				Minecraft::GetInstance()->skins->addTexturePackFromDLC(pack, pack->GetPackId() );
			}
		}
		else
		{
			app.DebugPrintf("Pack \"%ls\" is already installed. Updating license to %d\n", CONTENT_DATA_DISPLAY_NAME(ContentData), dwLicenceMask);

			pack->SetDLCMountIndex(app.m_iTotalDLCInstalled);
			pack->SetDLCDeviceID(ContentData.DeviceID);
			pack->updateLicenseMask(dwLicenceMask);
		}

		StorageManager.UnmountInstalledDLC();
	}
	++app.m_iTotalDLCInstalled;
	app.MountNextDLC(iPad);

#endif // __PSVITA__
	return 0;
}
#undef CONTENT_DATA_DISPLAY_NAME

//  void CMinecraftApp::InstallDefaultCape()
//  {
// 	 if(!m_bDefaultCapeInstallAttempted)
// 	 {
// 		 // we only attempt to install the cape once per launch of the game
// 		 m_bDefaultCapeInstallAttempted=true;
//
// 		 wstring wTemp=L"Default_Cape.png";
// 		 bool bRes=app.IsFileInMemoryTextures(wTemp);
// 		 // if the file is not already in the memory textures, then read it from TMS
// 		 if(!bRes)
//       {
//           BYTE* pBuffer = nullptr;
// 			 DWORD dwSize=0;
// 			 // 4J-PB - out for now for DaveK so he doesn't get the birthday cape
// #ifdef _CONTENT_PACKAGE
// 			 C4JStorage::ETMSStatus eTMSStatus;
// 			 eTMSStatus=StorageManager.ReadTMSFile(ProfileManager.GetPrimaryPad(),C4JStorage::eGlobalStorage_Title,C4JStorage::eTMS_FileType_Graphic, L"Default_Cape.png",&pBuffer, &dwSize);
// 			 if(eTMSStatus==C4JStorage::ETMSStatus_Idle)
// 			 {
// 				 app.AddMemoryTextureFile(wTemp,pBuffer,dwSize);
// 			 }
// #endif
// 		 }
// 	 }
//  }

void CMinecraftApp::HandleDLC(DLCPack *pack)
{
	DWORD dwFilesProcessed = 0;
#ifndef _XBOX
#if defined(__PS3__) || defined(__ORBIS__) || defined(_WINDOWS64) || defined (__PSVITA__)
	std::vector<std::string> dlcFilenames;
#elif defined _DURANGO
	std::vector<std::wstring> dlcFilenames;
#endif
	StorageManager.GetMountedDLCFileList("DLCDrive", dlcFilenames);
#ifdef __ORBIS__
	// 4J Stu - I don't know why we handle more than one file here any more, however this doesn't seem to work with the PS4 patches
	if(dlcFilenames.size() > 0) m_dlcManager.readDLCDataFile(dwFilesProcessed, dlcFilenames[0], pack);
#else
	for(size_t i=0; i<dlcFilenames.size();i++)
	{
		m_dlcManager.readDLCDataFile(dwFilesProcessed, dlcFilenames[i], pack);
	}
#endif
#else
	WIN32_FIND_DATA wfd;
	HANDLE hFind;

	char szPath[] = "DLCDrive:\\";
	char szFullFilename[256];
	strcpy(szFullFilename,szPath);
	strcat(szFullFilename,"*");

	// Start the find and check for failure.
	hFind = FindFirstFile( szFullFilename, &wfd );

	if( INVALID_HANDLE_VALUE == hFind )
	{
		app.DebugPrintf( "FindFirstFile failed." );
	}
	else
	{
		// Display each file and ask for the next.
		do
		{
			strcpy(szFullFilename,szPath);
			strcat(szFullFilename,wfd.cFileName);

			if(( GetFileAttributes( szFullFilename ) & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
			{
#ifdef _XBOX
				DWORD dwPackID=m_dlcManager.retrievePackIDFromDLCDataFile(szFullFilename,pack);

				// Do we need to override the TexturePack.pck with an updated version in a TU?
				wstring wsTemp=getFilePath(dwPackID, wstring(L"TexturePack.pck"),false );
				File texturePCKPath(wsTemp );
				if(texturePCKPath.exists())
				{
					app.DebugPrintf("Found a replacement .pck\n");
					m_dlcManager.readDLCDataFile(dwFilesProcessed, wsTemp,pack);
				}
				else
				{
					m_dlcManager.readDLCDataFile(dwFilesProcessed, szFullFilename,pack);
				}
#else
				m_dlcManager.readDLCDataFile(dwFilesProcessed, szFullFilename,pack);

#endif
			}
		}
		while( FindNextFile( hFind, &wfd ) );

		// Close the find handle.
		FindClose( hFind );
	}
#endif // __PS3__ || __ORBIS__

	if( dwFilesProcessed == 0 ) m_dlcManager.removePack(pack);
}

//  int CMinecraftApp::DLCReadCallback(LPVOID pParam,C4JStorage::DLC_FILE_DETAILS *pDLCData)
//  {
//
//
// 	 return 0;
//  }

//-------------------------------------------------------------------------------------
// Name: InitTime()
// Desc: Initializes the timer variables
//-------------------------------------------------------------------------------------
void CMinecraftApp::InitTime()
{

	// Get the frequency of the timer
	LARGE_INTEGER qwTicksPerSec;
	QueryPerformanceFrequency( &qwTicksPerSec );
	m_Time.fSecsPerTick = 1.0f / static_cast<float>(qwTicksPerSec.QuadPart);

	// Save the start time
	QueryPerformanceCounter( &m_Time.qwTime );

	// Zero out the elapsed and total time
	m_Time.qwAppTime.QuadPart = 0;
	m_Time.fAppTime = 0.0f;
	m_Time.fElapsedTime = 0.0f;
}

//-------------------------------------------------------------------------------------
// Name: UpdateTime()
// Desc: Updates the elapsed time since our last frame.
//-------------------------------------------------------------------------------------
void CMinecraftApp::UpdateTime()
{
	LARGE_INTEGER qwNewTime;
	LARGE_INTEGER qwDeltaTime;

	QueryPerformanceCounter( &qwNewTime );
	qwDeltaTime.QuadPart = qwNewTime.QuadPart - m_Time.qwTime.QuadPart;

	m_Time.qwAppTime.QuadPart += qwDeltaTime.QuadPart;
	m_Time.qwTime.QuadPart     = qwNewTime.QuadPart;

    m_Time.fElapsedTime = m_Time.fSecsPerTick * static_cast<FLOAT>(qwDeltaTime.QuadPart);
    m_Time.fAppTime = m_Time.fSecsPerTick * static_cast<FLOAT>(m_Time.qwAppTime.QuadPart);
}

bool CMinecraftApp::isXuidNotch(PlayerUID xuid)
{
	if(m_xuidNotch != INVALID_XUID && xuid != INVALID_XUID)
	{
		return ProfileManager.AreXUIDSEqual(xuid, m_xuidNotch) == TRUE;
	}
	return false;
}

bool CMinecraftApp::isXuidDeadmau5(PlayerUID xuid)
{
	auto it = MojangData.find(xuid); // 4J Stu - The .at and [] accessors insert elements if they don't exist
	if (it != MojangData.end() )
	{
		MOJANG_DATA *pMojangData=MojangData[xuid];
		if(pMojangData && pMojangData->eXuid==eXUID_Deadmau5)
		{
			return true;
		}
	}

	return false;
}

void CMinecraftApp::AddMemoryTextureFile(const wstring &wName,PBYTE pbData,DWORD dwBytes)
{
	EnterCriticalSection(&csMemFilesLock);
	// check it's not already in
	PMEMDATA pData=nullptr;
	auto it = m_MEM_Files.find(wName);
	if(it != m_MEM_Files.end())
	{
#ifndef _CONTENT_PACKAGE
		wprintf(L"Incrementing the memory texture file count for %ls\n", wName.c_str());
#endif
		pData = (*it).second;

		if(pData->dwBytes == 0 && dwBytes != 0)
		{
			// This should never be nullptr if dwBytes is 0
			if(pData->pbData!=nullptr) delete [] pData->pbData;

			pData->pbData=pbData;
			pData->dwBytes=dwBytes;
		}

		++pData->ucRefCount;
		LeaveCriticalSection(&csMemFilesLock);
		return;
	}

#ifndef _CONTENT_PACKAGE
	//wprintf(L"Adding the memory texture file data for %ls\n", wName.c_str());
#endif
	// this is a texture (png) file

	// add this texture to the list of memory texture files - it will then be picked up by the level renderer's AddEntity

	pData = (PMEMDATA)new BYTE[sizeof(MEMDATA)];
	ZeroMemory( pData, sizeof(MEMDATA) );
	pData->pbData=pbData;
	pData->dwBytes=dwBytes;
	pData->ucRefCount = 1;

	// use the xuid to access the skin data
	m_MEM_Files[wName]=pData;

	LeaveCriticalSection(&csMemFilesLock);
}

void CMinecraftApp::RemoveMemoryTextureFile(const wstring &wName)
{
	EnterCriticalSection(&csMemFilesLock);

	auto it = m_MEM_Files.find(wName);
	if(it != m_MEM_Files.end())
	{
#ifndef _CONTENT_PACKAGE
		wprintf(L"Decrementing the memory texture file count for %ls\n", wName.c_str());
#endif
		PMEMDATA pData = (*it).second;
		--pData->ucRefCount;
		if(pData->ucRefCount <= 0)
		{
#ifndef _CONTENT_PACKAGE
			wprintf(L"Erasing the memory texture file data for %ls\n", wName.c_str());
#endif
			delete [] pData;
			m_MEM_Files.erase(wName);
		}
	}
	LeaveCriticalSection(&csMemFilesLock);
}

bool CMinecraftApp::DefaultCapeExists()
{
	wstring wTex=L"Special_Cape.png";
	bool val = false;

	EnterCriticalSection(&csMemFilesLock);
	auto it = m_MEM_Files.find(wTex);
	if(it != m_MEM_Files.end()) val = true;
	LeaveCriticalSection(&csMemFilesLock);

	return val;
}

bool CMinecraftApp::IsFileInMemoryTextures(const wstring &wName)
{
	bool val = false;

	EnterCriticalSection(&csMemFilesLock);
	auto it = m_MEM_Files.find(wName);
	if(it != m_MEM_Files.end()) val = true;
	LeaveCriticalSection(&csMemFilesLock);

	return val;
}

void CMinecraftApp::GetMemFileDetails(const wstring &wName,PBYTE *ppbData,DWORD *pdwBytes)
{
	EnterCriticalSection(&csMemFilesLock);
	auto it = m_MEM_Files.find(wName);
	if(it != m_MEM_Files.end())
	{
		PMEMDATA pData = (*it).second;
		*ppbData=pData->pbData;
		*pdwBytes=pData->dwBytes;
	}
	LeaveCriticalSection(&csMemFilesLock);
}

void CMinecraftApp::AddMemoryTPDFile(int iConfig,PBYTE pbData,DWORD dwBytes)
{
	EnterCriticalSection(&csMemTPDLock);
	// check it's not already in
	PMEMDATA pData=nullptr;
	auto it = m_MEM_TPD.find(iConfig);
	if(it == m_MEM_TPD.end())
	{
		pData = (PMEMDATA)new BYTE[sizeof(MEMDATA)];
		ZeroMemory( pData, sizeof(MEMDATA) );
		pData->pbData=pbData;
		pData->dwBytes=dwBytes;
		pData->ucRefCount = 1;

		m_MEM_TPD[iConfig]=pData;
	}

	LeaveCriticalSection(&csMemTPDLock);
}

void CMinecraftApp::RemoveMemoryTPDFile(int iConfig)
{
	EnterCriticalSection(&csMemTPDLock);
	// check it's not already in
	PMEMDATA pData=nullptr;
	auto it = m_MEM_TPD.find(iConfig);
	if(it != m_MEM_TPD.end())
	{
		pData=m_MEM_TPD[iConfig];
		delete [] pData;
		m_MEM_TPD.erase(iConfig);
	}

	LeaveCriticalSection(&csMemTPDLock);
}

#ifdef _XBOX
int CMinecraftApp::GetTPConfigVal(WCHAR *pwchDataFile)
{
	DLC_INFO *pDLCInfo=nullptr;
	// run through the DLC info to find the right texture pack/mash-up pack
	for(unsigned int i = 0; i < app.GetDLCInfoTexturesOffersCount(); ++i)
	{
		ULONGLONG ull=app.GetDLCInfoTexturesFullOffer(i);
		pDLCInfo=app.GetDLCInfoForFullOfferID(ull);

		if(wcscmp(pwchDataFile,pDLCInfo->wchDataFile)==0)
		{
			return pDLCInfo->iConfig;
		}
	}

	return -1;
}
#elif defined _XBOX_ONE
int CMinecraftApp::GetTPConfigVal(WCHAR *pwchDataFile)
{
	DLC_INFO *pDLCInfo=nullptr;
	// run through the DLC info to find the right texture pack/mash-up pack
	for(unsigned int i = 0; i < app.GetDLCInfoTexturesOffersCount(); ++i)
	{
		pDLCInfo=app.GetDLCInfoForFullOfferID((WCHAR *)app.GetDLCInfoTexturesFullOffer(i).c_str());

		if(wcscmp(pwchDataFile,pDLCInfo->wchDataFile)==0)
		{
			return pDLCInfo->iConfig;
		}
	}

	return -1;
}
#elif defined _WINDOWS64
int CMinecraftApp::GetTPConfigVal(WCHAR *pwchDataFile)
{
	return -1;
}
#endif
bool CMinecraftApp::IsFileInTPD(int iConfig)
{
	bool val = false;

	EnterCriticalSection(&csMemTPDLock);
	auto it = m_MEM_TPD.find(iConfig);
	if(it != m_MEM_TPD.end()) val = true;
	LeaveCriticalSection(&csMemTPDLock);

	return val;
}

void CMinecraftApp::GetTPD(int iConfig,PBYTE *ppbData,DWORD *pdwBytes)
{
	EnterCriticalSection(&csMemTPDLock);
	auto it = m_MEM_TPD.find(iConfig);
	if(it != m_MEM_TPD.end())
	{
		PMEMDATA pData = (*it).second;
		*ppbData=pData->pbData;
		*pdwBytes=pData->dwBytes;
	}
	LeaveCriticalSection(&csMemTPDLock);
}


// bool CMinecraftApp::UploadFileToGlobalStorage(int iQuadrant, C4JStorage::eGlobalStorage eStorageFacility, wstring *wsFile  )
// {
// 	bool bRes=false;
// #ifndef _CONTENT_PACKAGE
// 	// read the local file
// 	File gtsFile( wsFile->c_str() );
//
// 	int64_t fileSize = gtsFile.length();
//
// 	if(fileSize!=0)
// 	{
// 		FileInputStream fis(gtsFile);
// 		byteArray ba((int)fileSize);
// 		fis.read(ba);
// 		fis.close();
//
// 		bRes=StorageManager.WriteTMSFile(iQuadrant,eStorageFacility,(WCHAR *)wsFile->c_str(),ba.data, ba.length);
//
// 	}
// #endif
// 	return bRes;
// }






void CMinecraftApp::StoreLaunchData()
{

}

void CMinecraftApp::ExitGame()
{
}

// Invites

void CMinecraftApp::ProcessInvite(DWORD dwUserIndex, DWORD dwLocalUsersMask, const INVITE_INFO * pInviteInfo)
{
	m_InviteData.dwUserIndex=dwUserIndex;
	m_InviteData.dwLocalUsersMask=dwLocalUsersMask;
	m_InviteData.pInviteInfo=pInviteInfo;
	//memcpy(&m_InviteData,pJoinData,sizeof(JoinFromInviteData));
	SetAction(dwUserIndex,eAppAction_ExitAndJoinFromInvite);
}

int CMinecraftApp::ExitAndJoinFromInvite(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	CMinecraftApp* pApp = static_cast<CMinecraftApp *>(pParam);
	//Minecraft *pMinecraft=Minecraft::GetInstance();

	// buttons are swapped on this menu
	if(result==C4JStorage::EMessage_ResultDecline)
	{
		pApp->SetAction(iPad,eAppAction_ExitAndJoinFromInviteConfirmed);
	}

	return 0;
}

int CMinecraftApp::ExitAndJoinFromInviteSaveDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	CMinecraftApp *pClass = static_cast<CMinecraftApp *>(pParam);
	// Exit with or without saving
	// Decline means save in this dialog
	if(result==C4JStorage::EMessage_ResultDecline || result==C4JStorage::EMessage_ResultThirdOption)
	{
		if( result==C4JStorage::EMessage_ResultDecline ) // Save
		{
			// Check they have the full texture pack if they are using one
			// 4J-PB - Is the player trying to save but they are using a trial texturepack ?
			if(!Minecraft::GetInstance()->skins->isUsingDefaultSkin())
			{
				TexturePack *tPack = Minecraft::GetInstance()->skins->getSelected();

				DLCPack * pDLCPack=tPack->getDLCPack();
				if(!pDLCPack->hasPurchasedFile( DLCManager::e_DLCType_Texture, L"" ))
				{
					// upsell
					// get the dlc texture pack

#ifdef _XBOX
					DLCTexturePack *pDLCTexPack=(DLCTexturePack *)tPack;
					ULONGLONG ullOfferID_Full;
					app.GetDLCFullOfferIDForPackID(pDLCTexPack->getDLCParentPackId(),&ullOfferID_Full);

					// tell sentient about the upsell of the full version of the skin pack
					TelemetryManager->RecordUpsellPresented(iPad, eSet_UpsellID_Texture_DLC, ullOfferID_Full & 0xFFFFFFFF);
#endif

					UINT uiIDA[2];
					uiIDA[0]=IDS_CONFIRM_OK;
					uiIDA[1]=IDS_CONFIRM_CANCEL;

					// Give the player a warning about the trial version of the texture pack
					ui.RequestErrorMessage(IDS_WARNING_DLC_TRIALTEXTUREPACK_TITLE, IDS_WARNING_DLC_TRIALTEXTUREPACK_TEXT, uiIDA, 2, iPad,&CMinecraftApp::WarningTrialTexturePackReturned,pClass);

					return S_OK;
				}
			}
#ifndef _XBOX_ONE
			// does the save exist?
			bool bSaveExists;
			StorageManager.DoesSaveExist(&bSaveExists);
			// 4J-PB - we check if the save exists inside the libs
			// we need to ask if they are sure they want to overwrite the existing game
			if(bSaveExists)
			{
				UINT uiIDA[2];
				uiIDA[0]=IDS_CONFIRM_CANCEL;
				uiIDA[1]=IDS_CONFIRM_OK;
				ui.RequestErrorMessage(IDS_TITLE_SAVE_GAME, IDS_CONFIRM_SAVE_GAME, uiIDA, 2, ProfileManager.GetPrimaryPad(),&CMinecraftApp::ExitAndJoinFromInviteAndSaveReturned,pClass);
				return 0;
			}
			else
#endif
			{
#if defined(_XBOX_ONE) || defined(__ORBIS__)
				StorageManager.SetSaveDisabled(false);
#endif
				MinecraftServer::getInstance()->setSaveOnExit( true );
			}
		}
		else
		{
			// been a few requests for a confirm on exit without saving
			UINT uiIDA[2];
			uiIDA[0]=IDS_CONFIRM_CANCEL;
			uiIDA[1]=IDS_CONFIRM_OK;
			ui.RequestErrorMessage(IDS_TITLE_DECLINE_SAVE_GAME, IDS_CONFIRM_DECLINE_SAVE_GAME, uiIDA, 2, ProfileManager.GetPrimaryPad(),&CMinecraftApp::ExitAndJoinFromInviteDeclineSaveReturned,pClass);
			return 0;
		}

		app.SetAction(ProfileManager.GetPrimaryPad(),eAppAction_ExitAndJoinFromInviteConfirmed);
	}
	return 0;
}

int CMinecraftApp::WarningTrialTexturePackReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	// 4J Stu - I added this in when fixing an X1 bug. We should probably add this as well but I don't have time to test all platforms atm
#if 0 //defined(__PS3__) || defined(__ORBIS__) || defined(__PSVITA__)
	if(result==C4JStorage::EMessage_ResultAccept)
	{
		if(!ProfileManager.IsSignedInLive(iPad))
		{
			// you're not signed in to PSN!

		}
		else
		{
			// 4J-PB - need to check this user can access the store
			bool bContentRestricted;
			ProfileManager.GetChatAndContentRestrictions(iPad,true,nullptr,&bContentRestricted,nullptr);
			if(bContentRestricted)
			{
				UINT uiIDA[1];
				uiIDA[0]=IDS_CONFIRM_OK;
				ui.RequestAlertMessage(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, iPad);
			}
			else
			{
				// need to get info on the pack to see if the user has already downloaded it
				TexturePack *tPack = Minecraft::GetInstance()->skins->getSelected();
				DLCTexturePack *pDLCTexPack=(DLCTexturePack *)tPack;

				// retrieve the store name for the skin pack
				DLCPack *pDLCPack=pDLCTexPack->getDLCInfoParentPack();//tPack->getDLCPack();
				const char *pchPackName=wstringtofilename(pDLCPack->getName());
				app.DebugPrintf("Texture Pack - %s\n",pchPackName);
				SONYDLC *pSONYDLCInfo=app.GetSONYDLCInfo((char *)pchPackName);

				if(pSONYDLCInfo!=nullptr)
				{
					char chName[42];
					char chSkuID[SCE_NP_COMMERCE2_SKU_ID_LEN];

					memset(chSkuID,0,SCE_NP_COMMERCE2_SKU_ID_LEN);
					// find the info on the skin pack
					// we have to retrieve the skuid from the store info, it can't be hardcoded since Sony may change it.
					// So we assume the first sku for the product is the one we want
#ifdef __ORBIS__
					sprintf(chName,"%s",pSONYDLCInfo->chDLCKeyname);
#else
					sprintf(chName,"%s-%s",app.GetCommerceCategory(),pSONYDLCInfo->chDLCKeyname);
#endif
					app.GetDLCSkuIDFromProductList(chName,chSkuID);
					// 4J-PB - need to check for an empty store
#if defined __ORBIS__ || defined __PSVITA__ || defined __PS3__
					if(app.CheckForEmptyStore(iPad)==false)
#endif
					{
						if(app.DLCAlreadyPurchased(chSkuID))
						{
							app.DownloadAlreadyPurchased(chSkuID);
						}
						else
						{
							app.Checkout(chSkuID);
						}
					}
				}
			}
		}
	}
#endif		//

#ifdef _XBOX_ONE
	if(result==C4JStorage::EMessage_ResultAccept)
	{
		if(ProfileManager.IsSignedIn(iPad))
		{
			if (ProfileManager.IsSignedInLive(iPad))
			{
				TexturePack *tPack = Minecraft::GetInstance()->skins->getSelected();
				// get the dlc texture pack
				DLCTexturePack *pDLCTexPack=(DLCTexturePack *)tPack;

				DLCPack *pDLCPack=pDLCTexPack->getDLCInfoParentPack();

				DLC_INFO *pDLCInfo=app.GetDLCInfoForProductName((WCHAR *)pDLCPack->getName().c_str());

				StorageManager.InstallOffer(1,(WCHAR *)pDLCInfo->wsProductId.c_str(),nullptr,nullptr);

				// the license change coming in when the offer has been installed will cause this scene to refresh
			}
			else
			{
				// 4J-JEV: Fix for XB1: #165863 - XR-074: Compliance: With no active network connection user is unable to convert from Trial to Full texture pack and is not messaged why.
				UINT uiIDA[1] = { IDS_CONFIRM_OK };
				ui.RequestErrorMessage(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_XBOXLIVE_NOTIFICATION, uiIDA, 1, iPad);
			}
		}
	}

#endif
#ifdef _XBOX

	CMinecraftApp* pClass = (CMinecraftApp*)pParam;

	TexturePack *tPack = Minecraft::GetInstance()->skins->getSelected();
	// get the dlc texture pack
	DLCTexturePack *pDLCTexPack=(DLCTexturePack *)tPack;
	ULONGLONG ullIndexA[1];

	// Need to get the parent packs id, since this may be one of many child packs with their own ids
	app.GetDLCFullOfferIDForPackID(pDLCTexPack->getDLCParentPackId(),&ullIndexA[0]);

	if(result==C4JStorage::EMessage_ResultAccept)
	{
		if(ProfileManager.IsSignedIn(iPad))
		{
			// need to allow downloads here, or the player would need to quit the game to let the download of a texture pack happen. This might affect the network traffic, since the download could take all the bandwidth...
			XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE_ALWAYS_ALLOW);

			StorageManager.InstallOffer(1,ullIndexA,nullptr,nullptr);
		}
	}
	else
	{
		TelemetryManager->RecordUpsellResponded(iPad, eSet_UpsellID_Texture_DLC, ( ullIndexA[0] & 0xFFFFFFFF ), eSen_UpsellOutcome_Declined);
	}
#endif
	return 0;
}

int CMinecraftApp::ExitAndJoinFromInviteAndSaveReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	//CMinecraftApp* pClass = (CMinecraftApp*)pParam;

	// results switched for this dialog
	if(result==C4JStorage::EMessage_ResultDecline)
	{
		INT saveOrCheckpointId = 0;

		// Check they have the full texture pack if they are using one
		// 4J-PB - Is the player trying to save but they are using a trial texturepack ?
		if(!Minecraft::GetInstance()->skins->isUsingDefaultSkin())
		{
			TexturePack *tPack = Minecraft::GetInstance()->skins->getSelected();

			DLCPack * pDLCPack=tPack->getDLCPack();
			if(!pDLCPack->hasPurchasedFile( DLCManager::e_DLCType_Texture, L"" ))
			{
				// upsell
				// get the dlc texture pack

#ifdef _XBOX
				DLCTexturePack *pDLCTexPack=(DLCTexturePack *)tPack;
				ULONGLONG ullOfferID_Full;
				app.GetDLCFullOfferIDForPackID(pDLCTexPack->getDLCParentPackId(),&ullOfferID_Full);

				// tell sentient about the upsell of the full version of the skin pack
				TelemetryManager->RecordUpsellPresented(iPad, eSet_UpsellID_Texture_DLC, ullOfferID_Full & 0xFFFFFFFF);
#endif

				UINT uiIDA[2];
				uiIDA[0]=IDS_CONFIRM_OK;
				uiIDA[1]=IDS_CONFIRM_CANCEL;

				// Give the player a warning about the trial version of the texture pack
				ui.RequestErrorMessage(IDS_WARNING_DLC_TRIALTEXTUREPACK_TITLE, IDS_WARNING_DLC_TRIALTEXTUREPACK_TEXT, uiIDA, 2, iPad,&CMinecraftApp::WarningTrialTexturePackReturned,nullptr);

				return S_OK;
			}
		}
		//bool validSave = StorageManager.GetSaveUniqueNumber(&saveOrCheckpointId);
		//SentientManager.RecordLevelSaveOrCheckpoint(ProfileManager.GetPrimaryPad(), saveOrCheckpointId);
		MinecraftServer::getInstance()->setSaveOnExit( true );
		// flag a app action of exit and join game from invite
		app.SetAction(iPad,eAppAction_ExitAndJoinFromInviteConfirmed);
	}
	return 0;
}

int CMinecraftApp::ExitAndJoinFromInviteDeclineSaveReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	// results switched for this dialog
	if(result==C4JStorage::EMessage_ResultDecline)
	{
#if defined(_XBOX_ONE) || defined(__ORBIS__)
		StorageManager.SetSaveDisabled(false);
#endif
		MinecraftServer::getInstance()->setSaveOnExit( false );
		// flag a app action of exit and join game from invite
		app.SetAction(iPad,eAppAction_ExitAndJoinFromInviteConfirmed);
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
//
// FatalLoadError
//
// This is called when we can't load one of the required files at startup
// It tends to mean the files have been corrupted.
// We have to assume that we've not been able to load the text for the game.
//
//////////////////////////////////////////////////////////////////////////
void CMinecraftApp::FatalLoadError()
{

}

TIPSTRUCT CMinecraftApp::m_GameTipA[MAX_TIPS_GAMETIP]=
{
	{ 0, IDS_TIPS_GAMETIP_1},
	{ 0, IDS_TIPS_GAMETIP_2},
	{ 0, IDS_TIPS_GAMETIP_3},
	{ 0, IDS_TIPS_GAMETIP_4},
	{ 0, IDS_TIPS_GAMETIP_5},
	{ 0, IDS_TIPS_GAMETIP_6},
	{ 0, IDS_TIPS_GAMETIP_7},
	{ 0, IDS_TIPS_GAMETIP_8},
	{ 0, IDS_TIPS_GAMETIP_9},
	{ 0, IDS_TIPS_GAMETIP_10},
	{ 0, IDS_TIPS_GAMETIP_11},
	{ 0, IDS_TIPS_GAMETIP_12},
	{ 0, IDS_TIPS_GAMETIP_13},
	{ 0, IDS_TIPS_GAMETIP_14},
	{ 0, IDS_TIPS_GAMETIP_15},
	{ 0, IDS_TIPS_GAMETIP_16},
	{ 0, IDS_TIPS_GAMETIP_17},
	{ 0, IDS_TIPS_GAMETIP_18},
	{ 0, IDS_TIPS_GAMETIP_19},
	{ 0, IDS_TIPS_GAMETIP_20},
	{ 0, IDS_TIPS_GAMETIP_21},
	{ 0, IDS_TIPS_GAMETIP_22},
	{ 0, IDS_TIPS_GAMETIP_23},
	{ 0, IDS_TIPS_GAMETIP_24},
	{ 0, IDS_TIPS_GAMETIP_25},
	{ 0, IDS_TIPS_GAMETIP_26},
	{ 0, IDS_TIPS_GAMETIP_27},
	{ 0, IDS_TIPS_GAMETIP_28},
	{ 0, IDS_TIPS_GAMETIP_29},
	{ 0, IDS_TIPS_GAMETIP_30},
	{ 0, IDS_TIPS_GAMETIP_31},
	{ 0, IDS_TIPS_GAMETIP_32},
	{ 0, IDS_TIPS_GAMETIP_33},
	{ 0, IDS_TIPS_GAMETIP_34},
	{ 0, IDS_TIPS_GAMETIP_35},
	{ 0, IDS_TIPS_GAMETIP_36},
	{ 0, IDS_TIPS_GAMETIP_37},
	{ 0, IDS_TIPS_GAMETIP_38},
	{ 0, IDS_TIPS_GAMETIP_39},
	{ 0, IDS_TIPS_GAMETIP_40},
	{ 0, IDS_TIPS_GAMETIP_41},
	{ 0, IDS_TIPS_GAMETIP_42},
	{ 0, IDS_TIPS_GAMETIP_43},
	{ 0, IDS_TIPS_GAMETIP_44},
	{ 0, IDS_TIPS_GAMETIP_45},
	{ 0, IDS_TIPS_GAMETIP_46},
	{ 0, IDS_TIPS_GAMETIP_47},
	{ 0, IDS_TIPS_GAMETIP_48},
	{ 0, IDS_TIPS_GAMETIP_49},
	{ 0, IDS_TIPS_GAMETIP_50},
};

TIPSTRUCT CMinecraftApp::m_TriviaTipA[MAX_TIPS_TRIVIATIP]=
{
	{ 0, IDS_TIPS_TRIVIA_1},
	{ 0, IDS_TIPS_TRIVIA_2},
	{ 0, IDS_TIPS_TRIVIA_3},
	{ 0, IDS_TIPS_TRIVIA_4},
	{ 0, IDS_TIPS_TRIVIA_5},
	{ 0, IDS_TIPS_TRIVIA_6},
	{ 0, IDS_TIPS_TRIVIA_7},
	{ 0, IDS_TIPS_TRIVIA_8},
	{ 0, IDS_TIPS_TRIVIA_9},
	{ 0, IDS_TIPS_TRIVIA_10},
	{ 0, IDS_TIPS_TRIVIA_11},
	{ 0, IDS_TIPS_TRIVIA_12},
	{ 0, IDS_TIPS_TRIVIA_13},
	{ 0, IDS_TIPS_TRIVIA_14},
	{ 0, IDS_TIPS_TRIVIA_15},
	{ 0, IDS_TIPS_TRIVIA_16},
	{ 0, IDS_TIPS_TRIVIA_17},
	{ 0, IDS_TIPS_TRIVIA_18},
	{ 0, IDS_TIPS_TRIVIA_19},
	{ 0, IDS_TIPS_TRIVIA_20},
};

Random *CMinecraftApp::TipRandom = new Random();

int CMinecraftApp::TipsSortFunction(const void* a, const void* b)
{
	return ((TIPSTRUCT*)a)->iSortValue - ((TIPSTRUCT*)b)->iSortValue;
}

void CMinecraftApp::InitialiseTips()
{
	// We'll randomise the tips at start up based on their priority

	ZeroMemory(m_TipIDA,sizeof(UINT)*MAX_TIPS_GAMETIP+MAX_TIPS_TRIVIATIP);

	// Make the first tip tell you that you can play splitscreen in HD modes if you are in SD
	if(!RenderManager.IsHiDef())
	{
		m_GameTipA[0].uiStringID=IDS_TIPS_GAMETIP_0;
	}
	// randomise then quicksort
	// going to leave the multiplayer tip so it is always first

	// Only randomise the content package build
#ifdef _CONTENT_PACKAGE

	for(int i=1;i<MAX_TIPS_GAMETIP;i++)
	{
		m_GameTipA[i].iSortValue=TipRandom->nextInt();
	}
	qsort( &m_GameTipA[1], MAX_TIPS_GAMETIP-1, sizeof(TIPSTRUCT), TipsSortFunction );
#endif

	for(int i=0;i<MAX_TIPS_TRIVIATIP;i++)
	{
		m_TriviaTipA[i].iSortValue=TipRandom->nextInt();
	}
	qsort( m_TriviaTipA, MAX_TIPS_TRIVIATIP, sizeof(TIPSTRUCT), TipsSortFunction );


	int iCurrentGameTip=0;
	int iCurrentTriviaTip=0;

	for(int i=0;i<MAX_TIPS_GAMETIP+MAX_TIPS_TRIVIATIP;i++)
	{
		// Add a trivia one every third tip (if there are any left)
		if((i%3==2) && (iCurrentTriviaTip<MAX_TIPS_TRIVIATIP))
		{
			// Add a trivia one
			m_TipIDA[i]=m_TriviaTipA[iCurrentTriviaTip++].uiStringID;
		}
		else
		{
			if(iCurrentGameTip<MAX_TIPS_GAMETIP)
			{
				// Add a gametip
				m_TipIDA[i]=m_GameTipA[iCurrentGameTip++].uiStringID;
			}
			else
			{
				// Add a trivia one
				m_TipIDA[i]=m_TriviaTipA[iCurrentTriviaTip++].uiStringID;
			}
		}

		if(m_TipIDA[i]==0)
		{
			// the m_TriviaTipA or the m_GameTipA are out of sync
#ifndef _CONTENT_PACKAGE
			__debugbreak();
#endif
		}
	}

	m_uiCurrentTip=0;
}

UINT CMinecraftApp::GetNextTip()
{
	static bool bShowSkinDLCTip=true;
	// don't display the DLC tip in the trial game
	if(ProfileManager.IsFullVersion() && app.GetNewDLCAvailable() && app.DisplayNewDLCTip())
	{
		return IDS_TIPS_GAMETIP_NEWDLC;
	}
	else
	{
		if(bShowSkinDLCTip && ProfileManager.IsFullVersion())
		{
			bShowSkinDLCTip=false;
			if( app.DLCInstallProcessCompleted() )
			{
				if(app.m_dlcManager.getPackCount(DLCManager::e_DLCType_Skin)==0)
				{
					return IDS_TIPS_GAMETIP_SKINPACKS;
				}
			}
			else
			{
				return IDS_TIPS_GAMETIP_SKINPACKS;
			}
		}
	}

	if(m_uiCurrentTip==MAX_TIPS_GAMETIP+MAX_TIPS_TRIVIATIP) m_uiCurrentTip=0;

	return m_TipIDA[m_uiCurrentTip++];
}

int CMinecraftApp::GetHTMLColour(eMinecraftColour colour)
{
	Minecraft *pMinecraft = Minecraft::GetInstance();
	return pMinecraft->skins->getSelected()->getColourTable()->getColour(colour);
}

int CMinecraftApp::GetHTMLFontSize(EHTMLFontSize size)
{
	return s_iHTMLFontSizesA[size];
}

wstring CMinecraftApp::FormatHTMLString(int iPad, const wstring &desc, int shadowColour /*= 0xFFFFFFFF*/)
{
	wstring text(desc);

	wchar_t replacements[64];
	// We will also insert line breaks here as couldn't figure out how to get them to come through from strings.resx !
	text = replaceAll(text, L"{*B*}", L"<br />" );
	swprintf(replacements,64,L"<font color=\"#%08x\">", GetHTMLColour(eHTMLColor_T1));
	text = replaceAll(text, L"{*T1*}", replacements );
	swprintf(replacements,64,L"<font color=\"#%08x\">", GetHTMLColour(eHTMLColor_T2));
	text = replaceAll(text, L"{*T2*}", replacements );
	swprintf(replacements,64,L"<font color=\"#%08x\">", GetHTMLColour(eHTMLColor_T3));
	text = replaceAll(text, L"{*T3*}", replacements ); // for How To Play
	swprintf(replacements,64,L"</font><font color=\"#%08x\">", GetHTMLColour(eHTMLColor_Black));
	text = replaceAll(text, L"{*ETB*}", replacements );
	swprintf(replacements,64,L"</font><font color=\"#%08x\">", GetHTMLColour(eHTMLColor_White));
	text = replaceAll(text, L"{*ETW*}", replacements );
	text = replaceAll(text, L"{*EF*}", L"</font>" );

	swprintf(replacements,64,L"<font color=\"#%08x\" shadowcolor=\"#%08x\">", GetHTMLColour(eHTMLColor_0), shadowColour);
	text = replaceAll(text, L"{*C0*}", replacements );
	swprintf(replacements,64,L"<font color=\"#%08x\" shadowcolor=\"#%08x\">", GetHTMLColour(eHTMLColor_1), shadowColour);
	text = replaceAll(text, L"{*C1*}", replacements );
	swprintf(replacements,64,L"<font color=\"#%08x\" shadowcolor=\"#%08x\">", GetHTMLColour(eHTMLColor_2), shadowColour);
	text = replaceAll(text, L"{*C2*}", replacements );
	swprintf(replacements,64,L"<font color=\"#%08x\" shadowcolor=\"#%08x\">", GetHTMLColour(eHTMLColor_3), shadowColour);
	text = replaceAll(text, L"{*C3*}", replacements );
	swprintf(replacements,64,L"<font color=\"#%08x\" shadowcolor=\"#%08x\">", GetHTMLColour(eHTMLColor_4), shadowColour);
	text = replaceAll(text, L"{*C4*}", replacements );
	swprintf(replacements,64,L"<font color=\"#%08x\" shadowcolor=\"#%08x\">", GetHTMLColour(eHTMLColor_5), shadowColour);
	text = replaceAll(text, L"{*C5*}", replacements );
	swprintf(replacements,64,L"<font color=\"#%08x\" shadowcolor=\"#%08x\">", GetHTMLColour(eHTMLColor_6), shadowColour);
	text = replaceAll(text, L"{*C6*}", replacements );
	swprintf(replacements,64,L"<font color=\"#%08x\" shadowcolor=\"#%08x\">", GetHTMLColour(eHTMLColor_7), shadowColour);
	text = replaceAll(text, L"{*C7*}", replacements );
	swprintf(replacements,64,L"<font color=\"#%08x\" shadowcolor=\"#%08x\">", GetHTMLColour(eHTMLColor_8), shadowColour);
	text = replaceAll(text, L"{*C8*}", replacements );
	swprintf(replacements,64,L"<font color=\"#%08x\" shadowcolor=\"#%08x\">", GetHTMLColour(eHTMLColor_9), shadowColour);
	text = replaceAll(text, L"{*C9*}", replacements );
	swprintf(replacements,64,L"<font color=\"#%08x\" shadowcolor=\"#%08x\">", GetHTMLColour(eHTMLColor_a), shadowColour);
	text = replaceAll(text, L"{*CA*}", replacements );
	swprintf(replacements,64,L"<font color=\"#%08x\" shadowcolor=\"#%08x\">", GetHTMLColour(eHTMLColor_b), shadowColour);
	text = replaceAll(text, L"{*CB*}", replacements );
	swprintf(replacements,64,L"<font color=\"#%08x\" shadowcolor=\"#%08x\">", GetHTMLColour(eHTMLColor_c), shadowColour);
	text = replaceAll(text, L"{*CC*}", replacements );
	swprintf(replacements,64,L"<font color=\"#%08x\" shadowcolor=\"#%08x\">", GetHTMLColour(eHTMLColor_d), shadowColour);
	text = replaceAll(text, L"{*CD*}", replacements );
	swprintf(replacements,64,L"<font color=\"#%08x\" shadowcolor=\"#%08x\">", GetHTMLColour(eHTMLColor_e), shadowColour);
	text = replaceAll(text, L"{*CE*}", replacements );
	swprintf(replacements,64,L"<font color=\"#%08x\" shadowcolor=\"#%08x\">", GetHTMLColour(eHTMLColor_f), shadowColour);
	text = replaceAll(text, L"{*CF*}", replacements );

	// Swap for southpaw.
	if ( app.GetGameSettings(iPad,eGameSetting_ControlSouthPaw) )
	{
		text = replaceAll(text, L"{*CONTROLLER_ACTION_MOVE*}",			GetActionReplacement(iPad,MINECRAFT_ACTION_LOOK_RIGHT		) );
		text = replaceAll(text, L"{*CONTROLLER_ACTION_LOOK*}",			GetActionReplacement(iPad,MINECRAFT_ACTION_RIGHT	) );

		text = replaceAll(text, L"{*CONTROLLER_MENU_NAVIGATE*}",		GetVKReplacement(VK_PAD_RTHUMB_LEFT) );
	}
	else // Normal right handed.
	{
		text = replaceAll(text, L"{*CONTROLLER_ACTION_MOVE*}",			GetActionReplacement(iPad,MINECRAFT_ACTION_RIGHT		) );
		text = replaceAll(text, L"{*CONTROLLER_ACTION_LOOK*}",			GetActionReplacement(iPad,MINECRAFT_ACTION_LOOK_RIGHT	) );

		text = replaceAll(text, L"{*CONTROLLER_MENU_NAVIGATE*}",		GetVKReplacement(VK_PAD_LTHUMB_LEFT) );
	}

	text = replaceAll(text, L"{*CONTROLLER_ACTION_JUMP*}",			GetActionReplacement(iPad,MINECRAFT_ACTION_JUMP			) );
	text = replaceAll(text, L"{*CONTROLLER_ACTION_SNEAK*}",			GetActionReplacement(iPad,MINECRAFT_ACTION_SNEAK_TOGGLE			) );
	text = replaceAll(text, L"{*CONTROLLER_ACTION_USE*}",			GetActionReplacement(iPad,MINECRAFT_ACTION_USE			) );
	text = replaceAll(text, L"{*CONTROLLER_ACTION_ACTION*}",		GetActionReplacement(iPad,MINECRAFT_ACTION_ACTION		) );
	text = replaceAll(text, L"{*CONTROLLER_ACTION_LEFT_SCROLL*}",	GetActionReplacement(iPad,MINECRAFT_ACTION_LEFT_SCROLL	) );
	text = replaceAll(text, L"{*CONTROLLER_ACTION_RIGHT_SCROLL*}",	GetActionReplacement(iPad,MINECRAFT_ACTION_RIGHT_SCROLL ) );
	text = replaceAll(text, L"{*CONTROLLER_ACTION_INVENTORY*}",		GetActionReplacement(iPad,MINECRAFT_ACTION_INVENTORY	) );
	text = replaceAll(text, L"{*CONTROLLER_ACTION_CRAFTING*}",		GetActionReplacement(iPad,MINECRAFT_ACTION_CRAFTING		) );
	text = replaceAll(text, L"{*CONTROLLER_ACTION_DROP*}",			GetActionReplacement(iPad,MINECRAFT_ACTION_DROP			) );
	text = replaceAll(text, L"{*CONTROLLER_ACTION_CAMERA*}",		GetActionReplacement(iPad,MINECRAFT_ACTION_RENDER_THIRD_PERSON	) );
	text = replaceAll(text, L"{*CONTROLLER_ACTION_MENU_PAGEDOWN*}",	GetActionReplacement(iPad,ACTION_MENU_PAGEDOWN	) );
	text = replaceAll(text, L"{*CONTROLLER_ACTION_DISMOUNT*}",		GetActionReplacement(iPad,MINECRAFT_ACTION_SNEAK_TOGGLE	) );
	text = replaceAll(text, L"{*CONTROLLER_VK_A*}",					GetVKReplacement(VK_PAD_A) );
	text = replaceAll(text, L"{*CONTROLLER_VK_B*}",					GetVKReplacement(VK_PAD_B) );
	text = replaceAll(text, L"{*CONTROLLER_VK_X*}",					GetVKReplacement(VK_PAD_X) );
	text = replaceAll(text, L"{*CONTROLLER_VK_Y*}",					GetVKReplacement(VK_PAD_Y) );
	text = replaceAll(text, L"{*CONTROLLER_VK_LB*}",				GetVKReplacement(VK_PAD_LSHOULDER) );
	text = replaceAll(text, L"{*CONTROLLER_VK_RB*}",				GetVKReplacement(VK_PAD_RSHOULDER) );
	text = replaceAll(text, L"{*CONTROLLER_VK_LS*}",				GetVKReplacement(VK_PAD_LTHUMB_UP) );
	text = replaceAll(text, L"{*CONTROLLER_VK_RS*}",				GetVKReplacement(VK_PAD_RTHUMB_UP) );
	text = replaceAll(text, L"{*CONTROLLER_VK_LT*}",				GetVKReplacement(VK_PAD_LTRIGGER) );
	text = replaceAll(text, L"{*CONTROLLER_VK_RT*}",				GetVKReplacement(VK_PAD_RTRIGGER) );
	text = replaceAll(text, L"{*ICON_SHANK_01*}",					GetIconReplacement(XZP_ICON_SHANK_01) );
	text = replaceAll(text, L"{*ICON_SHANK_03*}",					GetIconReplacement(XZP_ICON_SHANK_03) );
	text = replaceAll(text, L"{*CONTROLLER_ACTION_DPAD_UP*}",		GetActionReplacement(iPad,MINECRAFT_ACTION_DPAD_UP			) );
	text = replaceAll(text, L"{*CONTROLLER_ACTION_DPAD_DOWN*}",		GetActionReplacement(iPad,MINECRAFT_ACTION_DPAD_DOWN		) );
	text = replaceAll(text, L"{*CONTROLLER_ACTION_DPAD_RIGHT*}",	GetActionReplacement(iPad,MINECRAFT_ACTION_DPAD_RIGHT		) );
	text = replaceAll(text, L"{*CONTROLLER_ACTION_DPAD_LEFT*}",		GetActionReplacement(iPad,MINECRAFT_ACTION_DPAD_LEFT		) );
#if defined _XBOX_ONE || defined __PSVITA__
	text = replaceAll(text, L"{*CONTROLLER_VK_START*}",			GetVKReplacement(VK_PAD_START		) );
	text = replaceAll(text, L"{*CONTROLLER_VK_BACK*}",			GetVKReplacement(VK_PAD_BACK		) );
#endif

#ifdef _XBOX
	wstring imageRoot = L"";

	Minecraft *pMinecraft = Minecraft::GetInstance();
	imageRoot = pMinecraft->skins->getSelected()->getXuiRootPath();

	text = replaceAll(text, L"{*IMAGEROOT*}", imageRoot);
#endif // _XBOX

	// Fix for #8903 - UI: Localization: KOR/JPN/CHT: Button Icons are rendered with padding space, which looks no good
	DWORD dwLanguage = XGetLanguage( );
	switch(dwLanguage)
	{
	case XC_LANGUAGE_KOREAN:
	case XC_LANGUAGE_JAPANESE:
	case XC_LANGUAGE_TCHINESE:
		text = replaceAll(text, L"&nbsp;", L"" );
		break;
	}

	return text;
}

wstring CMinecraftApp::GetActionReplacement(int iPad, unsigned char ucAction)
{
	unsigned int input = InputManager.GetGameJoypadMaps(InputManager.GetJoypadMapVal(iPad) ,ucAction);

#ifdef _XBOX
	switch(input)
	{
	case _360_JOY_BUTTON_A:
		return app.GetString( IDS_CONTROLLER_A );
	case _360_JOY_BUTTON_B:
		return app.GetString( IDS_CONTROLLER_B );
	case _360_JOY_BUTTON_X:
		return app.GetString( IDS_CONTROLLER_X );
	case _360_JOY_BUTTON_Y:
		return app.GetString( IDS_CONTROLLER_Y );
	case _360_JOY_BUTTON_LSTICK_UP:
	case _360_JOY_BUTTON_LSTICK_DOWN:
	case _360_JOY_BUTTON_LSTICK_LEFT:
	case _360_JOY_BUTTON_LSTICK_RIGHT:
		return app.GetString( IDS_CONTROLLER_LEFT_STICK );
	case _360_JOY_BUTTON_RSTICK_LEFT:
	case _360_JOY_BUTTON_RSTICK_RIGHT:
	case _360_JOY_BUTTON_RSTICK_UP:
	case _360_JOY_BUTTON_RSTICK_DOWN:
		return app.GetString( IDS_CONTROLLER_RIGHT_STICK );
	case _360_JOY_BUTTON_LT:
		return app.GetString( IDS_CONTROLLER_LEFT_TRIGGER );
	case _360_JOY_BUTTON_RT:
		return app.GetString( IDS_CONTROLLER_RIGHT_TRIGGER );
	case _360_JOY_BUTTON_RB:
		return app.GetString( IDS_CONTROLLER_RIGHT_BUMPER );
	case _360_JOY_BUTTON_LB:
		return app.GetString( IDS_CONTROLLER_LEFT_BUMPER );
	case _360_JOY_BUTTON_BACK:
		return app.GetString( IDS_CONTROLLER_BACK );
	case _360_JOY_BUTTON_START:
		return app.GetString( IDS_CONTROLLER_START );
	case _360_JOY_BUTTON_RTHUMB:
		return app.GetString( IDS_CONTROLLER_RIGHT_THUMBSTICK );
	case _360_JOY_BUTTON_LTHUMB:
		return app.GetString( IDS_CONTROLLER_LEFT_THUMBSTICK );
	case _360_JOY_BUTTON_DPAD_LEFT:
		return app.GetString( IDS_CONTROLLER_DPAD_L );
	case _360_JOY_BUTTON_DPAD_RIGHT:
		return app.GetString( IDS_CONTROLLER_DPAD_R );
	case _360_JOY_BUTTON_DPAD_UP:
		return app.GetString( IDS_CONTROLLER_DPAD_U );
	case _360_JOY_BUTTON_DPAD_DOWN:
		return app.GetString( IDS_CONTROLLER_DPAD_D );
	};
	return L"";
#else
	wstring replacement = L"";

	// 4J Stu - Some of our actions can be mapped to multiple physical buttons, so replaces the switch that was here
	if (input & _360_JOY_BUTTON_A) replacement = L"ButtonA";
	else if(input &_360_JOY_BUTTON_B) replacement = L"ButtonB";
	else if(input &_360_JOY_BUTTON_X) replacement = L"ButtonX";
	else if(input &_360_JOY_BUTTON_Y) replacement = L"ButtonY";
	else if(
		(input &_360_JOY_BUTTON_LSTICK_UP) ||
		(input &_360_JOY_BUTTON_LSTICK_DOWN) ||
		(input &_360_JOY_BUTTON_LSTICK_LEFT) ||
		(input &_360_JOY_BUTTON_LSTICK_RIGHT)
		)
	{
		replacement = L"ButtonLeftStick";
	}
	else if(
		(input &_360_JOY_BUTTON_RSTICK_LEFT) ||
		(input &_360_JOY_BUTTON_RSTICK_RIGHT) ||
		(input &_360_JOY_BUTTON_RSTICK_UP) ||
		(input &_360_JOY_BUTTON_RSTICK_DOWN)
		)
	{
		replacement = L"ButtonRightStick";
	}
	else if(input &_360_JOY_BUTTON_DPAD_LEFT) replacement = L"ButtonDpadL";
	else if(input &_360_JOY_BUTTON_DPAD_RIGHT) replacement = L"ButtonDpadR";
	else if(input &_360_JOY_BUTTON_DPAD_UP) replacement = L"ButtonDpadU";
	else if(input &_360_JOY_BUTTON_DPAD_DOWN) replacement = L"ButtonDpadD";
	else if(input &_360_JOY_BUTTON_LT) replacement = L"ButtonLeftTrigger";
	else if(input &_360_JOY_BUTTON_RT) replacement = L"ButtonRightTrigger";
	else if(input &_360_JOY_BUTTON_RB) replacement = L"ButtonRightBumper";
	else if(input &_360_JOY_BUTTON_LB) replacement = L"ButtonLeftBumper";
	else if(input &_360_JOY_BUTTON_BACK) replacement = L"ButtonBack";
	else if(input &_360_JOY_BUTTON_START) replacement = L"ButtonStart";
	else if(input &_360_JOY_BUTTON_RTHUMB) replacement = L"ButtonRS";
	else if(input &_360_JOY_BUTTON_LTHUMB) replacement = L"ButtonLS";

	wchar_t string[128];

#ifdef __PS3__
	int size = 30;
#elif defined _WIN64
	int size = 45;
	if(ui.getScreenHeight() < 1080) size = 30;
#else
	int size = 45;
#endif

	swprintf(string,128,L"<img src=\"%ls\" align=\"middle\" height=\"%d\" width=\"%d\"/>", replacement.c_str(), size, size);

	return string;
#endif
}

wstring CMinecraftApp::GetVKReplacement(unsigned int uiVKey)
{
#ifdef _XBOX
	switch(uiVKey)
	{
	case VK_PAD_A:
		return app.GetString( IDS_CONTROLLER_A );
	case VK_PAD_B:
		return app.GetString( IDS_CONTROLLER_B );
	case VK_PAD_X:
		return app.GetString( IDS_CONTROLLER_X );
	case VK_PAD_Y:
		return app.GetString( IDS_CONTROLLER_Y );
	case VK_PAD_LSHOULDER:
		return app.GetString( IDS_CONTROLLER_LEFT_BUMPER );
	case VK_PAD_RSHOULDER:
		return app.GetString( IDS_CONTROLLER_RIGHT_BUMPER );
	case VK_PAD_LTRIGGER:
		return app.GetString( IDS_CONTROLLER_LEFT_TRIGGER );
	case VK_PAD_RTRIGGER:
		return app.GetString( IDS_CONTROLLER_RIGHT_TRIGGER );
	case VK_PAD_LTHUMB_UP       :
	case VK_PAD_LTHUMB_DOWN     :
	case VK_PAD_LTHUMB_RIGHT    :
	case VK_PAD_LTHUMB_LEFT     :
	case VK_PAD_LTHUMB_UPLEFT   :
	case VK_PAD_LTHUMB_UPRIGHT  :
	case VK_PAD_LTHUMB_DOWNRIGHT:
	case VK_PAD_LTHUMB_DOWNLEFT :
		return app.GetString( IDS_CONTROLLER_LEFT_STICK );
	case VK_PAD_RTHUMB_UP       :
	case VK_PAD_RTHUMB_DOWN     :
	case VK_PAD_RTHUMB_RIGHT    :
	case VK_PAD_RTHUMB_LEFT     :
	case VK_PAD_RTHUMB_UPLEFT   :
	case VK_PAD_RTHUMB_UPRIGHT  :
	case VK_PAD_RTHUMB_DOWNRIGHT:
	case VK_PAD_RTHUMB_DOWNLEFT :
		return app.GetString( IDS_CONTROLLER_RIGHT_STICK );
	default:
		break;
	}
	return nullptr;
#else
	wstring replacement = L"";
	switch(uiVKey)
	{
	case VK_PAD_A:
#if defined(__PS3__) || defined(__ORBIS__) || defined (__PSVITA__)
		if( InputManager.IsCircleCrossSwapped() ) replacement = L"ButtonB";
		else replacement = L"ButtonA";
#else
		replacement = L"ButtonA";
#endif
		break;
	case VK_PAD_B:
#if defined(__PS3__) || defined(__ORBIS__) || defined (__PSVITA__)
		if( InputManager.IsCircleCrossSwapped() ) replacement = L"ButtonA";
		else replacement = L"ButtonB";
#else
		replacement = L"ButtonB";
#endif
		break;
	case VK_PAD_X:
		replacement = L"ButtonX";
		break;
	case VK_PAD_Y:
		replacement = L"ButtonY";
		break;
	case VK_PAD_LSHOULDER:
		replacement = L"ButtonLeftBumper";
		break;
	case VK_PAD_RSHOULDER:
		replacement = L"ButtonRightBumper";
		break;
	case VK_PAD_LTRIGGER:
		replacement = L"ButtonLeftTrigger";
		break;
	case VK_PAD_RTRIGGER:
		replacement = L"ButtonRightTrigger";
		break;
	case VK_PAD_LTHUMB_UP       :
	case VK_PAD_LTHUMB_DOWN     :
	case VK_PAD_LTHUMB_RIGHT    :
	case VK_PAD_LTHUMB_LEFT     :
	case VK_PAD_LTHUMB_UPLEFT   :
	case VK_PAD_LTHUMB_UPRIGHT  :
	case VK_PAD_LTHUMB_DOWNRIGHT:
	case VK_PAD_LTHUMB_DOWNLEFT :
		replacement = L"ButtonLeftStick";
		break;
	case VK_PAD_RTHUMB_UP       :
	case VK_PAD_RTHUMB_DOWN     :
	case VK_PAD_RTHUMB_RIGHT    :
	case VK_PAD_RTHUMB_LEFT     :
	case VK_PAD_RTHUMB_UPLEFT   :
	case VK_PAD_RTHUMB_UPRIGHT  :
	case VK_PAD_RTHUMB_DOWNRIGHT:
	case VK_PAD_RTHUMB_DOWNLEFT :
		replacement = L"ButtonRightStick";
		break;
#if defined _XBOX_ONE || defined __PSVITA__
	case VK_PAD_START:
		replacement = L"ButtonStart";
		break;
	case VK_PAD_BACK:
		replacement = L"ButtonBack";
		break;
#endif
	default:
		break;
	}
	wchar_t string[128];

#ifdef __PS3__
	int size = 30;
#elif defined _WIN64
	int size = 45;
	if(ui.getScreenHeight() < 1080) size = 30;
#else
	int size = 45;
#endif

	swprintf(string,128,L"<img src=\"%ls\" align=\"middle\" height=\"%d\" width=\"%d\"/>", replacement.c_str(), size, size);

	return string;
#endif
}

wstring CMinecraftApp::GetIconReplacement(unsigned int uiIcon)
{
#ifdef _XBOX
	switch(uiIcon)
	{
	case XZP_ICON_SHANK_01:
		return app.GetString( IDS_ICON_SHANK_01 );
	case XZP_ICON_SHANK_03:
		return app.GetString( IDS_ICON_SHANK_03 );
	default:
		break;
	}
	return nullptr;
#else
	wchar_t string[128];

#ifdef __PS3__
	int size = 22;
#elif defined _WIN64
	int size = 33;
	if(ui.getScreenHeight() < 1080) size = 22;
#else
	int size = 33;
#endif

	swprintf(string,128,L"<img src=\"Icon_Shank\" align=\"middle\" height=\"%d\" width=\"%d\"/>", size, size);
	wstring result = L"";
	switch(uiIcon)
	{
	case XZP_ICON_SHANK_01:
		result = string;
		break;
	case XZP_ICON_SHANK_03:
		result.append(string).append(string).append(string);
		break;
	default:
		break;
	}
	return result;
#endif
}

#if defined(__PS3__) || defined(__ORBIS__) || defined (__PSVITA__)
unordered_map<PlayerUID, MOJANG_DATA *, PlayerUID::Hash> CMinecraftApp::MojangData;
unordered_map<int, char * >  CMinecraftApp::DLCTextures_PackID;
unordered_map<string, DLC_INFO * >  CMinecraftApp::DLCInfo;
unordered_map<wstring, ULONGLONG >  CMinecraftApp::DLCInfo_SkinName;
#elif defined(_DURANGO)
unordered_map<PlayerUID,MOJANG_DATA *, PlayerUID::Hash > CMinecraftApp::MojangData;
unordered_map<int, wstring >  CMinecraftApp::DLCTextures_PackID; // for mash-up packs & texture packs
//unordered_map<ULONGLONG,DLC_INFO * > CMinecraftApp::DLCInfo_Trial; // full offerid, dlc_info
unordered_map<wstring,DLC_INFO * > CMinecraftApp::DLCInfo_Full; // full offerid, dlc_info
unordered_map<wstring, wstring >  CMinecraftApp::DLCInfo_SkinName; // skin name, full offer id
#else
unordered_map<PlayerUID, MOJANG_DATA *> CMinecraftApp::MojangData;
unordered_map<int, ULONGLONG >  CMinecraftApp::DLCTextures_PackID;
unordered_map<ULONGLONG, DLC_INFO * >  CMinecraftApp::DLCInfo_Trial;
unordered_map<ULONGLONG, DLC_INFO * >  CMinecraftApp::DLCInfo_Full;
unordered_map<wstring, ULONGLONG >  CMinecraftApp::DLCInfo_SkinName;
#endif



HRESULT CMinecraftApp::RegisterMojangData(WCHAR *pXuidName, PlayerUID xuid, WCHAR *pSkin, WCHAR *pCape)
{
	HRESULT hr=S_OK;
	eXUID eTempXuid=eXUID_Undefined;
	MOJANG_DATA *pMojangData=nullptr;

	// ignore the names if we don't recognize them
    if (pXuidName != nullptr)
    {
		if( wcscmp( pXuidName, L"XUID_NOTCH" ) == 0 )
		{
			eTempXuid = eXUID_Notch; // might be needed for the apple at some point
		}
		else if( wcscmp( pXuidName, L"XUID_DEADMAU5" ) == 0 )
		{
			eTempXuid = eXUID_Deadmau5; // Needed for the deadmau5 ears
		}
		else
		{
			eTempXuid=eXUID_NoName;
		}
	}

	if(eTempXuid!=eXUID_Undefined)
	{
		pMojangData = new MOJANG_DATA;
		ZeroMemory(pMojangData,sizeof(MOJANG_DATA));
		pMojangData->eXuid=eTempXuid;

		wcsncpy( pMojangData->wchSkin, pSkin, MAX_CAPENAME_SIZE);
		wcsncpy( pMojangData->wchCape, pCape, MAX_CAPENAME_SIZE);
		MojangData[xuid]=pMojangData;
	}

	return hr;
}

MOJANG_DATA *CMinecraftApp::GetMojangDataForXuid(PlayerUID xuid)
{
	return MojangData[xuid];
}

HRESULT CMinecraftApp::RegisterConfigValues(WCHAR *pType, int iValue)
{
	HRESULT hr=S_OK;

	// #ifdef _XBOX
	// 	if(pType!=nullptr)
	// 	{	
	// 		if(wcscmp(pType,L"XboxOneTransfer")==0)
	// 		{
	// 			if(iValue>0)
	// 			{
	// 				app.m_bTransferSavesToXboxOne=true;
	// 			}
	// 			else
	// 			{
	// 				app.m_bTransferSavesToXboxOne=false;
	// 			}
	// 		}
	// 		else if(wcscmp(pType,L"TransferSlotCount")==0)
	// 		{
	// 			app.m_uiTransferSlotC=iValue;
	// 		}
	//
	// 	}
	// #endif


	return hr;
}

#if (defined _XBOX || defined _WINDOWS64)
HRESULT CMinecraftApp::RegisterDLCData(WCHAR *pType, WCHAR *pBannerName, int iGender, uint64_t ullOfferID_Full, uint64_t ullOfferID_Trial, WCHAR *pFirstSkin, unsigned int uiSortIndex, int iConfig, WCHAR *pDataFile)
{
	HRESULT hr=S_OK;
	DLC_INFO *pDLCData=new DLC_INFO;
	ZeroMemory(pDLCData,sizeof(DLC_INFO));
	pDLCData->ullOfferID_Full=ullOfferID_Full;
	pDLCData->ullOfferID_Trial=ullOfferID_Trial;
	pDLCData->eDLCType=e_DLC_NotDefined;
	pDLCData->iGender=iGender;
	pDLCData->uiSortIndex=uiSortIndex;
	pDLCData->iConfig=iConfig;

#ifndef __ORBIS__
	// ignore the names if we don't recognize them
	if(pBannerName!=L"")
	{
		wcsncpy_s( pDLCData->wchBanner, pBannerName, MAX_BANNERNAME_SIZE);
	}

	if(pDataFile[0]!=0)
	{
		wcsncpy_s( pDLCData->wchDataFile, pDataFile, MAX_BANNERNAME_SIZE);
	}
#endif

	if(pType!=nullptr)
	{	
		if(wcscmp(pType,L"Skin")==0)
		{
			pDLCData->eDLCType=e_DLC_SkinPack;
		}
		else if(wcscmp(pType,L"Gamerpic")==0)
		{
			pDLCData->eDLCType=e_DLC_Gamerpics;
		}
		else if(wcscmp(pType,L"Theme")==0)
		{
			pDLCData->eDLCType=e_DLC_Themes;
		}
		else if(wcscmp(pType,L"Avatar")==0)
		{
			pDLCData->eDLCType=e_DLC_AvatarItems;
		}
		else if(wcscmp(pType,L"MashUpPack")==0)
		{
			pDLCData->eDLCType=e_DLC_MashupPacks;
			DLCTextures_PackID[pDLCData->iConfig]=ullOfferID_Full;
		}
		else if(wcscmp(pType,L"TexturePack")==0)
		{
			pDLCData->eDLCType=e_DLC_TexturePacks;
			DLCTextures_PackID[pDLCData->iConfig]=ullOfferID_Full;
		}


	}

	if(ullOfferID_Trial!=0ll) DLCInfo_Trial[ullOfferID_Trial]=pDLCData;
	if(ullOfferID_Full!=0ll) DLCInfo_Full[ullOfferID_Full]=pDLCData;
	if(pFirstSkin[0]!=0) DLCInfo_SkinName[pFirstSkin]=ullOfferID_Full;

	return hr;
}
#elif defined _XBOX_ONE

unordered_map<wstring,DLC_INFO * > *CMinecraftApp::GetDLCInfo()
{
	return &DLCInfo_Full;
}

HRESULT CMinecraftApp::RegisterDLCData(eDLCContentType eType, WCHAR *pwchBannerName,WCHAR *pwchProductId, WCHAR *pwchProductName, WCHAR *pwchFirstSkin, int iConfig, unsigned int uiSortIndex)
{
	HRESULT hr=S_OK;
	// 4J-PB - need to convert the product id to uppercase because the catalog calls come back with upper case
	WCHAR wchUppercaseProductID[64];
	if(pwchProductId[0]!=0)
	{
		for(int i=0;i<64;i++)
		{
			wchUppercaseProductID[i]=towupper((wchar_t)pwchProductId[i]);
		}
	}

	// check if we already have this info from the local DLC file
	wstring wsTemp=wchUppercaseProductID;

	auto it = DLCInfo_Full.find(wsTemp);
	if( it == DLCInfo_Full.end() )
	{
		// Not found

		DLC_INFO *pDLCData=new DLC_INFO;
		ZeroMemory(pDLCData,sizeof(DLC_INFO));

		pDLCData->eDLCType=e_DLC_NotDefined;
		pDLCData->uiSortIndex=uiSortIndex;
		pDLCData->iConfig=iConfig;

		if(pwchProductId[0]!=0)
		{
			pDLCData->wsProductId=wchUppercaseProductID;
		}

		// ignore the names if we don't recognize them
		if(pwchBannerName!=L"")
		{
			wcsncpy_s( pDLCData->wchBanner, pwchBannerName, MAX_BANNERNAME_SIZE);
		}

		if(pwchProductName[0]!=0)
		{
			pDLCData->wsDisplayName=pwchProductName;
		}

		pDLCData->eDLCType=eType;

		switch(eType)
		{
		case e_DLC_MashupPacks:
		case e_DLC_TexturePacks:
			DLCTextures_PackID[iConfig]=pDLCData->wsProductId;
			break;
		}

		if(pwchFirstSkin[0]!=0) DLCInfo_SkinName[pwchFirstSkin]=pDLCData->wsProductId;

#ifdef _XBOX_ONE
		// ignore the names, and use the product id instead
		DLCInfo_Full[pDLCData->wsProductId]=pDLCData;
#else
		DLCInfo_Full[pDLCData->wsDisplayName]=pDLCData;
#endif
	}
	app.DebugPrintf("DLCInfo - type - %d, productID - %ls, name - %ls , banner - %ls, iconfig - %d, sort index - %d\n",eType,pwchProductId, pwchProductName,pwchBannerName, iConfig, uiSortIndex);
	return hr;
}
#else

HRESULT CMinecraftApp::RegisterDLCData(char *pchDLCName, unsigned int uiSortIndex,char *pchImageURL)
{
	// on PS3 we get all the required info from the name
	char chDLCType[3];
	HRESULT hr=S_OK;
	DLC_INFO *pDLCData=new DLC_INFO;
	ZeroMemory(pDLCData,sizeof(DLC_INFO));

	chDLCType[0]=pchDLCName[0];
	chDLCType[1]=pchDLCName[1];
	chDLCType[2]=0;

	pDLCData->iConfig = app.GetiConfigFromName(pchDLCName);
	pDLCData->uiSortIndex=uiSortIndex;
	pDLCData->eDLCType = app.GetDLCTypeFromName(pchDLCName);
	strcpy(pDLCData->chImageURL,pchImageURL);
	//bool bIsTrialDLC = app.GetTrialFromName(pchDLCName);

	switch(pDLCData->eDLCType)
	{
	case e_DLC_TexturePacks:
		{
			char *pchName=(char *)malloc(strlen(pchDLCName)+1);
			strcpy(pchName,pchDLCName);
			DLCTextures_PackID[pDLCData->iConfig]=pchName;
		}
		break;
	case e_DLC_MashupPacks:
		{
			char *pchName=(char *)malloc(strlen(pchDLCName)+1);
			strcpy(pchName,pchDLCName);
			DLCTextures_PackID[pDLCData->iConfig]=pchName;
		}
		break;
	default:
		break;
	}

	app.DebugPrintf(5,"Adding DLC - %s\n",pchDLCName);
	DLCInfo[pchDLCName]=pDLCData;

	// 	if(ullOfferID_Trial!=0ll) DLCInfo_Trial[ullOfferID_Trial]=pDLCData;
	// 	if(ullOfferID_Full!=0ll) DLCInfo_Full[ullOfferID_Full]=pDLCData;
	// 	if(pFirstSkin[0]!=0) DLCInfo_SkinName[pFirstSkin]=ullOfferID_Full;

	//	DLCInfo[ullOfferID_Trial]=pDLCData;

	return hr;
}
#endif



#if defined( __PS3__) || defined(__ORBIS__) || defined(__PSVITA__)
bool CMinecraftApp::GetDLCFullOfferIDForSkinID(const wstring &FirstSkin,ULONGLONG *pullVal)
{
	auto it = DLCInfo_SkinName.find(FirstSkin);
	if( it == DLCInfo_SkinName.end() )
	{
		return false;
	}
	else
	{
		*pullVal=(ULONGLONG)it->second;
		return true;
	}
}
bool CMinecraftApp::GetDLCNameForPackID(const int iPackID,char **ppchKeyID)
{
	auto it = DLCTextures_PackID.find(iPackID);
	if( it == DLCTextures_PackID.end() )
	{
		*ppchKeyID=nullptr;
		return false;
	}
	else
	{
		*ppchKeyID=(char *)it->second;
		return true;
	}
}
DLC_INFO *CMinecraftApp::GetDLCInfo(char *pchDLCName)
{
	string tempString=pchDLCName;

	if(DLCInfo.size()>0)
	{
		auto it = DLCInfo.find(tempString);

		if( it == DLCInfo.end() )
		{
			// nothing for this
			return nullptr;
		}
		else
		{
			return it->second;
		}
	}
	else return nullptr;
}

DLC_INFO *CMinecraftApp::GetDLCInfoFromTPackID(int iTPID)
{
	unordered_map<string, DLC_INFO *>::iterator it= DLCInfo.begin();

	for(size_t i=0;i<DLCInfo.size();i++)
	{
		if(((DLC_INFO *)it->second)->iConfig==iTPID)
		{
			return it->second;
		}
		++it;
	}
	return nullptr;
}

DLC_INFO *CMinecraftApp::GetDLCInfo(int iIndex)
{
	unordered_map<string, DLC_INFO *>::iterator it= DLCInfo.begin();

	for(int i=0;i<iIndex;i++)
	{
		++it;
	}

	return it->second;
}

char *CMinecraftApp::GetDLCInfoTextures(int iIndex)
{
	unordered_map<int, char * >::iterator it= DLCTextures_PackID.begin();

	for(int i=0;i<iIndex;i++)
	{
		++it;
	}

	return it->second;
}

#elif defined _XBOX_ONE
bool CMinecraftApp::GetDLCFullOfferIDForSkinID(const wstring &FirstSkin,wstring &ProductId)
{
	auto it = DLCInfo_SkinName.find(FirstSkin);
	if( it == DLCInfo_SkinName.end() )
	{
		return false;
	}
	else
	{
		ProductId=it->second;
		return true;
	}
}
bool CMinecraftApp::GetDLCFullOfferIDForPackID(const int iPackID,wstring &ProductId)
{
	auto it = DLCTextures_PackID.find(iPackID);
	if( it == DLCTextures_PackID.end() )
	{
		return false;
	}
	else
	{
		ProductId=it->second;
		return true;
	}
}
// DLC_INFO *CMinecraftApp::GetDLCInfoForTrialOfferID(wstring &ProductId)
// {
// 	return nullptr;
// }

DLC_INFO *CMinecraftApp::GetDLCInfoTrialOffer(int iIndex)
{
	return nullptr;
}
DLC_INFO *CMinecraftApp::GetDLCInfoFullOffer(int iIndex)
{
	unordered_map<wstring, DLC_INFO *>::iterator it= DLCInfo_Full.begin();

	for(int i=0;i<iIndex;i++)
	{
		++it;
	}

	return it->second;
}
wstring CMinecraftApp::GetDLCInfoTexturesFullOffer(int iIndex)
{
	unordered_map<int, wstring >::iterator it= DLCTextures_PackID.begin();

	for(int i=0;i<iIndex;i++)
	{
		++it;
	}

	return it->second;
}
#else
bool CMinecraftApp::GetDLCFullOfferIDForSkinID(const wstring &FirstSkin,ULONGLONG *pullVal)
{
	auto it = DLCInfo_SkinName.find(FirstSkin);
	if( it == DLCInfo_SkinName.end() )
	{
		return false;
	}
	else
	{
		*pullVal=(ULONGLONG)it->second;
		return true;
	}
}
bool CMinecraftApp::GetDLCFullOfferIDForPackID(const int iPackID,ULONGLONG *pullVal)
{
	auto it = DLCTextures_PackID.find(iPackID);
	if( it == DLCTextures_PackID.end() )
	{
		*pullVal=0ULL;
		return false;
	}
	else
	{
		*pullVal=it->second;
		return true;
	}
}
DLC_INFO *CMinecraftApp::GetDLCInfoForTrialOfferID(ULONGLONG ullOfferID_Trial)
{
	//DLC_INFO *pDLCInfo=nullptr;
	if(DLCInfo_Trial.size()>0)
	{
		auto it = DLCInfo_Trial.find(ullOfferID_Trial);

		if( it == DLCInfo_Trial.end() )
		{
			// nothing for this
			return nullptr;
		}
		else
		{
			return it->second;
		}
	}
	else return nullptr;
}

DLC_INFO *CMinecraftApp::GetDLCInfoTrialOffer(int iIndex)
{
	unordered_map<ULONGLONG, DLC_INFO *>::iterator it= DLCInfo_Trial.begin();

	for(int i=0;i<iIndex;i++)
	{
		++it;
	}

	return it->second;
}
DLC_INFO *CMinecraftApp::GetDLCInfoFullOffer(int iIndex)
{
	unordered_map<ULONGLONG, DLC_INFO *>::iterator it= DLCInfo_Full.begin();

	for(int i=0;i<iIndex;i++)
	{
		++it;
	}

	return it->second;
}
ULONGLONG CMinecraftApp::GetDLCInfoTexturesFullOffer(int iIndex)
{
	unordered_map<int, ULONGLONG >::iterator it= DLCTextures_PackID.begin();

	for(int i=0;i<iIndex;i++)
	{
		++it;
	}

	return it->second;
}
#endif

#ifdef _XBOX_ONE

DLC_INFO *CMinecraftApp::GetDLCInfoForFullOfferID(WCHAR *pwchProductID)
{
	wstring wsTemp = pwchProductID;
	if(DLCInfo_Full.size()>0)
	{
		auto it = DLCInfo_Full.find(wsTemp);

		if( it == DLCInfo_Full.end() )
		{
			// nothing for this
			return nullptr;
		}
		else
		{
			return it->second;
		}
	}
	else return nullptr;
}
DLC_INFO *CMinecraftApp::GetDLCInfoForProductName(WCHAR *pwchProductName)
{
	unordered_map<wstring, DLC_INFO *>::iterator it= DLCInfo_Full.begin();
	wstring wsProductName=pwchProductName;

	for(size_t i=0;i<DLCInfo_Full.size();i++)
	{
		DLC_INFO *pDLCInfo=(DLC_INFO *)it->second;
		if(wsProductName==pDLCInfo->wsDisplayName)
		{
			return pDLCInfo;
		}
		++it;
	}

	return nullptr;
}

#elif defined(__PS3__) || defined(__ORBIS__) || defined (__PSVITA__)
#else

DLC_INFO *CMinecraftApp::GetDLCInfoForFullOfferID(ULONGLONG ullOfferID_Full)
{

	if(DLCInfo_Full.size()>0)
	{
		auto it = DLCInfo_Full.find(ullOfferID_Full);

		if( it == DLCInfo_Full.end() )
		{
			// nothing for this
			return nullptr;
		}
		else
		{
			return it->second;
		}
	}
	else return nullptr;
}
#endif

void CMinecraftApp::EnterSaveNotificationSection()
{
	EnterCriticalSection(&m_saveNotificationCriticalSection);
	if( m_saveNotificationDepth++ == 0 )
	{
		if(g_NetworkManager.IsInSession())  // this can be triggered from the front end if we're downloading a save
		{
		MinecraftServer::getInstance()->broadcastStartSavingPacket();

		if( g_NetworkManager.IsLocalGame() && g_NetworkManager.GetPlayerCount() == 1 )
		{
			app.SetXuiServerAction(ProfileManager.GetPrimaryPad(),eXuiServerAction_PauseServer,(void *)TRUE);
		}
	}
	}
	LeaveCriticalSection(&m_saveNotificationCriticalSection);
}

void CMinecraftApp::LeaveSaveNotificationSection()
{
	EnterCriticalSection(&m_saveNotificationCriticalSection);
	if( --m_saveNotificationDepth == 0 )
	{
		if(g_NetworkManager.IsInSession())  // this can be triggered from the front end if we're downloading a save
		{
		MinecraftServer::getInstance()->broadcastStopSavingPacket();

		if( g_NetworkManager.IsLocalGame() && g_NetworkManager.GetPlayerCount() == 1 )
		{
			app.SetXuiServerAction(ProfileManager.GetPrimaryPad(),eXuiServerAction_PauseServer,(void *)FALSE);
		}
	}
	}
	LeaveCriticalSection(&m_saveNotificationCriticalSection);
}


int CMinecraftApp::RemoteSaveThreadProc( void* lpParameter )
{
	// The game should be stopped while we are doing this, but the connections ticks may try to create some AABB's or Vec3's
	AABB::UseDefaultThreadStorage();
	Vec3::UseDefaultThreadStorage();
	Compression::UseDefaultThreadStorage();

	// 4J-PB - Xbox 360 - 163153 - [CRASH] TU17: Code: Multiplayer: During the Autosave in an online Multiplayer session, the game occasionally crashes for one or more Clients
	// callstack - >	if(tls->tileId != this->id) updateDefaultShape();
	// callstack - >	default.exe!WaterlilyTile::getAABB(Level * level, int x, int y, int z)  line 38 + 8 bytes	C++
	// ...
	//  	default.exe!CMinecraftApp::RemoteSaveThreadProc(void * lpParameter)  line 6694	C++
	// host autosave, and the clients can crash on receiving handleMoveEntity when it's a tile within this thread, so need to do the tls for tiles
	Tile::CreateNewThreadStorage();

	Minecraft *pMinecraft = Minecraft::GetInstance();

	pMinecraft->progressRenderer->progressStartNoAbort( IDS_PROGRESS_HOST_SAVING );
	pMinecraft->progressRenderer->progressStage( -1 );
	pMinecraft->progressRenderer->progressStagePercentage(0);

	while( !app.GetGameStarted() && app.GetXuiAction( ProfileManager.GetPrimaryPad() ) == eAppAction_WaitRemoteServerSaveComplete )
	{
		// Tick all the games connections
		pMinecraft->tickAllConnections();
		Sleep( 100 );
	}

	if( app.GetXuiAction( ProfileManager.GetPrimaryPad() ) != eAppAction_WaitRemoteServerSaveComplete )
	{
		// Something cancelled us?
		return ERROR_CANCELLED;
	}
	app.SetAction(ProfileManager.GetPrimaryPad(),eAppAction_Idle);

	ui.UpdatePlayerBasePositions();

	Tile::ReleaseThreadStorage();

	return S_OK;
}

void CMinecraftApp::ExitGameFromRemoteSave( LPVOID lpParameter )
{
	int primaryPad = ProfileManager.GetPrimaryPad();

	UINT uiIDA[3];
	uiIDA[0]=IDS_CONFIRM_CANCEL;
	uiIDA[1]=IDS_CONFIRM_OK;

	ui.RequestAlertMessage(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME, uiIDA, 2, primaryPad,&CMinecraftApp::ExitGameFromRemoteSaveDialogReturned,nullptr);
}

int CMinecraftApp::ExitGameFromRemoteSaveDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	//CScene_Pause* pClass = (CScene_Pause*)pParam;

	// results switched for this dialog
	if(result==C4JStorage::EMessage_ResultDecline)
	{
		app.SetAction(iPad,eAppAction_ExitWorld);
	}
	else
	{
#ifndef _XBOX
		// Inform fullscreen progress scene that it's not being cancelled after all
		UIScene_FullscreenProgress *pScene = static_cast<UIScene_FullscreenProgress *>(ui.FindScene(eUIScene_FullscreenProgress));
#ifdef __PS3__
		if(pScene!=nullptr)
#else
		if (pScene != nullptr)
#endif
		{
			pScene->SetWasCancelled(false);
		}
#else
		// Don't have to worry about this on Xbox
#endif
	}
	return 0;
}

void CMinecraftApp::SetSpecialTutorialCompletionFlag(int iPad, int index)
{
	if(index >= 0 && index < 32 && GameSettingsA[iPad] != nullptr)
	{
		GameSettingsA[iPad]->uiSpecialTutorialBitmask |= (1<<index);
	}
}

// BANNED LIST FUNCTIONS

void CMinecraftApp::SetUniqueMapName(char *pszUniqueMapName)
{
	memcpy(m_pszUniqueMapName,pszUniqueMapName,14);
}

char *CMinecraftApp::GetUniqueMapName(void)
{
	return m_pszUniqueMapName;
}

void CMinecraftApp::InvalidateBannedList(int iPad)
{
	if(m_bRead_BannedListA[iPad]==true)
	{
		m_bRead_BannedListA[iPad]=false;
		SetBanListCheck(iPad,false);
		m_vBannedListA[iPad]->clear();

		if(BannedListA[iPad].pBannedList)
		{
			delete [] BannedListA[iPad].pBannedList;
			BannedListA[iPad].pBannedList=nullptr;
		}
	}
}

#ifdef _XBOX_ONE
void CMinecraftApp::AddLevelToBannedLevelList(int iPad, PBANNEDLISTDATA pBannedListData, bool bWriteToTMS)
{
	PlayerUID xuid= pBannedListData->wchPlayerUID;

	AddLevelToBannedLevelList(iPad,xuid,pBannedListData->pszLevelName,bWriteToTMS);
}
#endif

void CMinecraftApp::AddLevelToBannedLevelList(int iPad, PlayerUID xuid, char *pszLevelName, bool bWriteToTMS)
{
	// we will have retrieved the banned level list from TMS, so add this one to it and write it back to TMS

	BANNEDLISTDATA *pBannedListData = new BANNEDLISTDATA;
	memset(pBannedListData,0,sizeof(BANNEDLISTDATA));

#ifdef _DURANGO
	memcpy(&pBannedListData->wchPlayerUID, xuid.toString().c_str(), sizeof(WCHAR)*64);
#else
	memcpy(&pBannedListData->xuid, &xuid, sizeof(PlayerUID));
#endif
	strcpy(pBannedListData->pszLevelName,pszLevelName);
	m_vBannedListA[iPad]->push_back(pBannedListData);

    if (bWriteToTMS)
    {
        DWORD dwDataBytes = static_cast<DWORD>(sizeof(BANNEDLISTDATA)* m_vBannedListA[iPad]->size());
		PBANNEDLISTDATA pBannedList = reinterpret_cast<BANNEDLISTDATA*>(new CHAR [dwDataBytes]);
		int iCount=0;
		for (PBANNEDLISTDATA pData : *m_vBannedListA[iPad] )
		{
			memcpy(&pBannedList[iCount++],pData,sizeof(BANNEDLISTDATA));
		}

		// 4J-PB - write to TMS++ now

		//bool bRes=StorageManager.WriteTMSFile(iPad,C4JStorage::eGlobalStorage_TitleUser,L"BannedList",(PBYTE)pBannedList, dwDataBytes);
#ifdef _XBOX
		StorageManager.TMSPP_WriteFile(iPad,C4JStorage::eGlobalStorage_TitleUser,C4JStorage::TMS_FILETYPE_BINARY,C4JStorage::TMS_UGCTYPE_NONE,"BannedList",(PCHAR) pBannedList, dwDataBytes,nullptr,nullptr, 0);
#elif defined _XBOX_ONE
		StorageManager.TMSPP_WriteFile(iPad,C4JStorage::eGlobalStorage_TitleUser,C4JStorage::TMS_FILETYPE_BINARY,L"BannedList",(PBYTE) pBannedList, dwDataBytes,nullptr,nullptr, 0);
#endif
	}
	// update telemetry too
}

bool CMinecraftApp::IsInBannedLevelList(int iPad, PlayerUID xuid, char *pszLevelName)
{
	for( PBANNEDLISTDATA pData : *m_vBannedListA[iPad] )
	{
#ifdef _XBOX_ONE
		PlayerUID bannedPlayerUID = pData->wchPlayerUID;
		if(IsEqualXUID (bannedPlayerUID,xuid) && (strcmp(pData->pszLevelName,pszLevelName)==0))
#else
		if(IsEqualXUID (pData->xuid,xuid) && (strcmp(pData->pszLevelName,pszLevelName)==0))
#endif
		{
			return true;
		}
	}

	return false;
}

void CMinecraftApp::RemoveLevelFromBannedLevelList(int iPad, PlayerUID xuid, char *pszLevelName)
{
	//bool bFound=false;
	//bool bRes;

	// we will have retrieved the banned level list from TMS, so remove this one from it and write it back to TMS
	for (auto it = m_vBannedListA[iPad]->begin(); it != m_vBannedListA[iPad]->end(); )
	{
		PBANNEDLISTDATA pBannedListData = *it;

		if(pBannedListData!=nullptr)
		{
#ifdef _XBOX_ONE
			PlayerUID bannedPlayerUID = pBannedListData->wchPlayerUID;
			if(IsEqualXUID (bannedPlayerUID,xuid) && (strcmp(pBannedListData->pszLevelName,pszLevelName)==0))
#else
			if(IsEqualXUID (pBannedListData->xuid,xuid) && (strcmp(pBannedListData->pszLevelName,pszLevelName)==0))
#endif
			{
				TelemetryManager->RecordUnBanLevel(iPad);

				// match found, so remove this entry
				it = m_vBannedListA[iPad]->erase(it);
			}
			else
			{
				++it;
			}
		}
		else
		{
			++it;
		}
	}

	DWORD dwDataBytes=static_cast<DWORD>(sizeof(BANNEDLISTDATA) * m_vBannedListA[iPad]->size());
	if(dwDataBytes==0)
	{
		// wipe the file
#ifdef _XBOX
		StorageManager.DeleteTMSFile(iPad,C4JStorage::eGlobalStorage_TitleUser,L"BannedList");
#elif defined _XBOX_ONE
		StorageManager.TMSPP_DeleteFile(iPad,C4JStorage::eGlobalStorage_TitleUser,C4JStorage::TMS_FILETYPE_BINARY,L"BannedList",nullptr,nullptr, 0);
#endif
	}
	else
	{
		PBANNEDLISTDATA pBannedList = (BANNEDLISTDATA *)(new BYTE [dwDataBytes]);

		size_t iSize=m_vBannedListA[iPad]->size();
		for(size_t i=0;i<iSize;i++)
		{
			PBANNEDLISTDATA pBannedListData =m_vBannedListA[iPad]->at(i);

			memcpy(&pBannedList[i],pBannedListData,sizeof(BANNEDLISTDATA));
		}
#ifdef _XBOX
		StorageManager.WriteTMSFile(iPad,C4JStorage::eGlobalStorage_TitleUser,L"BannedList",(PBYTE)pBannedList, dwDataBytes);
#elif defined _XBOX_ONE
		StorageManager.TMSPP_WriteFile(iPad,C4JStorage::eGlobalStorage_TitleUser,C4JStorage::TMS_FILETYPE_BINARY,L"BannedList",(PBYTE) pBannedList, dwDataBytes,nullptr,nullptr, 0);
#endif
		delete [] pBannedList;
	}

	// update telemetry too
}

// function to add credits for the DLC packs
void CMinecraftApp::AddCreditText(LPCWSTR lpStr)
{
	DebugPrintf("ADDING CREDIT - %ls\n",lpStr);
	// add a string from the DLC to a credits vector
	SCreditTextItemDef *pCreditStruct = new SCreditTextItemDef;
	pCreditStruct->m_eType=eSmallText;
	pCreditStruct->m_iStringID[0]=NO_TRANSLATED_STRING;
	pCreditStruct->m_iStringID[1]=NO_TRANSLATED_STRING;
	pCreditStruct->m_Text=new WCHAR [wcslen(lpStr)+1];
	wcscpy((WCHAR *)pCreditStruct->m_Text,lpStr);

	vDLCCredits.push_back(pCreditStruct);
}

bool CMinecraftApp::AlreadySeenCreditText(const wstring &wstemp)
{

	for(unsigned int i=0;i<m_vCreditText.size();i++)
	{
		wstring temp=m_vCreditText.at(i);

		// if they are the same, break out of the case
		if(temp.compare(wstemp)==0)
		{
			return true;
		}
	}

	// add this text
	m_vCreditText.push_back((WCHAR * )wstemp.c_str());
	return false;
}

unsigned int CMinecraftApp::GetDLCCreditsCount()
{
	return static_cast<unsigned int>(vDLCCredits.size());
}

SCreditTextItemDef * CMinecraftApp::GetDLCCredits(int iIndex)
{
	return vDLCCredits.at(iIndex);
}

// Game Host options

void CMinecraftApp::SetGameHostOption(eGameHostOption eVal,unsigned int uiVal)
{
	SetGameHostOption(m_uiGameHostSettings,eVal,uiVal);
}


void CMinecraftApp::SetGameHostOption(unsigned int &uiHostSettings, eGameHostOption eVal, unsigned int uiVal)
{
	switch(eVal)
	{
	case eGameHostOption_FriendsOfFriends:
		if(uiVal!=0)
		{
			uiHostSettings|=GAME_HOST_OPTION_BITMASK_FRIENDSOFFRIENDS;
		}
		else
		{
			// off
			uiHostSettings&=~GAME_HOST_OPTION_BITMASK_FRIENDSOFFRIENDS;
		}
		break;
	case eGameHostOption_Difficulty:
		// clear the difficulty first
		uiHostSettings&=~GAME_HOST_OPTION_BITMASK_DIFFICULTY;
		uiHostSettings|=(GAME_HOST_OPTION_BITMASK_DIFFICULTY&uiVal);
		break;
	case eGameHostOption_Gamertags:
		if(uiVal!=0)
		{
			uiHostSettings|=GAME_HOST_OPTION_BITMASK_GAMERTAGS;
		}
		else
		{
			// off
			uiHostSettings&=~GAME_HOST_OPTION_BITMASK_GAMERTAGS;
		}

		break;
	case eGameHostOption_GameType:
		// clear the game type first
		uiHostSettings&=~GAME_HOST_OPTION_BITMASK_GAMETYPE;
		uiHostSettings|=(GAME_HOST_OPTION_BITMASK_GAMETYPE&(uiVal<<4));
		break;
	case eGameHostOption_LevelType:
		if(uiVal!=0)
		{
			uiHostSettings|=GAME_HOST_OPTION_BITMASK_LEVELTYPE;
		}
		else
		{
			// off
			uiHostSettings&=~GAME_HOST_OPTION_BITMASK_LEVELTYPE;
		}

		break;
	case eGameHostOption_Structures:
		if(uiVal!=0)
		{
			uiHostSettings|=GAME_HOST_OPTION_BITMASK_STRUCTURES;
		}
		else
		{
			// off
			uiHostSettings&=~GAME_HOST_OPTION_BITMASK_STRUCTURES;
		}

		break;
	case eGameHostOption_BonusChest:
		if(uiVal!=0)
		{
			uiHostSettings|=GAME_HOST_OPTION_BITMASK_BONUSCHEST;
		}
		else
		{
			// off
			uiHostSettings&=~GAME_HOST_OPTION_BITMASK_BONUSCHEST;
		}

		break;
	case eGameHostOption_HasBeenInCreative:
		if(uiVal!=0)
		{
			uiHostSettings|=GAME_HOST_OPTION_BITMASK_BEENINCREATIVE;
		}
		else
		{
			// off
			uiHostSettings&=~GAME_HOST_OPTION_BITMASK_BEENINCREATIVE;
		}

		break;
	case eGameHostOption_PvP:
		if(uiVal!=0)
		{
			uiHostSettings|=GAME_HOST_OPTION_BITMASK_PVP;
		}
		else
		{
			// off
			uiHostSettings&=~GAME_HOST_OPTION_BITMASK_PVP;
		}

		break;
	case eGameHostOption_TrustPlayers:
		if(uiVal!=0)
		{
			uiHostSettings|=GAME_HOST_OPTION_BITMASK_TRUSTPLAYERS;
		}
		else
		{
			// off
			uiHostSettings&=~GAME_HOST_OPTION_BITMASK_TRUSTPLAYERS;
		}

		break;
	case eGameHostOption_TNT:
		if(uiVal!=0)
		{
			uiHostSettings|=GAME_HOST_OPTION_BITMASK_TNT;
		}
		else
		{
			// off
			uiHostSettings&=~GAME_HOST_OPTION_BITMASK_TNT;
		}

		break;
	case eGameHostOption_FireSpreads:
		if(uiVal!=0)
		{
			uiHostSettings|=GAME_HOST_OPTION_BITMASK_FIRESPREADS;
		}
		else
		{
			// off
			uiHostSettings&=~GAME_HOST_OPTION_BITMASK_FIRESPREADS;
		}
		break;
	case eGameHostOption_CheatsEnabled:
		if(uiVal!=0)
		{
			uiHostSettings|=GAME_HOST_OPTION_BITMASK_HOSTFLY;
			uiHostSettings|=GAME_HOST_OPTION_BITMASK_HOSTHUNGER;
			uiHostSettings|=GAME_HOST_OPTION_BITMASK_HOSTINVISIBLE;
		}
		else
		{
			// off
			uiHostSettings&=~GAME_HOST_OPTION_BITMASK_HOSTFLY;
			uiHostSettings&=~GAME_HOST_OPTION_BITMASK_HOSTHUNGER;
			uiHostSettings&=~GAME_HOST_OPTION_BITMASK_HOSTINVISIBLE;
		}
		break;
	case eGameHostOption_HostCanFly:
		if(uiVal!=0)
		{
			uiHostSettings|=GAME_HOST_OPTION_BITMASK_HOSTFLY;
		}
		else
		{
			// off
			uiHostSettings&=~GAME_HOST_OPTION_BITMASK_HOSTFLY;
		}
		break;
	case eGameHostOption_HostCanChangeHunger:
		if(uiVal!=0)
		{
			uiHostSettings|=GAME_HOST_OPTION_BITMASK_HOSTHUNGER;
		}
		else
		{
			// off
			uiHostSettings&=~GAME_HOST_OPTION_BITMASK_HOSTHUNGER;
		}
		break;
	case eGameHostOption_HostCanBeInvisible:
		if(uiVal!=0)
		{
			uiHostSettings|=GAME_HOST_OPTION_BITMASK_HOSTINVISIBLE;
		}
		else
		{
			// off
			uiHostSettings&=~GAME_HOST_OPTION_BITMASK_HOSTINVISIBLE;
		}
		break;

	case eGameHostOption_BedrockFog:
		if(uiVal!=0)
		{
			uiHostSettings|=GAME_HOST_OPTION_BITMASK_BEDROCKFOG;
		}
		else
		{
			// off
			uiHostSettings&=~GAME_HOST_OPTION_BITMASK_BEDROCKFOG;
		}
		break;
	case eGameHostOption_DisableSaving:
		if(uiVal!=0)
		{
			uiHostSettings|=GAME_HOST_OPTION_BITMASK_DISABLESAVE;
		}
		else
		{
			// off
			uiHostSettings&=~GAME_HOST_OPTION_BITMASK_DISABLESAVE;
		}
		break;
	case eGameHostOption_WasntSaveOwner:
		if(uiVal!=0)
		{
			uiHostSettings|=GAME_HOST_OPTION_BITMASK_NOTOWNER;
		}
		else
		{
			// off
			uiHostSettings&=~GAME_HOST_OPTION_BITMASK_NOTOWNER;
		}
		break;
	case eGameHostOption_MobGriefing:
		if(uiVal!=1)
		{
			uiHostSettings |= GAME_HOST_OPTION_BITMASK_MOBGRIEFING;
		}
		else
		{
			// off
			uiHostSettings &= ~GAME_HOST_OPTION_BITMASK_MOBGRIEFING;
		}
		break;
	case eGameHostOption_KeepInventory:
		if(uiVal!=0)
		{
			uiHostSettings |= GAME_HOST_OPTION_BITMASK_KEEPINVENTORY;
		}
		else
		{
			// off
			uiHostSettings &= ~GAME_HOST_OPTION_BITMASK_KEEPINVENTORY;
		}
		break;
	case eGameHostOption_DoMobSpawning:
		if(uiVal!=1)
		{
			uiHostSettings |= GAME_HOST_OPTION_BITMASK_DOMOBSPAWNING;
		}
		else
		{
			// off
			uiHostSettings &=~ GAME_HOST_OPTION_BITMASK_DOMOBSPAWNING;
		}
		break;
	case eGameHostOption_DoMobLoot:
		if(uiVal!=1)
		{
			uiHostSettings |= GAME_HOST_OPTION_BITMASK_DOMOBLOOT;
		}
		else
		{
			// off
			uiHostSettings &= ~GAME_HOST_OPTION_BITMASK_DOMOBLOOT;
		}
		break;
	case eGameHostOption_DoTileDrops:
		if(uiVal!=1)
		{
			uiHostSettings |= GAME_HOST_OPTION_BITMASK_DOTILEDROPS;
		}
		else
		{
			// off
			uiHostSettings &= ~GAME_HOST_OPTION_BITMASK_DOTILEDROPS;
		}
		break;
	case eGameHostOption_NaturalRegeneration:
		if(uiVal!=1)
		{
			uiHostSettings |= GAME_HOST_OPTION_BITMASK_NATURALREGEN;
		}
		else
		{
			// off
			uiHostSettings &= ~GAME_HOST_OPTION_BITMASK_NATURALREGEN;
		}
		break;
	case eGameHostOption_DoDaylightCycle:
		if(uiVal!=1)
		{
			uiHostSettings |= GAME_HOST_OPTION_BITMASK_DODAYLIGHTCYCLE;
		}
		else
		{
			// off
			uiHostSettings &= ~GAME_HOST_OPTION_BITMASK_DODAYLIGHTCYCLE;
		}
		break;
	case eGameHostOption_WorldSize:
		// clear the difficulty first
		uiHostSettings&=~GAME_HOST_OPTION_BITMASK_WORLDSIZE;
		uiHostSettings|=(GAME_HOST_OPTION_BITMASK_WORLDSIZE & (uiVal<<GAME_HOST_OPTION_BITMASK_WORLDSIZE_BITSHIFT));
		break;
	case eGameHostOption_All:
		uiHostSettings=uiVal;
		break;
	}
}

unsigned int CMinecraftApp::GetGameHostOption(eGameHostOption eVal)
{
	return GetGameHostOption(m_uiGameHostSettings, eVal);
}

unsigned int CMinecraftApp::GetGameHostOption(unsigned int uiHostSettings, eGameHostOption eVal)
{
	//unsigned int uiVal=0;
	switch(eVal)
	{
	case eGameHostOption_FriendsOfFriends:
		return (uiHostSettings&GAME_HOST_OPTION_BITMASK_FRIENDSOFFRIENDS);
		break;
	case eGameHostOption_Difficulty:
		return (uiHostSettings&GAME_HOST_OPTION_BITMASK_DIFFICULTY);
		break;
	case eGameHostOption_Gamertags:
		return (uiHostSettings&GAME_HOST_OPTION_BITMASK_GAMERTAGS);
		break;
	case eGameHostOption_GameType:
		return (uiHostSettings&GAME_HOST_OPTION_BITMASK_GAMETYPE)>>4;
		break;
	case eGameHostOption_All:
		return (uiHostSettings&GAME_HOST_OPTION_BITMASK_ALL);
		break;
	case eGameHostOption_Tutorial:
		// special case - tutorial is offline, but we want the gamertag option, and set Easy mode, structures on, fire on, tnt on, pvp on, trust players on
		return ((uiHostSettings&GAME_HOST_OPTION_BITMASK_GAMERTAGS)|
			GAME_HOST_OPTION_BITMASK_TRUSTPLAYERS|
			GAME_HOST_OPTION_BITMASK_FIRESPREADS|
			GAME_HOST_OPTION_BITMASK_TNT|
			GAME_HOST_OPTION_BITMASK_PVP|
			GAME_HOST_OPTION_BITMASK_STRUCTURES|1);
		break;
	case eGameHostOption_LevelType:
		return (uiHostSettings&GAME_HOST_OPTION_BITMASK_LEVELTYPE);
		break;
	case eGameHostOption_Structures:
		return (uiHostSettings&GAME_HOST_OPTION_BITMASK_STRUCTURES);
		break;
	case eGameHostOption_BonusChest:
		return (uiHostSettings&GAME_HOST_OPTION_BITMASK_BONUSCHEST);
		break;
	case eGameHostOption_HasBeenInCreative:
		return (uiHostSettings&GAME_HOST_OPTION_BITMASK_BEENINCREATIVE);
		break;
	case eGameHostOption_PvP:
		return (uiHostSettings&GAME_HOST_OPTION_BITMASK_PVP);
		break;
	case eGameHostOption_TrustPlayers:
		return (uiHostSettings&GAME_HOST_OPTION_BITMASK_TRUSTPLAYERS);
		break;
	case eGameHostOption_TNT:
		return (uiHostSettings&GAME_HOST_OPTION_BITMASK_TNT);
		break;
	case eGameHostOption_FireSpreads:
		return (uiHostSettings&GAME_HOST_OPTION_BITMASK_FIRESPREADS);
		break;
	case eGameHostOption_CheatsEnabled:
		return (uiHostSettings&(GAME_HOST_OPTION_BITMASK_HOSTFLY|GAME_HOST_OPTION_BITMASK_HOSTHUNGER|GAME_HOST_OPTION_BITMASK_HOSTINVISIBLE));
		break;
	case eGameHostOption_HostCanFly:
		return (uiHostSettings&GAME_HOST_OPTION_BITMASK_HOSTFLY);
		break;
	case eGameHostOption_HostCanChangeHunger:
		return (uiHostSettings&GAME_HOST_OPTION_BITMASK_HOSTHUNGER);
		break;
	case eGameHostOption_HostCanBeInvisible:
		return (uiHostSettings&GAME_HOST_OPTION_BITMASK_HOSTINVISIBLE);
		break;
	case eGameHostOption_BedrockFog:
		return (uiHostSettings&GAME_HOST_OPTION_BITMASK_BEDROCKFOG);
		break;
	case eGameHostOption_DisableSaving:
		return (uiHostSettings&GAME_HOST_OPTION_BITMASK_DISABLESAVE);
		break;
	case eGameHostOption_WasntSaveOwner:
		return (uiHostSettings&GAME_HOST_OPTION_BITMASK_NOTOWNER);
	case eGameHostOption_WorldSize:
		return (uiHostSettings&GAME_HOST_OPTION_BITMASK_WORLDSIZE) >> GAME_HOST_OPTION_BITMASK_WORLDSIZE_BITSHIFT;
	case eGameHostOption_MobGriefing:
		return !(uiHostSettings&GAME_HOST_OPTION_BITMASK_MOBGRIEFING);
	case eGameHostOption_KeepInventory:
		return (uiHostSettings&GAME_HOST_OPTION_BITMASK_KEEPINVENTORY);
	case eGameHostOption_DoMobSpawning:
		return !(uiHostSettings&GAME_HOST_OPTION_BITMASK_DOMOBSPAWNING);
	case eGameHostOption_DoMobLoot:
		return !(uiHostSettings&GAME_HOST_OPTION_BITMASK_DOMOBLOOT);
	case eGameHostOption_DoTileDrops:
		return !(uiHostSettings&GAME_HOST_OPTION_BITMASK_DOTILEDROPS);
	case eGameHostOption_NaturalRegeneration:
		return !(uiHostSettings&GAME_HOST_OPTION_BITMASK_NATURALREGEN);
	case eGameHostOption_DoDaylightCycle:
		return !(uiHostSettings&GAME_HOST_OPTION_BITMASK_DODAYLIGHTCYCLE);
		break;
	}

	return false;
}

bool CMinecraftApp::CanRecordStatsAndAchievements()
{
	bool isTutorial = Minecraft::GetInstance() != nullptr && Minecraft::GetInstance()->isTutorial();
	// 4J Stu - All of these options give the host player some advantage, so should not allow achievements
	return !(app.GetGameHostOption(eGameHostOption_HasBeenInCreative) ||
		app.GetGameHostOption(eGameHostOption_HostCanBeInvisible) ||
		app.GetGameHostOption(eGameHostOption_HostCanChangeHunger) ||
		app.GetGameHostOption(eGameHostOption_HostCanFly) ||
		app.GetGameHostOption(eGameHostOption_WasntSaveOwner) ||
		!app.GetGameHostOption(eGameHostOption_MobGriefing) ||
		app.GetGameHostOption(eGameHostOption_KeepInventory) ||
		!app.GetGameHostOption(eGameHostOption_DoMobSpawning) ||
		(!app.GetGameHostOption(eGameHostOption_DoDaylightCycle) && !isTutorial )
		);
}

void CMinecraftApp::processSchematics(LevelChunk *levelChunk)
{
	m_gameRules.processSchematics(levelChunk);
}

void CMinecraftApp::processSchematicsLighting(LevelChunk *levelChunk)
{
	m_gameRules.processSchematicsLighting(levelChunk);
}

void CMinecraftApp::loadDefaultGameRules()
{
	m_gameRules.loadDefaultGameRules();
}

void CMinecraftApp::setLevelGenerationOptions(LevelGenerationOptions *levelGen)
{
	m_gameRules.setLevelGenerationOptions(levelGen);
}

LPCWSTR	CMinecraftApp::GetGameRulesString(const wstring &key)
{
	return m_gameRules.GetGameRulesString(key);
}

unsigned char CMinecraftApp::m_szPNG[8]=
{
	137,80,78,71,13,10,26,10
};

#define PNG_TAG_tEXt 0x74455874

unsigned int CMinecraftApp::FromBigEndian(unsigned int uiValue)
{
#if defined(__PS3__) || defined(_XBOX)
	// Keep it in big endian
	return uiValue;
#else
	unsigned int uiReturn = ( ( uiValue >> 24 ) & 0x000000ff ) |
		( ( uiValue >> 8 )  & 0x0000ff00 ) |
		( ( uiValue << 8 )  & 0x00ff0000 ) |
		( ( uiValue << 24 ) & 0xff000000 );
	return uiReturn;
#endif
}

void CMinecraftApp::GetImageTextData(PBYTE pbImageData, DWORD dwImageBytes,unsigned char *pszSeed,unsigned int &uiHostOptions,bool &bHostOptionsRead,DWORD &uiTexturePack)
{
	unsigned char *ucPtr=pbImageData;
	unsigned int uiCount=0;
	unsigned int uiChunkLen;
	unsigned int uiChunkType;
	unsigned int uiCRC;
	char szKeyword[80];

	// check it's a png
	for(int i=0;i<8;i++)
	{
		if(m_szPNG[i]!=ucPtr[i]) return;
	}

	uiCount+=8;

	while(uiCount<dwImageBytes)
	{
		uiChunkLen=*(unsigned int *)&ucPtr[uiCount];
		uiChunkLen=FromBigEndian(uiChunkLen);
		uiCount+=sizeof(int);
		uiChunkType=*(unsigned int *)&ucPtr[uiCount];
		uiChunkType=FromBigEndian(uiChunkType);
		uiCount+=sizeof(int);

		if(uiChunkType==PNG_TAG_tEXt) // tEXt
		{
			// check that it's the 4J text
			unsigned char *pszKeyword=&ucPtr[uiCount];
			while(pszKeyword < ucPtr + uiCount + uiChunkLen)
			{
				ZeroMemory(szKeyword,80);
				unsigned int uiKeywordC=0;
				while(*pszKeyword!=0)
				{
					szKeyword[uiKeywordC++]=*pszKeyword;
					pszKeyword++;
				}
				pszKeyword++;
				if(strcmp(szKeyword,"4J_SEED")==0)
				{
					// read the seed value
					unsigned int uiValueC=0;
					while(*pszKeyword!=0 && (pszKeyword < ucPtr + uiCount + uiChunkLen) )
					{
						pszSeed[uiValueC++]=*pszKeyword;
						pszKeyword++;
					}
					//memcpy(pszSeed,pszKeyword,uiChunkLen-8);
				}
				else if(strcmp(szKeyword,"4J_HOSTOPTIONS")==0)
				{
					bHostOptionsRead = true;
					// read the host options value
					unsigned int uiValueC=0;
					unsigned char pszHostOptions[9]; // Hex representation of unsigned int
					ZeroMemory(&pszHostOptions,9);
					while(*pszKeyword!=0 && (pszKeyword < ucPtr + uiCount + uiChunkLen) && uiValueC < 8)
					{
						pszHostOptions[uiValueC++]=*pszKeyword;
						pszKeyword++;
					}

					uiHostOptions = 0;
					std::stringstream ss;
					ss << pszHostOptions;
					ss >> std::hex >> uiHostOptions;
				}
				else if(strcmp(szKeyword,"4J_TEXTUREPACK")==0)
				{
					// read the texture pack value
					unsigned int uiValueC=0;
					unsigned char pszTexturePack[9]; // Hex representation of unsigned int
					ZeroMemory(&pszTexturePack,9);
					while(*pszKeyword!=0 && (pszKeyword < ucPtr + uiCount + uiChunkLen) && uiValueC < 8)
					{
						pszTexturePack[uiValueC++]=*pszKeyword;
						pszKeyword++;
					}

					std::stringstream ss;
					ss << pszTexturePack;
					ss >> std::hex >> uiTexturePack;
				}
			}
		}
		uiCount+=uiChunkLen;
		uiCRC=*(unsigned int*)&ucPtr[uiCount];
		uiCRC=FromBigEndian(uiCRC);
		uiCount+=sizeof(int);
	}

	return;
}

unsigned int CMinecraftApp::CreateImageTextData(PBYTE bTextMetadata, int64_t seed, bool hasSeed, unsigned int uiHostOptions, unsigned int uiTexturePackId)
{
	int iTextMetadataBytes = 0;
	if(hasSeed)
	{
		strcpy((char *)bTextMetadata,"4J_SEED");
		_i64toa_s(seed,(char *)&bTextMetadata[8],42,10);

		// get the length
		iTextMetadataBytes+=8;
		while(bTextMetadata[iTextMetadataBytes]!=0) iTextMetadataBytes++;
		++iTextMetadataBytes; // Add a null terminator at the end of the seed value
	}

	// Save the host options that this world was last played with
	strcpy((char *)&bTextMetadata[iTextMetadataBytes],"4J_HOSTOPTIONS");
	_itoa_s(uiHostOptions,(char *)&bTextMetadata[iTextMetadataBytes+15],9,16);

	iTextMetadataBytes += 15;
	while(bTextMetadata[iTextMetadataBytes]!=0) iTextMetadataBytes++;
	++iTextMetadataBytes; // Add a null terminator at the end of the host options value

	// Save the texture pack id
	strcpy((char *)&bTextMetadata[iTextMetadataBytes],"4J_TEXTUREPACK");
	_itoa_s(uiTexturePackId,(char *)&bTextMetadata[iTextMetadataBytes+15],9,16);

	iTextMetadataBytes += 15;
	while(bTextMetadata[iTextMetadataBytes]!=0) iTextMetadataBytes++;

	return iTextMetadataBytes;
}

void CMinecraftApp::AddTerrainFeaturePosition(_eTerrainFeatureType eFeatureType,int x,int z)
{
	// check we don't already have this in
	for( FEATURE_DATA *pFeatureData : m_vTerrainFeatures )
	{
		if((pFeatureData->eTerrainFeature==eFeatureType) &&(pFeatureData->x==x) && (pFeatureData->z==z)) return;
	}

	FEATURE_DATA *pFeatureData= new FEATURE_DATA;
	pFeatureData->eTerrainFeature=eFeatureType;
	pFeatureData->x=x;
	pFeatureData->z=z;

	m_vTerrainFeatures.push_back(pFeatureData);
}

_eTerrainFeatureType CMinecraftApp::IsTerrainFeature(int x,int z)
{
	for(FEATURE_DATA *pFeatureData : m_vTerrainFeatures )
	{
		if((pFeatureData->x==x) && (pFeatureData->z==z)) return pFeatureData->eTerrainFeature;
	}

	return eTerrainFeature_None;
}

bool CMinecraftApp::GetTerrainFeaturePosition(_eTerrainFeatureType eType,int *pX, int *pZ)
{
	for ( const FEATURE_DATA *pFeatureData : m_vTerrainFeatures )
	{
		if(pFeatureData->eTerrainFeature==eType)
		{
			*pX=pFeatureData->x;
			*pZ=pFeatureData->z;
			return true;
		}
	}

	return false;
}

void CMinecraftApp::ClearTerrainFeaturePosition()
{
	FEATURE_DATA *pFeatureData;
	while(m_vTerrainFeatures.size()>0)
	{
		pFeatureData = m_vTerrainFeatures.back();
		m_vTerrainFeatures.pop_back();
		delete pFeatureData;
	}
}

void CMinecraftApp::UpdatePlayerInfo(BYTE networkSmallId, SHORT playerColourIndex, unsigned int playerGamePrivileges)
{
	for(unsigned int i = 0; i < MINECRAFT_NET_MAX_PLAYERS; ++i)
	{
		if(m_playerColours[i]==networkSmallId)
		{
			m_playerColours[i] = 0;
			m_playerGamePrivileges[i] = 0;
		}
	}
	if(playerColourIndex >=0 && playerColourIndex < MINECRAFT_NET_MAX_PLAYERS)
	{
		m_playerColours[playerColourIndex] = networkSmallId;
		m_playerGamePrivileges[playerColourIndex] = playerGamePrivileges;
	}
}

short CMinecraftApp::GetPlayerColour(BYTE networkSmallId)
{
	short index = -1;
	for(unsigned int i = 0; i < MINECRAFT_NET_MAX_PLAYERS; ++i)
	{
		if(m_playerColours[i]==networkSmallId)
		{
			index = i;
			break;
		}
	}
	return index;
}


unsigned int CMinecraftApp::GetPlayerPrivileges(BYTE networkSmallId)
{
	unsigned int privileges = 0;
	for(unsigned int i = 0; i < MINECRAFT_NET_MAX_PLAYERS; ++i)
	{
		if(m_playerColours[i]==networkSmallId)
		{
			privileges = m_playerGamePrivileges[i];
			break;
		}
	}
	return privileges;
}

wstring CMinecraftApp::getEntityName(eINSTANCEOF type)
{
	switch(type)
	{
	case eTYPE_WOLF:
		return app.GetString(IDS_WOLF);
	case eTYPE_CREEPER:
		return app.GetString(IDS_CREEPER);
	case eTYPE_SKELETON:
		return app.GetString(IDS_SKELETON);
	case eTYPE_SPIDER:
		return app.GetString(IDS_SPIDER);
	case eTYPE_ZOMBIE:
		return app.GetString(IDS_ZOMBIE);
	case eTYPE_PIGZOMBIE:
		return app.GetString(IDS_PIGZOMBIE);
	case eTYPE_ENDERMAN:
		return app.GetString(IDS_ENDERMAN);
	case eTYPE_SILVERFISH:
		return app.GetString(IDS_SILVERFISH);
	case eTYPE_CAVESPIDER:
		return app.GetString(IDS_CAVE_SPIDER);
	case eTYPE_GHAST:
		return app.GetString(IDS_GHAST);
	case eTYPE_SLIME:
		return app.GetString(IDS_SLIME);
	case eTYPE_ARROW:
		return app.GetString(IDS_ITEM_ARROW);
	case eTYPE_ENDERDRAGON:
		return app.GetString(IDS_ENDERDRAGON);
	case eTYPE_BLAZE:
		return app.GetString(IDS_BLAZE);
	case eTYPE_LAVASLIME:
		return app.GetString(IDS_LAVA_SLIME);
		// 4J-PB - fix for #107167 - Customer Encountered: TU12: Content: UI: There is no information what killed Player after being slain by Iron Golem.
	case eTYPE_VILLAGERGOLEM:
		return app.GetString(IDS_IRONGOLEM);
	case eTYPE_HORSE:
		return app.GetString(IDS_HORSE);
	case eTYPE_WITCH:
		return app.GetString(IDS_WITCH);
	case eTYPE_WITHERBOSS:
		return app.GetString(IDS_WITHER);
	case eTYPE_BAT:
		return app.GetString(IDS_BAT);
	};

	return L"";
}

DWORD CMinecraftApp::m_dwContentTypeA[e_Marketplace_MAX]=
{
	XMARKETPLACE_OFFERING_TYPE_CONTENT,		// e_DLC_SkinPack, e_DLC_TexturePacks, e_DLC_MashupPacks
#ifndef _XBOX_ONE
	XMARKETPLACE_OFFERING_TYPE_THEME,		// e_DLC_Themes
	XMARKETPLACE_OFFERING_TYPE_AVATARITEM,	// e_DLC_AvatarItems
	XMARKETPLACE_OFFERING_TYPE_TILE,		// e_DLC_Gamerpics
#endif
};

unsigned int CMinecraftApp::AddDLCRequest(eDLCMarketplaceType eType, bool bPromote)
{
	// lock access
	EnterCriticalSection(&csDLCDownloadQueue);

	// If it's already in there, promote it to the top of the list
	int iPosition=0;
	for( DLCRequest  *pCurrent : m_DLCDownloadQueue )
	{
		if(pCurrent->dwType==m_dwContentTypeA[eType])
		{
			// already got this in the list
			if(pCurrent->eState == e_DLC_ContentState_Retrieving || pCurrent->eState == e_DLC_ContentState_Retrieved)
			{
				// already retrieved this
				LeaveCriticalSection(&csDLCDownloadQueue);
				return 0;
			}
			else
			{
				// promote
				if(bPromote)
				{
					m_DLCDownloadQueue.erase(m_DLCDownloadQueue.begin()+iPosition);
					m_DLCDownloadQueue.insert(m_DLCDownloadQueue.begin(),pCurrent);
				}
				LeaveCriticalSection(&csDLCDownloadQueue);
				return 0;
			}
		}
		iPosition++;
	}

	DLCRequest *pDLCreq = new DLCRequest;
	pDLCreq->dwType=m_dwContentTypeA[eType];
	pDLCreq->eState=e_DLC_ContentState_Idle;

	m_DLCDownloadQueue.push_back(pDLCreq);

	m_bAllDLCContentRetrieved=false;
	LeaveCriticalSection(&csDLCDownloadQueue);

	app.DebugPrintf("[Consoles_App] Added DLC request.\n");
	return 1;
}

unsigned int CMinecraftApp::AddTMSPPFileTypeRequest(eDLCContentType eType, bool bPromote)
{
#if !defined(__PS3__) && !defined(__ORBIS__) && !defined(__PSVITA__)
	// lock access
	EnterCriticalSection(&csTMSPPDownloadQueue);

	// If it's already in there, promote it to the top of the list
	int iPosition=0;
	//ignore promoting for now
	/*
	bool bPromoted=false;


	for ( TMSPPRequest *pCurrent : m_TMSPPDownloadQueue )
	{
	TMSPPRequest  *pCurrent = *it;

	if(pCurrent->eType==eType)
	{
	if(!(pCurrent->eState == e_TMS_ContentState_Retrieving || pCurrent->eState == e_TMS_ContentState_Retrieved))
	{
	// promote
	if(bPromote)
	{
	m_TMSPPDownloadQueue.erase(m_TMSPPDownloadQueue.begin()+iPosition);
	m_TMSPPDownloadQueue.insert(m_TMSPPDownloadQueue.begin(),pCurrent);
	bPromoted=true;
	}
	}
	}
	iPosition++;
	}

	if(bPromoted)
	{
	// re-ordered the list, so leave now
	LeaveCriticalSection(&csTMSPPDownloadQueue);
	return 0;
	}
	*/

	// special case for data files (not image files)
	if(eType==e_DLC_TexturePackData)
	{


		int iCount=GetDLCInfoFullOffersCount();

		for(int i=0;i<iCount;i++)
		{

			DLC_INFO *pDLC=GetDLCInfoFullOffer(i);

			if((pDLC->eDLCType==e_DLC_TexturePacks) || (pDLC->eDLCType==e_DLC_MashupPacks))
			{
				// first check if the image is already in the memory textures, since we might be loading some from the Title Update partition
				if(pDLC->wchDataFile[0]!=0)
				{
					//WCHAR *cString = pDLC->wchDataFile;
					// 4J-PB - shouldn't check this here - let the TMS files override it, so if they are on TMS, we'll take them first
					//int iIndex = app.GetLocalTMSFileIndex(pDLC->wchDataFile,true);

					//if(iIndex!=-1)
					{
						bool bPresent = app.IsFileInTPD(pDLC->iConfig);

						if(!bPresent)
						{
							// this may already be present in the vector because of a previous trial/full offer

							bool bAlreadyInQueue=false;
							for( TMSPPRequest  *pCurrent : m_TMSPPDownloadQueue )
							{
								if(wcscmp(pDLC->wchDataFile,pCurrent->wchFilename)==0)
								{
									bAlreadyInQueue=true;
									break;
								}
							}

							if(!bAlreadyInQueue)
							{
								TMSPPRequest *pTMSPPreq = new TMSPPRequest;

								pTMSPPreq->CallbackFunc=&CMinecraftApp::TMSPPFileReturned;
								pTMSPPreq->lpCallbackParam=this;
								pTMSPPreq->eStorageFacility=C4JStorage::eGlobalStorage_Title;
								pTMSPPreq->eFileTypeVal=C4JStorage::TMS_FILETYPE_BINARY;
								memcpy(pTMSPPreq->wchFilename,pDLC->wchDataFile,sizeof(WCHAR)*MAX_BANNERNAME_SIZE);
								pTMSPPreq->eType=e_DLC_TexturePackData;
								pTMSPPreq->eState=e_TMS_ContentState_Queued;
								m_bAllTMSContentRetrieved=false;
								m_TMSPPDownloadQueue.push_back(pTMSPPreq);
							}
						}
						else
						{
							app.DebugPrintf("Texture data already present in the TPD\n");
						}
					}
				}
			}
		}
	}
	else
	{		// for all the files of type eType, add them to the download list

		// run through the trial offers first, then the full offers. Any duplicates won't be added to the download queue
		int iCount;
#ifdef _XBOX // Only trial offers on Xbox 360
		iCount=GetDLCInfoTrialOffersCount();
		for(int i=0;i<iCount;i++)
		{
			DLC_INFO *pDLC=GetDLCInfoTrialOffer(i);

			// is this the right type?
			if(pDLC->eDLCType==eType)
			{

				WCHAR *cString = pDLC->wchBanner;

				// 4J-PB - shouldn't check this here - let the TMS files override it, so if they are on TMS, we'll take them first
				// is the file in the TMS XZP?
				//int iIndex = app.GetLocalTMSFileIndex(cString,true);

				//if(iIndex!=-1)
				{
					bool bPresent = app.IsFileInMemoryTextures(cString);

					if(!bPresent) // retrieve it from TMSPP
					{
						bool bAlreadyInQueue=false;
						for( TMSPPRequest  *pCurrent : m_TMSPPDownloadQueue )
						{
							if(wcscmp(pDLC->wchBanner,pCurrent->wchFilename)==0)
							{
								bAlreadyInQueue=true;
								break;
							}
						}

						if(!bAlreadyInQueue)
						{
							TMSPPRequest *pTMSPPreq = new TMSPPRequest;

							pTMSPPreq->CallbackFunc=&CMinecraftApp::TMSPPFileReturned;
							pTMSPPreq->lpCallbackParam=this;
							pTMSPPreq->eStorageFacility=C4JStorage::eGlobalStorage_Title;
							pTMSPPreq->eFileTypeVal=C4JStorage::TMS_FILETYPE_BINARY;
							//wcstombs(pTMSPPreq->szFilename,pDLC->wchBanner,MAX_TMSFILENAME_SIZE);
							memcpy(pTMSPPreq->wchFilename,pDLC->wchBanner,sizeof(WCHAR)*MAX_BANNERNAME_SIZE);
							pTMSPPreq->eType=eType;
							pTMSPPreq->eState=e_TMS_ContentState_Queued;

							m_bAllTMSContentRetrieved=false;
							m_TMSPPDownloadQueue.push_back(pTMSPPreq);
							app.DebugPrintf("===m_TMSPPDownloadQueue Adding %ls, q size is %d\n",pTMSPPreq->wchFilename,m_TMSPPDownloadQueue.size());
						}
					}
				}
			}
		}
#endif
		// and the full offers

		iCount=GetDLCInfoFullOffersCount();
		for(int i=0;i<iCount;i++)
		{
			DLC_INFO *pDLC=GetDLCInfoFullOffer(i);
			//if(wcscmp(pDLC->wchType,wchDLCTypeNames[eType])==0)
			if(pDLC->eDLCType==eType)
			{
				// first check if the image is already in the memory textures, since we might be loading some from the Title Update partition

				WCHAR *cString = pDLC->wchBanner;
				// 4J-PB - shouldn't check this here - let the TMS files override it, so if they are on TMS, we'll take them first
				//int iIndex = app.GetLocalTMSFileIndex(cString,true);

				//if(iIndex!=-1)
				{
					bool bPresent = app.IsFileInMemoryTextures(cString);

					if(!bPresent)
					{
						// this may already be present in the vector because of a previous trial/full offer

						bool bAlreadyInQueue=false;
						for( TMSPPRequest  *pCurrent : m_TMSPPDownloadQueue )
						{
							if(wcscmp(pDLC->wchBanner,pCurrent->wchFilename)==0)
							{
								bAlreadyInQueue=true;
								break;
							}
						}

						if(!bAlreadyInQueue)
						{
							//app.DebugPrintf("Adding a request to the TMSPP download queue - %ls\n",pDLC->wchBanner);
							TMSPPRequest *pTMSPPreq = new TMSPPRequest;
							ZeroMemory(pTMSPPreq,sizeof(TMSPPRequest));

							pTMSPPreq->CallbackFunc=&CMinecraftApp::TMSPPFileReturned;
							pTMSPPreq->lpCallbackParam=this;
							// 4J-PB - testing for now
							//pTMSPPreq->eStorageFacility=C4JStorage::eGlobalStorage_TitleUser;
							pTMSPPreq->eStorageFacility=C4JStorage::eGlobalStorage_Title;
							pTMSPPreq->eFileTypeVal=C4JStorage::TMS_FILETYPE_BINARY;
							//wcstombs(pTMSPPreq->szFilename,pDLC->wchBanner,MAX_TMSFILENAME_SIZE);

							memcpy(pTMSPPreq->wchFilename,pDLC->wchBanner,sizeof(WCHAR)*MAX_BANNERNAME_SIZE);
							pTMSPPreq->eType=eType;
							pTMSPPreq->eState=e_TMS_ContentState_Queued;
							m_bAllTMSContentRetrieved=false;
							m_TMSPPDownloadQueue.push_back(pTMSPPreq);
							app.DebugPrintf("===m_TMSPPDownloadQueue Adding %ls, q size is %d\n",pTMSPPreq->wchFilename,m_TMSPPDownloadQueue.size());
						}
					}
				}
			}
		}
	}

	LeaveCriticalSection(&csTMSPPDownloadQueue);
#endif
	return 1;
}

bool CMinecraftApp::CheckTMSDLCCanStop()
{
	EnterCriticalSection(&csTMSPPDownloadQueue);
	for( TMSPPRequest  *pCurrent : m_TMSPPDownloadQueue )
	{
		if(pCurrent->eState==e_TMS_ContentState_Retrieving)
		{
			LeaveCriticalSection(&csTMSPPDownloadQueue);
			return false;
		}
	}
	LeaveCriticalSection(&csTMSPPDownloadQueue);

	return true;
}


bool CMinecraftApp::RetrieveNextDLCContent()
{
	// If there's already a retrieve in progress, quit
	// we may have re-ordered the list, so need to check every item

	// is there a primary player and a network connection?
	int primPad = ProfileManager.GetPrimaryPad();
	if ( primPad == -1 || !ProfileManager.IsSignedInLive(primPad) )
	{
		return true; // 4J-JEV: We need to wait until the primary player is online.
	}

	EnterCriticalSection(&csDLCDownloadQueue);
	for( const DLCRequest* pCurrent : m_DLCDownloadQueue )
	{
		if(pCurrent->eState==e_DLC_ContentState_Retrieving)
		{
			LeaveCriticalSection(&csDLCDownloadQueue);
			return true;
		}
	}

	// Now look for the next retrieval
	for( DLCRequest  *pCurrent : m_DLCDownloadQueue )
	{
		if(pCurrent->eState==e_DLC_ContentState_Idle)
		{
#ifdef _DEBUG
			app.DebugPrintf("RetrieveNextDLCContent - type = %d\n",pCurrent->dwType);
#endif

			C4JStorage::EDLCStatus status = StorageManager.GetDLCOffers(ProfileManager.GetPrimaryPad(), &CMinecraftApp::DLCOffersReturned, this, pCurrent->dwType);
			if(status==C4JStorage::EDLC_Pending)
			{
				pCurrent->eState=e_DLC_ContentState_Retrieving;
			}
			else
			{
				// no content of this type, or some other problem
				app.DebugPrintf("RetrieveNextDLCContent - PROBLEM\n");
				pCurrent->eState=e_DLC_ContentState_Retrieved;
			}
			LeaveCriticalSection(&csDLCDownloadQueue);
			return true;
		}
	}
	LeaveCriticalSection(&csDLCDownloadQueue);

	app.DebugPrintf("[Consoles_App] Finished downloading dlc content.\n");
	return false;
}

#if !defined(__PS3__) && !defined(__ORBIS__) && !defined(__PSVITA__)
#ifdef _XBOX_ONE
int CMinecraftApp::TMSPPFileReturned(LPVOID pParam,int iPad,int iUserData,LPVOID lpvData, WCHAR* wchFilename)
{
	C4JStorage::PTMSPP_FILEDATA pFileData=(C4JStorage::PTMSPP_FILEDATA)lpvData;
#else
int CMinecraftApp::TMSPPFileReturned(LPVOID pParam,int iPad,int iUserData,C4JStorage::PTMSPP_FILEDATA pFileData, LPCSTR szFilename)
{
#endif

	CMinecraftApp* pClass = static_cast<CMinecraftApp *>(pParam);

	// find the right one in the vector
	EnterCriticalSection(&pClass->csTMSPPDownloadQueue);
	for( TMSPPRequest  *pCurrent : pClass->m_TMSPPDownloadQueue )
	{
#if defined(_XBOX) || defined(_WINDOWS64)
		char szFile[MAX_TMSFILENAME_SIZE];
		wcstombs(szFile,pCurrent->wchFilename,MAX_TMSFILENAME_SIZE);


		if(strcmp(szFilename,szFile)==0)
#elif _XBOX_ONE
		if(wcscmp(wchFilename,pCurrent->wchFilename)==0)
#endif
		{
			// set this to retrieved whether it found it or not
			pCurrent->eState=e_TMS_ContentState_Retrieved;

			if(pFileData!=nullptr)
			{	
#ifdef _XBOX_ONE


				switch(pCurrent->eType)
				{
				case e_DLC_TexturePackData:
					{
						// 4J-PB - we need to allocate memory for the file data and copy into it, since the current data is a reference into the blob download memory
						PBYTE pbData = new BYTE [pFileData->dwSize];
						memcpy(pbData,pFileData->pbData,pFileData->dwSize);

						pClass->m_vTMSPPData.push_back(pbData);
						app.DebugPrintf("Got texturepack data\n");
						// get the config value for the texture pack
						int iConfig=app.GetTPConfigVal(pCurrent->wchFilename);
						app.AddMemoryTPDFile(iConfig, pbData, pFileData->dwSize);
					}
					break;
				default:
					// 4J-PB - check the data is an image
					if(pFileData->pbData[0]==0x89)
					{
						// 4J-PB - we need to allocate memory for the file data and copy into it, since the current data is a reference into the blob download memory
						PBYTE pbData = new BYTE [pFileData->dwSize];
						memcpy(pbData,pFileData->pbData,pFileData->dwSize);

						pClass->m_vTMSPPData.push_back(pbData);
						app.DebugPrintf("Got image data - %ls\n",pCurrent->wchFilename);
						app.AddMemoryTextureFile(pCurrent->wchFilename, pbData, pFileData->dwSize);
					}
					else
					{
						app.DebugPrintf("Got image data, but it's not a png - %ls\n",pCurrent->wchFilename);
					}
					break;
				}

#else
				switch(pCurrent->eType)
				{
				case e_DLC_TexturePackData:
					{
						app.DebugPrintf("--- Got texturepack data %ls\n",pCurrent->wchFilename);
						// get the config value for the texture pack
						int iConfig=app.GetTPConfigVal(pCurrent->wchFilename);
						app.AddMemoryTPDFile(iConfig, pFileData->pbData, pFileData->dwSize);
					}
					break;
				default:
					app.DebugPrintf("--- Got image data - %ls\n",pCurrent->wchFilename);
					app.AddMemoryTextureFile(pCurrent->wchFilename, pFileData->pbData, pFileData->dwSize);
					break;
				}
#endif
			}
			else
			{
#ifdef _XBOX_ONE
				app.DebugPrintf("TMSImageReturned failed (%ls)...\n",wchFilename);
#else
				app.DebugPrintf("TMSImageReturned failed (%s)...\n",szFilename);
#endif
			}
			break;
		}

	}
	LeaveCriticalSection(&pClass->csTMSPPDownloadQueue);

	return 0;
}
#endif

bool CMinecraftApp::RetrieveNextTMSPPContent()
{
#if defined _XBOX || defined _XBOX_ONE
	// If there's already a retrieve in progress, quit
	// we may have re-ordered the list, so need to check every item

	// is there a primary player and a network connection?
	if(ProfileManager.GetPrimaryPad()==-1) return false;

	if(ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad())==false) return false;

	for( TMSPPRequest  *pCurrent : m_TMSPPDownloadQueue )
	{
		TMSPPRequest  *pCurrent = *it;

		if(pCurrent->eState==e_TMS_ContentState_Retrieving)
		{
			app.DebugPrintf(".");
			LeaveCriticalSection(&csTMSPPDownloadQueue);
			return true;
		}
	}

	// Now look for the next retrieval
	for( TMSPPRequest  *pCurrent : m_TMSPPDownloadQueue )
	{
		TMSPPRequest  *pCurrent = *it;

		if(pCurrent->eState==e_TMS_ContentState_Queued)
		{
			// 4J-PB - the file may be in the local TMS files, but try to retrieve it from the remote TMS in case it's been changed. If it's not in the list of TMS files, this will
			// return right away with a ETMSStatus_Fail_ReadDetailsNotRetrieved
#ifdef _XBOX
			char szFilename[MAX_TMSFILENAME_SIZE];
			wcstombs(szFilename,pCurrent->wchFilename,MAX_TMSFILENAME_SIZE);

			app.DebugPrintf("\nRetrieveNextTMSPPContent - type = %d, %s\n",pCurrent->eType,szFilename);

			C4JStorage::ETMSStatus status=StorageManager.TMSPP_ReadFile(ProfileManager.GetPrimaryPad(),pCurrent->eStorageFacility,pCurrent->eFileTypeVal,szFilename,pCurrent->CallbackFunc,this);
			switch(status)
			{
			case C4JStorage::ETMSStatus_Pending:
				pCurrent->eState=e_TMS_ContentState_Retrieving;
				break;
			case C4JStorage::ETMSStatus_Idle:
				pCurrent->eState=e_TMS_ContentState_Retrieved;
				break;
			case C4JStorage::ETMSStatus_Fail_ReadInProgress:
			case C4JStorage::ETMSStatus_ReadInProgress:
				pCurrent->eState=e_TMS_ContentState_Retrieving;
				if(pCurrent->eState==C4JStorage::ETMSStatus_Fail_ReadInProgress)
				{
					app.DebugPrintf("TMSPP_ReadFile failed - read in progress\n");
					Sleep(50);
					LeaveCriticalSection(&csTMSPPDownloadQueue);
					return false;
				}
				break;
			default:
				pCurrent->eState=e_TMS_ContentState_Retrieved;
				break;
			}
#else
			eTitleStorageState status;
			app.DebugPrintf("RetrieveNextTMSPPContent - type = %d, %ls\n",pCurrent->eType,pCurrent->wchFilename);
			//eTitleStorageState status=StorageManager.TMSPP_ReadFile(ProfileManager.GetPrimaryPad(),pCurrent->eStorageFacility,pCurrent->eFileTypeVal,pCurrent->wchFilename,pCurrent->CallbackFunc,this,0);
			if(0)//wcscmp(pCurrent->wchFilename,L"TP01.png")==0)
			{
				// TP01 fails because the blob size returned is bigger than the global metadata says it should be
				status=eTitleStorage_readerror;
			}
			else
			{
				status=StorageManager.TMSPP_ReadFile(ProfileManager.GetPrimaryPad(),pCurrent->eStorageFacility,pCurrent->eFileTypeVal,pCurrent->wchFilename,pCurrent->CallbackFunc,this,0);
			}
			switch(status)
			{
			case eTitleStorage_pending:
				pCurrent->eState=e_TMS_ContentState_Retrieving;
				break;
			case eTitleStorage_idle:
				pCurrent->eState=e_TMS_ContentState_Retrieved;
				break;
			case eTitleStorage_busy:
				// try again next time
				{
					app.DebugPrintf("@@@@@@@@@@@@@@@@@ TMSPP_ReadFile failed - busy (probably reading already)\n");
					Sleep(50);
					LeaveCriticalSection(&csTMSPPDownloadQueue);
					return false;
				}
				break;
			default:
				pCurrent->eState=e_TMS_ContentState_Retrieved;
				break;
			}
#endif



			LeaveCriticalSection(&csTMSPPDownloadQueue);
			return true;
		}
	}

	LeaveCriticalSection(&csTMSPPDownloadQueue);

#endif
	return false;
}

void CMinecraftApp::TickDLCOffersRetrieved()
{
	if(!m_bAllDLCContentRetrieved)
	{
		if (!app.RetrieveNextDLCContent())
		{
			app.DebugPrintf("[Consoles_App] All content retrieved.\n");
			m_bAllDLCContentRetrieved=true;
		}
	}
}
void  CMinecraftApp::ClearAndResetDLCDownloadQueue()
{
	app.DebugPrintf("[Consoles_App] Clear and reset download queue.\n");

	int iPosition=0;
	EnterCriticalSection(&csTMSPPDownloadQueue);
	for( DLCRequest  *pCurrent : m_DLCDownloadQueue )
	{
		if ( pCurrent )
			delete pCurrent;
		iPosition++;
	}
	m_DLCDownloadQueue.clear();
	m_bAllDLCContentRetrieved=true;
	LeaveCriticalSection(&csTMSPPDownloadQueue);
}

void CMinecraftApp::TickTMSPPFilesRetrieved()
{
	if(m_bTickTMSDLCFiles && !m_bAllTMSContentRetrieved)
	{
		if(app.RetrieveNextTMSPPContent()==false)
		{
			m_bAllTMSContentRetrieved=true;
		}
	}
}
void  CMinecraftApp::ClearTMSPPFilesRetrieved()
{
	int iPosition=0;
	EnterCriticalSection(&csTMSPPDownloadQueue);
	for ( TMSPPRequest  *pCurrent : m_TMSPPDownloadQueue )
	{
		if ( pCurrent )
			delete pCurrent;
		iPosition++;
	}
	m_TMSPPDownloadQueue.clear();
	m_bAllTMSContentRetrieved=true;
	LeaveCriticalSection(&csTMSPPDownloadQueue);
}

int CMinecraftApp::DLCOffersReturned(void *pParam, int iOfferC, DWORD dwType, int iPad)
{
	CMinecraftApp* pClass = static_cast<CMinecraftApp *>(pParam);

	// find the right one in the vector
	EnterCriticalSection(&pClass->csTMSPPDownloadQueue);
	for( DLCRequest  *pCurrent : pClass->m_DLCDownloadQueue )
	{
		// avatar items are coming back as type Content, so we can't trust the type setting
		if(pCurrent->dwType==dwType)
		{
			pClass->m_iDLCOfferC = iOfferC;
			app.DebugPrintf("DLCOffersReturned - type %d, count %d - setting to retrieved\n",dwType,iOfferC);
			pCurrent->eState=e_DLC_ContentState_Retrieved;
			break;
		}
	}
	LeaveCriticalSection(&pClass->csTMSPPDownloadQueue);
	return 0;
}

eDLCContentType CMinecraftApp::Find_eDLCContentType(DWORD dwType)
{
	for(int i=0;i<e_DLC_MAX;i++)
	{
		if(m_dwContentTypeA[i]==dwType)
		{
			return static_cast<eDLCContentType>(i);
		}
	}
	return static_cast<eDLCContentType>(0);
}
bool CMinecraftApp::DLCContentRetrieved(eDLCMarketplaceType eType)
{
	// If there's already a retrieve in progress, quit
	// we may have re-ordered the list, so need to check every item
	EnterCriticalSection(&csDLCDownloadQueue);
	for( DLCRequest  *pCurrent : m_DLCDownloadQueue )
	{
		if((pCurrent->dwType==m_dwContentTypeA[eType]) && (pCurrent->eState==e_DLC_ContentState_Retrieved))
		{
			LeaveCriticalSection(&csDLCDownloadQueue);
			return true;
		}
	}
	LeaveCriticalSection(&csDLCDownloadQueue);
	return false;
}

void CMinecraftApp::SetAdditionalSkinBoxes(DWORD dwSkinID, SKIN_BOX *SkinBoxA, DWORD dwSkinBoxC)
{
	EntityRenderer *renderer = EntityRenderDispatcher::instance->getRenderer(eTYPE_PLAYER);
	Model *pModel = renderer->getModel();
	vector<ModelPart *> *pvModelPart = new vector<ModelPart *>;
	vector<SKIN_BOX *> *pvSkinBoxes = new vector<SKIN_BOX *>;

	EnterCriticalSection( &csAdditionalModelParts );
	EnterCriticalSection( &csAdditionalSkinBoxes );

	app.DebugPrintf("*** SetAdditionalSkinBoxes - Inserting model parts for skin %d from array of Skin Boxes\n",dwSkinID&0x0FFFFFFF);

	// convert the skin boxes into model parts, and add to the humanoid model
	for(unsigned int i=0;i<dwSkinBoxC;i++)
	{
		if(pModel)
		{
			ModelPart *pModelPart=pModel->AddOrRetrievePart(&SkinBoxA[i]);
			pvModelPart->push_back(pModelPart);
			pvSkinBoxes->push_back(&SkinBoxA[i]);
		}
	}


	m_AdditionalModelParts.insert( std::pair<DWORD, vector<ModelPart *> *>(dwSkinID, pvModelPart) );
	m_AdditionalSkinBoxes.insert( std::pair<DWORD, vector<SKIN_BOX *> *>(dwSkinID, pvSkinBoxes) );

	LeaveCriticalSection( &csAdditionalSkinBoxes );
	LeaveCriticalSection( &csAdditionalModelParts );

}

vector<ModelPart *> * CMinecraftApp::SetAdditionalSkinBoxes(DWORD dwSkinID, vector<SKIN_BOX *> *pvSkinBoxA)
{
	EntityRenderer *renderer = EntityRenderDispatcher::instance->getRenderer(eTYPE_PLAYER);
	Model *pModel = renderer->getModel();
	vector<ModelPart *> *pvModelPart = new vector<ModelPart *>;

	EnterCriticalSection( &csAdditionalModelParts );
	EnterCriticalSection( &csAdditionalSkinBoxes );
	app.DebugPrintf("*** SetAdditionalSkinBoxes - Inserting model parts for skin %d from array of Skin Boxes\n",dwSkinID&0x0FFFFFFF);

	// convert the skin boxes into model parts, and add to the humanoid model
	for( auto& it : *pvSkinBoxA )
	{
		if(pModel)
		{
			ModelPart *pModelPart=pModel->AddOrRetrievePart(it);
			pvModelPart->push_back(pModelPart);
		}
	}

	m_AdditionalModelParts.emplace(dwSkinID, pvModelPart);
	m_AdditionalSkinBoxes.emplace(dwSkinID, pvSkinBoxA);

	LeaveCriticalSection( &csAdditionalSkinBoxes );
	LeaveCriticalSection( &csAdditionalModelParts );
	return pvModelPart;
}


vector<ModelPart *> *CMinecraftApp::GetAdditionalModelParts(DWORD dwSkinID)
{
	EnterCriticalSection( &csAdditionalModelParts );
	vector<ModelPart *> *pvModelParts=nullptr;
	if(m_AdditionalModelParts.size()>0)
	{
		auto it = m_AdditionalModelParts.find(dwSkinID);
		if(it!=m_AdditionalModelParts.end())
		{
			pvModelParts = (*it).second;
		}
	}

	LeaveCriticalSection( &csAdditionalModelParts );
	return pvModelParts;
}

vector<SKIN_BOX *> *CMinecraftApp::GetAdditionalSkinBoxes(DWORD dwSkinID)
{
	EnterCriticalSection( &csAdditionalSkinBoxes );
	vector<SKIN_BOX *> *pvSkinBoxes=nullptr;
	if(m_AdditionalSkinBoxes.size()>0)
	{
		auto it = m_AdditionalSkinBoxes.find(dwSkinID);
		if(it!=m_AdditionalSkinBoxes.end())
		{
			pvSkinBoxes = (*it).second;
		}
	}

	LeaveCriticalSection( &csAdditionalSkinBoxes );
	return pvSkinBoxes;
}

unsigned int CMinecraftApp::GetAnimOverrideBitmask(DWORD dwSkinID)
{
	EnterCriticalSection( &csAnimOverrideBitmask );
	unsigned int uiAnimOverrideBitmask=0L;

	if(m_AnimOverrides.size()>0)
	{
		auto it = m_AnimOverrides.find(dwSkinID);
		if(it!=m_AnimOverrides.end())
		{
			uiAnimOverrideBitmask = (*it).second;
		}
	}

	LeaveCriticalSection( &csAnimOverrideBitmask );
	return uiAnimOverrideBitmask;
}

void CMinecraftApp::SetAnimOverrideBitmask(DWORD dwSkinID,unsigned int uiAnimOverrideBitmask)
{
	// Make thread safe
	EnterCriticalSection( &csAnimOverrideBitmask );

	if(m_AnimOverrides.size()>0)
	{
		auto it = m_AnimOverrides.find(dwSkinID);
		if(it!=m_AnimOverrides.end())
		{
			LeaveCriticalSection( &csAnimOverrideBitmask );
			return; // already in here
		}
	}
	m_AnimOverrides.insert( std::pair<DWORD, unsigned long>(dwSkinID, uiAnimOverrideBitmask) );
	LeaveCriticalSection( &csAnimOverrideBitmask );
}

DWORD CMinecraftApp::getSkinIdFromPath(const wstring &skin)
{
	bool dlcSkin = false;
	unsigned int skinId = 0;

	if(skin.size() >= 14)
	{
		dlcSkin = skin.substr(0,3).compare(L"dlc") == 0;

		wstring skinValue = skin.substr(7,skin.size());
		skinValue = skinValue.substr(0,skinValue.find_first_of(L'.'));

		std::wstringstream ss;
		// 4J Stu - dlc skins are numbered using decimal to make it easier for artists/people to number manually
		// Everything else is numbered using hex
		if(dlcSkin)
			ss << std::dec << skinValue.c_str();
		else
			ss << std::hex << skinValue.c_str();
		ss >> skinId;

		skinId = MAKE_SKIN_BITMASK(dlcSkin, skinId);
	}
	return skinId;
}

wstring CMinecraftApp::getSkinPathFromId(DWORD skinId)
{
	// 4J Stu - This function maps the encoded DWORD we store in the player profile
	// to a filename that is stored as a memory texture and shared between systems in game
	wchar_t chars[256];
	if( GET_IS_DLC_SKIN_FROM_BITMASK(skinId) )
	{
		// 4J Stu - DLC skins are numbered using decimal rather than hex to make it easier to number manually
		swprintf(chars, 256, L"dlcskin%08d.png", GET_DLC_SKIN_ID_FROM_BITMASK(skinId));

	}
	else
	{
		DWORD ugcSkinIndex = GET_UGC_SKIN_ID_FROM_BITMASK(skinId);
		DWORD defaultSkinIndex = GET_DEFAULT_SKIN_ID_FROM_BITMASK(skinId);
		if( ugcSkinIndex == 0 )
		{
			swprintf(chars, 256, L"defskin%08X.png",defaultSkinIndex);
		}
		else
		{
			swprintf(chars, 256, L"ugcskin%08X.png",ugcSkinIndex);
		}
	}
	return chars;
}


int CMinecraftApp::TexturePackDialogReturned(void* pParam, int iPad, C4JStorage::EMessageResult result)
{

#if defined __PSVITA__ || defined __PS3__ || defined __ORBIS__
    if (result == C4JStorage::EMessage_ResultAccept)
    {
        Minecraft* pMinecraft = Minecraft::GetInstance();
        if (pMinecraft->skins->selectTexturePackById(app.GetRequiredTexturePackID()))
        {
            // it's been installed already
        }
        else
        {
            // we need to enable background downloading for the DLC
            XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE_ALWAYS_ALLOW);

            SONYDLC* pSONYDLCInfo = app.GetSONYDLCInfo(app.GetRequiredTexturePackID());
            if (pSONYDLCInfo != nullptr)
            {
                char chName[42];
                char chKeyName[20];
                char chSkuID[SCE_NP_COMMERCE2_SKU_ID_LEN];

                memset(chSkuID, 0, SCE_NP_COMMERCE2_SKU_ID_LEN);

                memset(chKeyName, 0, sizeof(chKeyName));
                strncpy(chKeyName, pSONYDLCInfo->chDLCKeyname, 16);

    #ifdef __ORBIS__
                strcpy(chName, chKeyName);
    #else
                sprintf(chName, "%s-%s", app.GetCommerceCategory(), chKeyName);
    #endif

                app.GetDLCSkuIDFromProductList(chName, chSkuID);

                if (app.CheckForEmptyStore(iPad) == false)
                {
                    if (app.DLCAlreadyPurchased(chSkuID))
                    {
                        app.DownloadAlreadyPurchased(chSkuID);
                    }
                    else
                    {
                        app.Checkout(chSkuID);
                    }
                }
            }
        }
    }
    else
    {
        app.DebugPrintf("Continuing without installing texture pack\n");
    }
#endif

#ifdef _XBOX
    if (result != C4JStorage::EMessage_Cancelled)
    {
        if (app.GetRequiredTexturePackID() != 0)
        {
            XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE_ALWAYS_ALLOW);

            ULONGLONG ullOfferID_Full;
            ULONGLONG ullIndexA[1];
            app.GetDLCFullOfferIDForPackID(app.GetRequiredTexturePackID(), &ullOfferID_Full);

            if (result == C4JStorage::EMessage_ResultAccept)
            {
                ullIndexA[0] = ullOfferID_Full;
                StorageManager.InstallOffer(1, ullIndexA, nullptr, nullptr);
            }
            else
            {
                DLC_INFO* pDLCInfo = app.GetDLCInfoForFullOfferID(ullOfferID_Full);
                ullIndexA[0] = pDLCInfo->ullOfferID_Trial;
                StorageManager.InstallOffer(1, ullIndexA, nullptr, nullptr);
            }
        }
    }
#endif
    return 0;
}
int CMinecraftApp::getArchiveFileSize(const wstring &filename)
{
	TexturePack *tPack = nullptr;
	Minecraft *pMinecraft = Minecraft::GetInstance();
	if(pMinecraft && pMinecraft->skins) tPack = pMinecraft->skins->getSelected();
	if(tPack && tPack->hasData() && tPack->getArchiveFile() && tPack->getArchiveFile()->hasFile(filename))
	{
		return tPack->getArchiveFile()->getFileSize(filename);
	}
	else return m_mediaArchive->getFileSize(filename);
}

bool CMinecraftApp::hasArchiveFile(const wstring &filename)
{
	TexturePack *tPack = nullptr;
	Minecraft *pMinecraft = Minecraft::GetInstance();
	if(pMinecraft && pMinecraft->skins) tPack = pMinecraft->skins->getSelected();
	if(tPack && tPack->hasData() && tPack->getArchiveFile() && tPack->getArchiveFile()->hasFile(filename)) return true;
	else return m_mediaArchive->hasFile(filename);
}

byteArray CMinecraftApp::getArchiveFile(const wstring &filename)
{
	TexturePack *tPack = nullptr;
	Minecraft *pMinecraft = Minecraft::GetInstance();
	if(pMinecraft && pMinecraft->skins) tPack = pMinecraft->skins->getSelected();
	if(tPack && tPack->hasData() && tPack->getArchiveFile() && tPack->getArchiveFile()->hasFile(filename))
	{
		return tPack->getArchiveFile()->getFile(filename);
	}
	else return m_mediaArchive->getFile(filename);
}

// DLC

#if defined(__PS3__) || defined(__ORBIS__) || defined (__PSVITA__)
int CMinecraftApp::GetDLCInfoCount()
{
	return (int)DLCInfo.size();
}
#elif defined _XBOX_ONE
int CMinecraftApp::GetDLCInfoTrialOffersCount()
{
	return 0;
}

int CMinecraftApp::GetDLCInfoFullOffersCount()
{
	return (int)DLCInfo_Full.size();
}
#else
int CMinecraftApp::GetDLCInfoTrialOffersCount()
{ 
    return static_cast<int>(DLCInfo_Trial.size());
}

int CMinecraftApp::GetDLCInfoFullOffersCount() 
{ 
    return static_cast<int>(DLCInfo_Full.size());
}
#endif

int CMinecraftApp::GetDLCInfoTexturesOffersCount()
{ 
    return static_cast<int>(DLCTextures_PackID.size());
}

// AUTOSAVE
void CMinecraftApp::SetAutosaveTimerTime(void)
{
#if defined(_XBOX_ONE) || defined(__ORBIS__)
	m_uiAutosaveTimer= GetTickCount()+1000*60;
#else
	m_uiAutosaveTimer= GetTickCount()+GetGameSettings(ProfileManager.GetPrimaryPad(),eGameSetting_Autosave)*1000*60*15;
#endif
}// value x 15 to get mins, x60 for secs

bool CMinecraftApp::AutosaveDue(void)
{
	return (GetTickCount()>m_uiAutosaveTimer);
}

unsigned int CMinecraftApp::SecondsToAutosave()
{
	return (m_uiAutosaveTimer - GetTickCount() ) / 1000;
}

void CMinecraftApp::SetTrialTimerStart(void)
{
	m_fTrialTimerStart=m_Time.fAppTime; mfTrialPausedTime=0.0f;
}

float CMinecraftApp::getTrialTimer(void)
{
	return m_Time.fAppTime-m_fTrialTimerStart-mfTrialPausedTime;
}

bool CMinecraftApp::IsLocalMultiplayerAvailable()
{
	DWORD connectedControllers = 0;
	for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
	{
		if( InputManager.IsPadConnected(i) || ProfileManager.IsSignedIn(i) ) ++connectedControllers;
	}

#ifdef _WINDOWS64
	bool available = connectedControllers > 1;
#else
	bool available = RenderManager.IsHiDef() && connectedControllers > 1;
#endif

#ifdef __ORBIS__
	// Check for remote play
	available = available && InputManager.IsLocalMultiplayerAvailable();
#endif

	return available;

	// Found this in GameNetworkManager?
	//#ifdef _DURANGO
	//		iOtherConnectedControllers = InputManager.GetConnectedGamepadCount();
	//		if((InputManager.IsPadConnected(userIndex) || ProfileManager.IsSignedIn(userIndex)))
	//		{
	//			--iOtherConnectedControllers;
	//		}
	//#else
	//		for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
	//		{
	//			if( (i!=userIndex) && (InputManager.IsPadConnected(i) || ProfileManager.IsSignedIn(i)) )
	//			{
	//				iOtherConnectedControllers++;
	//			}
	//		}
	//#endif
}


// 4J-PB - language and locale function

void CMinecraftApp::getLocale(vector<wstring> &vecWstrLocales)
{
	vector<eMCLang> locales;

	DWORD dwSystemLanguage = XGetLanguage( );

	// 4J-PB - restrict the 360 language until we're ready to have them in

#ifdef _XBOX
	switch(dwSystemLanguage)
	{
	case XC_LANGUAGE_FRENCH             :
		locales.push_back(eMCLang_frFR);
		break;
	case XC_LANGUAGE_ITALIAN            :
		locales.push_back(eMCLang_itIT);
		break;
	case XC_LANGUAGE_GERMAN             :
		locales.push_back(eMCLang_deDE);
		break;
	case XC_LANGUAGE_SPANISH            :
		locales.push_back(eMCLang_esES);
		break;
	case XC_LANGUAGE_PORTUGUESE         :
		if(XGetLocale()==XC_LOCALE_BRAZIL)
		{
			locales.push_back(eMCLang_ptBR);
		}
		locales.push_back(eMCLang_ptPT);
		break;
	case XC_LANGUAGE_JAPANESE           :
		locales.push_back(eMCLang_jaJP);
		break;
	case XC_LANGUAGE_KOREAN             :
		locales.push_back(eMCLang_koKR);
		break;
	case XC_LANGUAGE_TCHINESE           :
		locales.push_back(eMCLang_zhCHT);
		break;
	}
#else
	switch(dwSystemLanguage)
	{

	case XC_LANGUAGE_ENGLISH:
		switch(XGetLocale())
		{
		case XC_LOCALE_AUSTRALIA:
		case XC_LOCALE_CANADA:
		case XC_LOCALE_CZECH_REPUBLIC:
		case XC_LOCALE_GREECE:
		case XC_LOCALE_HONG_KONG:
		case XC_LOCALE_HUNGARY:
		case XC_LOCALE_INDIA:
		case XC_LOCALE_IRELAND:
		case XC_LOCALE_ISRAEL:
		case XC_LOCALE_NEW_ZEALAND:
		case XC_LOCALE_SAUDI_ARABIA:
		case XC_LOCALE_SINGAPORE:
		case XC_LOCALE_SLOVAK_REPUBLIC:
		case XC_LOCALE_SOUTH_AFRICA:
		case XC_LOCALE_UNITED_ARAB_EMIRATES:
		case XC_LOCALE_GREAT_BRITAIN:
			locales.push_back(eMCLang_enGB);
			break;
		default: //XC_LOCALE_UNITED_STATES
			break;
		}
		break;
	case XC_LANGUAGE_JAPANESE           :
		locales.push_back(eMCLang_jaJP);
		break;
	case XC_LANGUAGE_GERMAN             :
		switch(XGetLocale())
		{
		case XC_LOCALE_AUSTRIA:
			locales.push_back(eMCLang_deAT);
			break;
		case XC_LOCALE_SWITZERLAND:
			locales.push_back(eMCLang_deCH);
			break;
		default:// XC_LOCALE_GERMANY:
			break;
		}
		locales.push_back(eMCLang_deDE);
		break;
	case XC_LANGUAGE_FRENCH             :
		switch(XGetLocale())
		{
		case XC_LOCALE_BELGIUM:
			locales.push_back(eMCLang_frBE);
			break;
		case XC_LOCALE_CANADA:
			locales.push_back(eMCLang_frCA);
			break;
		case XC_LOCALE_SWITZERLAND:
			locales.push_back(eMCLang_frCH);
			break;
		default:// XC_LOCALE_FRANCE:
			break;
		}
		locales.push_back(eMCLang_frFR);
		break;
	case XC_LANGUAGE_SPANISH            :
		switch(XGetLocale())
		{
		case XC_LOCALE_MEXICO:
		case XC_LOCALE_ARGENTINA:
		case XC_LOCALE_CHILE:
		case XC_LOCALE_COLOMBIA:
		case XC_LOCALE_UNITED_STATES:
		case XC_LOCALE_LATIN_AMERICA:
			locales.push_back(eMCLang_laLAS);
			locales.push_back(eMCLang_esMX);
			break;
		default://XC_LOCALE_SPAIN
			break;
		}
		locales.push_back(eMCLang_esES);
		break;
	case XC_LANGUAGE_ITALIAN            :
		locales.push_back(eMCLang_itIT);
		break;
	case XC_LANGUAGE_KOREAN             :
		locales.push_back(eMCLang_koKR);
		break;
	case XC_LANGUAGE_TCHINESE           :
		switch(XGetLocale())
		{
		case XC_LOCALE_HONG_KONG:
			locales.push_back(eMCLang_zhHK);
			locales.push_back(eMCLang_zhTW);
			break;
		case XC_LOCALE_TAIWAN:
			locales.push_back(eMCLang_zhTW);
			locales.push_back(eMCLang_zhHK);
		default:
			break;
		}
		locales.push_back(eMCLang_hant);
		locales.push_back(eMCLang_zhCHT);
		break;
	case XC_LANGUAGE_PORTUGUESE         :
		if(XGetLocale()==XC_LOCALE_BRAZIL)
		{
			locales.push_back(eMCLang_ptBR);
		}
		locales.push_back(eMCLang_ptPT);
		break;
	case XC_LANGUAGE_POLISH             :
		locales.push_back(eMCLang_plPL);
		break;
	case XC_LANGUAGE_RUSSIAN            :
		locales.push_back(eMCLang_ruRU);
		break;
	case XC_LANGUAGE_SWEDISH            :
		locales.push_back(eMCLang_svSV);
		locales.push_back(eMCLang_svSE);
		break;
	case XC_LANGUAGE_TURKISH            :
		locales.push_back(eMCLang_trTR);
		break;
	case XC_LANGUAGE_BNORWEGIAN         :
		locales.push_back(eMCLang_nbNO);
		locales.push_back(eMCLang_noNO);
		locales.push_back(eMCLang_nnNO);
		break;
	case XC_LANGUAGE_DUTCH              :
		switch(XGetLocale())
		{
		case XC_LOCALE_BELGIUM:
			locales.push_back(eMCLang_nlBE);
			break;
		default:
			break;
		}
		locales.push_back(eMCLang_nlNL);
		break;
	case XC_LANGUAGE_SCHINESE           :
		switch(XGetLocale())
		{
		case XC_LOCALE_SINGAPORE:
			locales.push_back(eMCLang_zhSG);
			break;
		default:
			break;
		}
		locales.push_back(eMCLang_hans);
		locales.push_back(eMCLang_csCS);
		locales.push_back(eMCLang_zhCN);
		break;

#if defined __PS3__ || defined __ORBIS__ || defined __PSVITA__ || defined _DURANGO
	case XC_LANGUAGE_DANISH:
		locales.push_back(eMCLang_daDA);
		locales.push_back(eMCLang_daDK);
		break;

	case XC_LANGUAGE_FINISH				:
		locales.push_back(eMCLang_fiFI);
		break;

	case XC_LANGUAGE_CZECH				:
		locales.push_back(eMCLang_csCZ);
		locales.push_back(eMCLang_enCZ);
		break;

	case XC_LANGUAGE_SLOVAK				:
		locales.push_back(eMCLang_skSK);
		locales.push_back(eMCLang_enSK);
		break;

	case XC_LANGUAGE_GREEK				:
		locales.push_back(eMCLang_elEL);
		locales.push_back(eMCLang_elGR);
		locales.push_back(eMCLang_enGR);
		locales.push_back(eMCLang_enGB);
		break;
#endif
	}
#endif

	locales.push_back(eMCLang_enUS);
	locales.push_back(eMCLang_null);

	for (size_t i=0; i<locales.size(); i++)
	{
		eMCLang lang = locales.at(i);
		vecWstrLocales.push_back( m_localeA[lang] );
	}
}

DWORD CMinecraftApp::get_eMCLang(WCHAR *pwchLocale)
{
	return m_eMCLangA[pwchLocale];
}


DWORD CMinecraftApp::get_xcLang(WCHAR *pwchLocale)
{
	return m_xcLangA[pwchLocale];
}

void CMinecraftApp::LocaleAndLanguageInit()
{
	m_localeA[eMCLang_zhCHT]	=L"zh-CHT";
	m_localeA[eMCLang_csCS]		=L"cs-CS";
	m_localeA[eMCLang_laLAS]	=L"la-LAS";
	m_localeA[eMCLang_null]		=L"en-EN";
	m_localeA[eMCLang_enUS]		=L"en-US";
	m_localeA[eMCLang_enGB]		=L"en-GB";
	m_localeA[eMCLang_enIE]		=L"en-IE";
	m_localeA[eMCLang_enAU]		=L"en-AU";
	m_localeA[eMCLang_enNZ]		=L"en-NZ";
	m_localeA[eMCLang_enCA]		=L"en-CA";
	m_localeA[eMCLang_jaJP]		=L"ja-JP";
	m_localeA[eMCLang_deDE]		=L"de-DE";
	m_localeA[eMCLang_deAT]		=L"de-AT";
	m_localeA[eMCLang_frFR]		=L"fr-FR";
	m_localeA[eMCLang_frCA]		=L"fr-CA";
	m_localeA[eMCLang_esES]		=L"es-ES";
	m_localeA[eMCLang_esMX]		=L"es-MX";
	m_localeA[eMCLang_itIT]		=L"it-IT";
	m_localeA[eMCLang_koKR]		=L"ko-KR";
	m_localeA[eMCLang_ptPT]		=L"pt-PT";
	m_localeA[eMCLang_ptBR]		=L"pt-BR";
	m_localeA[eMCLang_ruRU]		=L"ru-RU";
	m_localeA[eMCLang_nlNL]		=L"nl-NL";
	m_localeA[eMCLang_fiFI]		=L"fi-FI";
	m_localeA[eMCLang_svSV]		=L"sv-SV";
	m_localeA[eMCLang_daDA]		=L"da-DA";
	m_localeA[eMCLang_noNO]		=L"no-NO";
	m_localeA[eMCLang_plPL]		=L"pl-PL";
	m_localeA[eMCLang_trTR]		=L"tr-TR";
	m_localeA[eMCLang_elEL]		=L"el-EL";

	m_localeA[eMCLang_zhSG]		=L"zh-SG";
	m_localeA[eMCLang_zhCN]		=L"zh-CN";
	m_localeA[eMCLang_zhHK]		=L"zh-HK";
	m_localeA[eMCLang_zhTW]		=L"zh-TW";
	m_localeA[eMCLang_nlBE]		=L"nl-BE";
	m_localeA[eMCLang_daDK]		=L"da-DK";
	m_localeA[eMCLang_frBE]		=L"fr-BE";
	m_localeA[eMCLang_frCH]		=L"fr-CH";
	m_localeA[eMCLang_deCH]		=L"de-CH";
	m_localeA[eMCLang_nbNO]		=L"nb-NO";
	m_localeA[eMCLang_enGR]		=L"en-GR";
	m_localeA[eMCLang_enHK]		=L"en-HK";
	m_localeA[eMCLang_enSA]		=L"en-SA";
	m_localeA[eMCLang_enHU]		=L"en-HU";
	m_localeA[eMCLang_enIN]		=L"en-IN";
	m_localeA[eMCLang_enIL]		=L"en-IL";
	m_localeA[eMCLang_enSG]		=L"en-SG";
	m_localeA[eMCLang_enSK]		=L"en-SK";
	m_localeA[eMCLang_enZA]		=L"en-ZA";
	m_localeA[eMCLang_enCZ]		=L"en-CZ";
	m_localeA[eMCLang_enAE]		=L"en-AE";
	m_localeA[eMCLang_esAR]		=L"es-AR";
	m_localeA[eMCLang_esCL]		=L"es-CL";
	m_localeA[eMCLang_esCO]		=L"es-CO";
	m_localeA[eMCLang_esUS]		=L"es-US";
	m_localeA[eMCLang_svSE]		=L"sv-SE";

	m_localeA[eMCLang_csCZ]		=L"cs-CZ";
	m_localeA[eMCLang_elGR]		=L"el-GR";
	m_localeA[eMCLang_nnNO]		=L"nn-NO";
	m_localeA[eMCLang_skSK]		=L"sk-SK";

	m_localeA[eMCLang_hans]		=L"zh-HANS";
	m_localeA[eMCLang_hant]		=L"zh-HANT";

	m_eMCLangA[L"zh-CHT"]		=eMCLang_zhCHT;
	m_eMCLangA[L"cs-CS"]		=eMCLang_csCS;
	m_eMCLangA[L"la-LAS"]		=eMCLang_laLAS;
	m_eMCLangA[L"en-EN"]		=eMCLang_null;
	m_eMCLangA[L"en-US"]		=eMCLang_enUS;
	m_eMCLangA[L"en-GB"]		=eMCLang_enGB;
	m_eMCLangA[L"en-IE"]		=eMCLang_enIE;
	m_eMCLangA[L"en-AU"]		=eMCLang_enAU;
	m_eMCLangA[L"en-NZ"]		=eMCLang_enNZ;
	m_eMCLangA[L"en-CA"]		=eMCLang_enCA;
	m_eMCLangA[L"ja-JP"]		=eMCLang_jaJP;
	m_eMCLangA[L"de-DE"]		=eMCLang_deDE;
	m_eMCLangA[L"de-AT"]		=eMCLang_deAT;
	m_eMCLangA[L"fr-FR"]		=eMCLang_frFR;
	m_eMCLangA[L"fr-CA"]		=eMCLang_frCA;
	m_eMCLangA[L"es-ES"]		=eMCLang_esES;
	m_eMCLangA[L"es-MX"]		=eMCLang_esMX;
	m_eMCLangA[L"it-IT"]		=eMCLang_itIT;
	m_eMCLangA[L"ko-KR"]		=eMCLang_koKR;
	m_eMCLangA[L"pt-PT"]		=eMCLang_ptPT;
	m_eMCLangA[L"pt-BR"]		=eMCLang_ptBR;
	m_eMCLangA[L"ru-RU"]		=eMCLang_ruRU;
	m_eMCLangA[L"nl-NL"]		=eMCLang_nlNL;
	m_eMCLangA[L"fi-FI"]		=eMCLang_fiFI;
	m_eMCLangA[L"sv-SV"]		=eMCLang_svSV;
	m_eMCLangA[L"da-DA"]		=eMCLang_daDA;
	m_eMCLangA[L"no-NO"]		=eMCLang_noNO;
	m_eMCLangA[L"pl-PL"]		=eMCLang_plPL;
	m_eMCLangA[L"tr-TR"]		=eMCLang_trTR;
	m_eMCLangA[L"el-EL"]		=eMCLang_elEL;

	m_eMCLangA[L"zh-SG"]		=eMCLang_zhSG;
	m_eMCLangA[L"zh-CN"]		=eMCLang_zhCN;
	m_eMCLangA[L"zh-HK"]		=eMCLang_zhHK;
	m_eMCLangA[L"zh-TW"]		=eMCLang_zhTW;
	m_eMCLangA[L"nl-BE"]		=eMCLang_nlBE;
	m_eMCLangA[L"da-DK"]		=eMCLang_daDK;
	m_eMCLangA[L"fr-BE"]		=eMCLang_frBE;
	m_eMCLangA[L"fr-CH"]		=eMCLang_frCH;
	m_eMCLangA[L"de-CH"]		=eMCLang_deCH;
	m_eMCLangA[L"nb-NO"]		=eMCLang_nbNO;
	m_eMCLangA[L"en-GR"]		=eMCLang_enGR;
	m_eMCLangA[L"en-HK"]		=eMCLang_enHK;
	m_eMCLangA[L"en-SA"]		=eMCLang_enSA;
	m_eMCLangA[L"en-HU"]		=eMCLang_enHU;
	m_eMCLangA[L"en-IN"]		=eMCLang_enIN;
	m_eMCLangA[L"en-IL"]		=eMCLang_enIL;
	m_eMCLangA[L"en-SG"]		=eMCLang_enSG;
	m_eMCLangA[L"en-SK"]		=eMCLang_enSK;
	m_eMCLangA[L"en-ZA"]		=eMCLang_enZA;
	m_eMCLangA[L"en-CZ"]		=eMCLang_enCZ;
	m_eMCLangA[L"en-AE"]		=eMCLang_enAE;
	m_eMCLangA[L"es-AR"]		=eMCLang_esAR;
	m_eMCLangA[L"es-CL"]		=eMCLang_esCL;
	m_eMCLangA[L"es-CO"]		=eMCLang_esCO;
	m_eMCLangA[L"es-US"]		=eMCLang_esUS;
	m_eMCLangA[L"sv-SE"]		=eMCLang_svSE;

	m_eMCLangA[L"cs-CZ"]		=eMCLang_csCZ;
	m_eMCLangA[L"el-GR"]		=eMCLang_elGR;
	m_eMCLangA[L"nn-NO"]		=eMCLang_nnNO;
	m_eMCLangA[L"sk-SK"]		=eMCLang_skSK;

	m_eMCLangA[L"zh-HANS"]		=eMCLang_hans;
	m_eMCLangA[L"zh-HANT"]		=eMCLang_hant;

	m_xcLangA[L"zh-CHT"]		=XC_LOCALE_CHINA;
	m_xcLangA[L"cs-CS"]			=XC_LOCALE_CHINA;
	m_xcLangA[L"en-EN"]			=XC_LOCALE_UNITED_STATES;
	m_xcLangA[L"en-US"]			=XC_LOCALE_UNITED_STATES;
	m_xcLangA[L"en-GB"]			=XC_LOCALE_GREAT_BRITAIN;
	m_xcLangA[L"en-IE"]			=XC_LOCALE_IRELAND;
	m_xcLangA[L"en-AU"]			=XC_LOCALE_AUSTRALIA;
	m_xcLangA[L"en-NZ"]			=XC_LOCALE_NEW_ZEALAND;
	m_xcLangA[L"en-CA"]			=XC_LOCALE_CANADA;
	m_xcLangA[L"ja-JP"]			=XC_LOCALE_JAPAN;
	m_xcLangA[L"de-DE"]			=XC_LOCALE_GERMANY;
	m_xcLangA[L"de-AT"]			=XC_LOCALE_AUSTRIA;
	m_xcLangA[L"fr-FR"]			=XC_LOCALE_FRANCE;
	m_xcLangA[L"fr-CA"]			=XC_LOCALE_CANADA;
	m_xcLangA[L"es-ES"]			=XC_LOCALE_SPAIN;
	m_xcLangA[L"es-MX"]			=XC_LOCALE_MEXICO;
	m_xcLangA[L"it-IT"]			=XC_LOCALE_ITALY;
	m_xcLangA[L"ko-KR"]			=XC_LOCALE_KOREA;
	m_xcLangA[L"pt-PT"]			=XC_LOCALE_PORTUGAL;
	m_xcLangA[L"pt-BR"]			=XC_LOCALE_BRAZIL;
	m_xcLangA[L"ru-RU"]			=XC_LOCALE_RUSSIAN_FEDERATION;
	m_xcLangA[L"nl-NL"]			=XC_LOCALE_NETHERLANDS;
	m_xcLangA[L"fi-FI"]			=XC_LOCALE_FINLAND;
	m_xcLangA[L"sv-SV"]			=XC_LOCALE_SWEDEN;
	m_xcLangA[L"da-DA"]			=XC_LOCALE_DENMARK;
	m_xcLangA[L"no-NO"]			=XC_LOCALE_NORWAY;
	m_xcLangA[L"pl-PL"]			=XC_LOCALE_POLAND;
	m_xcLangA[L"tr-TR"]			=XC_LOCALE_TURKEY;
	m_xcLangA[L"el-EL"]			=XC_LOCALE_GREECE;
#ifndef _XBOX
	m_xcLangA[L"la-LAS"]		=XC_LOCALE_LATIN_AMERICA;
#endif

	// New ones for Xbox One
	m_xcLangA[L"zh-SG"]			=XC_LOCALE_SINGAPORE;
	m_xcLangA[L"Zh-CN"]			=XC_LOCALE_CHINA;
	m_xcLangA[L"zh-HK"]			=XC_LOCALE_HONG_KONG;
	m_xcLangA[L"zh-TW"]			=XC_LOCALE_TAIWAN;
	m_xcLangA[L"nl-BE"]			=XC_LOCALE_BELGIUM;
	m_xcLangA[L"da-DK"]			=XC_LOCALE_DENMARK;
	m_xcLangA[L"fr-BE"]			=XC_LOCALE_BELGIUM;
	m_xcLangA[L"fr-CH"]			=XC_LOCALE_SWITZERLAND;
	m_xcLangA[L"de-CH"]			=XC_LOCALE_SWITZERLAND;
	m_xcLangA[L"nb-NO"]			=XC_LOCALE_NORWAY;
	m_xcLangA[L"en-GR"]			=XC_LOCALE_GREECE;
	m_xcLangA[L"en-HK"]			=XC_LOCALE_HONG_KONG;
	m_xcLangA[L"en-SA"]			=XC_LOCALE_SAUDI_ARABIA;
	m_xcLangA[L"en-HU"]			=XC_LOCALE_HUNGARY;
	m_xcLangA[L"en-IN"]			=XC_LOCALE_INDIA;
	m_xcLangA[L"en-IL"]			=XC_LOCALE_ISRAEL;
	m_xcLangA[L"en-SG"]			=XC_LOCALE_SINGAPORE;
	m_xcLangA[L"en-SK"]			=XC_LOCALE_SLOVAK_REPUBLIC;
	m_xcLangA[L"en-ZA"]			=XC_LOCALE_SOUTH_AFRICA;
	m_xcLangA[L"en-CZ"]			=XC_LOCALE_CZECH_REPUBLIC;
	m_xcLangA[L"en-AE"]			=XC_LOCALE_UNITED_ARAB_EMIRATES;
	m_xcLangA[L"ja-IP"]			=XC_LOCALE_JAPAN;
	m_xcLangA[L"es-AR"]			=XC_LOCALE_ARGENTINA;
	m_xcLangA[L"es-CL"]			=XC_LOCALE_CHILE;
	m_xcLangA[L"es-CO"]			=XC_LOCALE_COLOMBIA;
	m_xcLangA[L"es-US"]			=XC_LOCALE_UNITED_STATES;
	m_xcLangA[L"sv-SE"]			=XC_LOCALE_SWEDEN;

	m_xcLangA[L"cs-CZ"]			=XC_LOCALE_CZECH_REPUBLIC;
	m_xcLangA[L"el-GR"]			=XC_LOCALE_GREECE;
	m_xcLangA[L"sk-SK"]			=XC_LOCALE_SLOVAK_REPUBLIC;

	m_xcLangA[L"zh-HANS"]		=XC_LOCALE_CHINA;
	m_xcLangA[L"zh-HANT"]		=XC_LOCALE_CHINA;
}

void CMinecraftApp::SetTickTMSDLCFiles(bool bVal)
{
	// 4J-PB - we need to stop the retrieval of minecraft store images from TMS when we aren't in the DLC, since going in to Play Game will change the title id group
	m_bTickTMSDLCFiles=bVal;
}

wstring CMinecraftApp::getFilePath(DWORD packId, wstring filename, bool bAddDataFolder, wstring mountPoint)
{
	wstring path = getRootPath(packId, true, bAddDataFolder, mountPoint) + filename;
	File f(path);
	if(f.exists())
	{
		return path;
	}
	return getRootPath(packId, false, true, mountPoint) + filename;
}

#ifdef _XBOX
// Texture packs that have their data in the TU data
enum ETitleUpdateTexturePacks
{
	eTUTP_MassEffect = 0x400,
	eTUTP_Skyrim = 0x401,
	eTUTP_Halo = 0x402,
	eTUTP_Festive = 0x405,

	eTUTP_Plastic = 0x801,
	eTUTP_Candy = 0x802,
	eTUTP_Fantasy = 0x803,
	eTUTP_Halloween = 0x804,
	eTUTP_Natural = 0x805,
	eTUTP_City = 0x01000806, // 4J Stu - The released City pack had a sub-pack ID
	eTUTP_Cartoon = 0x807,
	eTUTP_Steampunk = 0x01000808, // 4J Stu - The released Steampunk pack had a sub-pack ID
};

#ifdef _TU_BUILD
wstring titleUpdateTexturePackRoot = L"UPDATE:\\res\\DLC\\";
#else
wstring titleUpdateTexturePackRoot = L"GAME:\\res\\TitleUpdate\\DLC\\";
#endif
#else
enum ETitleUpdateTexturePacks
{
	//eTUTP_MassEffect = 0x400,
	//eTUTP_Skyrim = 0x401,
	//eTUTP_Halo = 0x402,
	//eTUTP_Festive = 0x405,

	//eTUTP_Plastic = 0x801,
	//eTUTP_Candy = 0x802,
	//eTUTP_Fantasy = 0x803,
	eTUTP_Halloween = 0x804,
	//eTUTP_Natural = 0x805,
	//eTUTP_City = 0x01000806, // 4J Stu - The released City pack had a sub-pack ID
	//eTUTP_Cartoon = 0x807,
	//eTUTP_Steampunk = 0x01000808, // 4J Stu - The released Steampunk pack had a sub-pack ID
};

#ifdef _WINDOWS64
wstring titleUpdateTexturePackRoot = L"Windows64\\DLC\\";
#elif defined(__ORBIS__)
wstring titleUpdateTexturePackRoot = L"/app0/orbis/CU/DLC/";
#elif defined(__PSVITA__)
wstring titleUpdateTexturePackRoot = L"PSVita/CU/DLC/";
#elif defined(__PS3__)
wstring titleUpdateTexturePackRoot = L"PS3/CU/DLC/";
#else
wstring titleUpdateTexturePackRoot = L"CU\\DLC\\";
#endif

#endif

wstring CMinecraftApp::getRootPath(DWORD packId, bool allowOverride, bool bAddDataFolder, wstring mountPoint)
{
	wstring path = mountPoint;
#ifdef _XBOX
	if(allowOverride)
	{
		switch(packId)
		{
		case eTUTP_MassEffect:
			path = titleUpdateTexturePackRoot + L"MassEffect";
			break;
		case eTUTP_Skyrim:
			path = titleUpdateTexturePackRoot + L"Skyrim";
			break;
		case eTUTP_Halo:
			path = titleUpdateTexturePackRoot + L"Halo";
			break;
		case eTUTP_Festive:
			path = titleUpdateTexturePackRoot + L"Festive";
			break;
		case eTUTP_Plastic:
			path = titleUpdateTexturePackRoot + L"Plastic";
			break;
		case eTUTP_Candy:
			path = titleUpdateTexturePackRoot + L"Candy";
			break;
		case eTUTP_Fantasy:
			path = titleUpdateTexturePackRoot + L"Fantasy";
			break;
		case eTUTP_Halloween:
			path = titleUpdateTexturePackRoot + L"Halloween";
			break;
		case eTUTP_Natural:
			path = titleUpdateTexturePackRoot + L"Natural";
			break;
		case eTUTP_City:
			path = titleUpdateTexturePackRoot + L"City";
			break;
		case eTUTP_Cartoon:
			path = titleUpdateTexturePackRoot + L"Cartoon";
			break;
		case eTUTP_Steampunk:
			path = titleUpdateTexturePackRoot + L"Steampunk";
			break;
		};
		File folder(path);
		if(!folder.exists())
		{
			path = mountPoint;
		}
	}
#else
	if(allowOverride)
	{
		switch(packId)
		{
		case eTUTP_Halloween:
			path = titleUpdateTexturePackRoot + L"Halloween Texture Pack";
			break;
		};
		File folder(path);
		if(!folder.exists())
		{
			path = mountPoint;
		}
	}
#endif

	if(bAddDataFolder)
	{
		return path + L"\\Data\\";
	}
	else
	{
		return path + L"\\";
	}

}

#ifdef _XBOX_ONE
void CMinecraftApp::SetReachedMainMenu()
{
	m_hasReachedMainMenu = true;
}

bool CMinecraftApp::HasReachedMainMenu()
{
	return m_hasReachedMainMenu;
}
#endif
