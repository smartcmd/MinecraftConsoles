#include "stdafx.h"

#include "..\Common\UI\UIStructs.h"

#include "..\Common\XUI\XUI_Intro.h"
#include "..\Common\XUI\XUI_MainMenu.h"
#include "..\Common\XUI\XUI_NewUpdateMessage.h"
#include "..\Common\XUI\XUI_HelpAndOptions.h"
#include "..\Common\XUI\XUI_TextEntry.h"
#include "..\Common\XUI\XUI_HelpHowToPlay.h"
#include "..\Common\XUI\XUI_HowToPlayMenu.h"
#include "..\Common\XUI\XUI_HelpControls.h"
#include "..\Common\XUI\XUI_TextEntry.h"
#include "..\Common\XUI\XUI_LoadSettings.h"
#include "..\Common\XUI\XUI_SettingsAll.h"
#include "..\Common\XUI\XUI_SettingsOptions.h"
#include "..\Common\XUI\XUI_SettingsAudio.h"
#include "..\Common\XUI\XUI_SettingsControl.h"
#include "..\Common\XUI\XUI_SettingsGraphics.h"
#include "..\Common\XUI\XUI_SettingsUI.h"
#include "..\Common\XUI\XUI_Leaderboards.h"
#include "..\Common\XUI\XUI_Debug.h"
#include "..\Common\XUI\XUI_DebugTips.h"
#include "..\Common\XUI\XUI_Reinstall.h"
#include "..\Common\XUI\XUI_Death.h"
#include "..\Common\XUI\XUI_DebugOverlay.h"
#include "..\Common\XUI\XUI_DLCOffers.h"
#include "..\Common\XUI\XUI_SocialPost.h"
#include "..\Common\XUI\XUI_Scene_Container.h"
#include "..\Common\XUI\XUI_Scene_Furnace.h"
#include "..\Common\XUI\XUI_Scene_Inventory.h"
#include "..\Common\XUI\XUI_Scene_Trap.h"
#include "..\Common\XUI\XUI_Control_ComboBox.h"
#include "..\Common\XUI\XUI_MultiGameInfo.h"
#include "..\Common\XUI\XUI_MultiGameJoinLoad.h"
#include "..\Common\XUI\XUI_MultiGameCreate.h"
#include "..\Common\XUI\XUI_MultiGameLaunchMoreOptions.h"
#include "..\Common\XUI\XUI_SignEntry.h"
#include "..\Common\XUI\XUI_InGameInfo.h"
#include "..\Common\XUI\XUI_TrialExitUpsell.h"
#include "..\Common\XUI\XUI_BasePlayer.h"
#include "..\Common\XUI\XUI_PartnernetPassword.h"
#include "..\Common\XUI\XUI_SaveMessage.h"
#include "..\Common\XUI\XUI_Chat.h"
#include "..\Common\XUI\XUI_SkinSelect.h"
#include "..\Common\XUI\XUI_InGameHostOptions.h"
#include "..\Common\XUI\XUI_InGamePlayerOptions.h"
#include "..\Common\XUI\XUI_Scene_Enchant.h"
#include "..\Common\XUI\XUI_Scene_BrewingStand.h"
#include "..\Common\XUI\XUI_XZP_Icons.h"
#include "..\Common\XUI\XUI_Controls.h"
#include "..\Common\XUI\XUI_Scene_Win.h"
#include "..\Common\XUI\XUI_HUD.h"
#include "..\Common\XUI\XUI_DebugSetCamera.h"
//#include "..\Common\XUI\XUI_TransferToXboxOne.h"

#ifdef _DEBUG_MENUS_ENABLED
#include "..\Common\XUI\XUI_DebugItemEditor.h"
#endif
#ifndef _CONTENT_PACKAGE
#include "..\Common\XUI\XUI_DebugSchematicCreator.h"
#endif

#include "..\..\Minecraft.World\Recipy.h"
#include "GameConfig\Minecraft.spa.h"
#include "..\..\Minecraft.Client\Options.h"
#include "..\..\Minecraft.World\AABB.h"
#include "..\..\Minecraft.World\Vec3.h"
#include "..\MinecraftServer.h"
#include "..\GameRenderer.h"
#include "..\ProgressRenderer.h"
#include "..\..\Minecraft.Client\LevelRenderer.h"
#include "..\..\Minecraft.Client\MobSkinMemTextureProcessor.h"

#include "..\..\Minecraft.Client\Minecraft.h"

#include "..\ClientConnection.h"
#include "..\MultiPlayerLocalPlayer.h"
#include "..\..\Minecraft.Client\LocalPlayer.h"
#include "..\..\Minecraft.World\Player.h"
#include "..\..\Minecraft.World\Inventory.h"
#include "..\..\Minecraft.World\Level.h"
#include "..\..\Minecraft.World\FurnaceTileEntity.h"
#include "..\..\Minecraft.World\Container.h"
#include "..\..\Minecraft.World\DispenserTileEntity.h"
#include "..\..\Minecraft.World\SignTileEntity.h"
#include "..\..\Minecraft.Client\StatsCounter.h"
#include "..\GameMode.h"
#include "Social\SocialManager.h"
#include <xgraphics.h>
#include "..\Common\Tutorial\TutorialMode.h"
#include "..\..\Minecraft.Client\Xbox\XML\ATGXmlParser.h"
#include "..\..\Minecraft.Client\Xbox\XML\xmlFilesCallback.h"
#include "4JLibs\inc\4J_Input.h"
#include "Font\XUI_FontRenderer.h"
#include "..\Common\Minecraft_Macros.h"
#include "..\..\Minecraft.Client\PlayerList.h"
#include "..\..\Minecraft.Client\ServerPlayer.h"
#include "..\Common\GameRules\ConsoleGameRules.h"
#include "..\Common\GameRules\ConsoleSchematicFile.h"
#include "..\..\Minecraft.World\InputOutputStream.h"
#include "..\..\Minecraft.World\LevelSettings.h"
#include "..\User.h"

#include "Sentient\SentientManager.h"
#include "..\..\Minecraft.World\LevelData.h"
#include "..\..\Minecraft.World\net.minecraft.world.entity.player.h"
#include "..\..\Minecraft.Client\EntityRenderDispatcher.h"
#include "..\..\Minecraft.World\compression.h"
#include "..\TexturePackRepository.h"
#include "..\TexturePack.h"
#include "..\Common\DLC\DLCAudioFile.h"
#include "..\DLCTexturePack.h"

#include "..\Common\XUI\XUI_Intro.h"
#include "..\Common\XUI\XUI_MainMenu.h"
#include "..\Common\XUI\XUI_NewUpdateMessage.h"
#include "..\Common\XUI\XUI_HelpAndOptions.h"
#include "..\Common\XUI\XUI_TextEntry.h"
#include "..\Common\XUI\XUI_HelpHowToPlay.h"
#include "..\Common\XUI\XUI_HowToPlayMenu.h"
#include "..\Common\XUI\XUI_HelpControls.h"
#include "..\Common\XUI\XUI_TextEntry.h"
#include "..\Common\XUI\XUI_LoadSettings.h"
#include "..\Common\XUI\XUI_SettingsAll.h"
#include "..\Common\XUI\XUI_SettingsOptions.h"
#include "..\Common\XUI\XUI_SettingsAudio.h"
#include "..\Common\XUI\XUI_SettingsControl.h"
#include "..\Common\XUI\XUI_SettingsGraphics.h"
#include "..\Common\XUI\XUI_SettingsUI.h"
#include "..\Common\XUI\XUI_Leaderboards.h"
#include "..\Common\XUI\XUI_Debug.h"
#include "..\Common\XUI\XUI_DebugTips.h"
#include "..\Common\XUI\XUI_Reinstall.h"
#include "..\Common\XUI\XUI_Death.h"
#include "..\Common\XUI\XUI_DebugOverlay.h"
#include "..\Common\XUI\XUI_DLCOffers.h"
#include "..\Common\XUI\XUI_SocialPost.h"
#include "..\Common\XUI\XUI_Scene_Container.h"
#include "..\Common\XUI\XUI_Scene_Furnace.h"
#include "..\Common\XUI\XUI_Scene_Inventory.h"
#include "..\Common\XUI\XUI_Scene_Trap.h"
#include "..\Common\XUI\XUI_Control_ComboBox.h"
#include "..\Common\XUI\XUI_MultiGameInfo.h"
#include "..\Common\XUI\XUI_MultiGameJoinLoad.h"
#include "..\Common\XUI\XUI_MultiGameCreate.h"
#include "..\Common\XUI\XUI_MultiGameLaunchMoreOptions.h"
#include "..\Common\XUI\XUI_SignEntry.h"
#include "..\Common\XUI\XUI_InGameInfo.h"
#include "..\Common\XUI\XUI_TrialExitUpsell.h"
#include "..\Common\XUI\XUI_BasePlayer.h"
#include "..\Common\XUI\XUI_PartnernetPassword.h"
#include "..\Common\XUI\XUI_SaveMessage.h"
#include "..\Common\XUI\XUI_Chat.h"
#include "..\Common\XUI\XUI_SkinSelect.h"
#include "..\Common\XUI\XUI_InGameHostOptions.h"
#include "..\Common\XUI\XUI_InGamePlayerOptions.h"
#include "..\Common\XUI\XUI_Scene_Enchant.h"
#include "..\Common\XUI\XUI_Scene_BrewingStand.h"
#include "..\Common\XUI\XUI_XZP_Icons.h"
#include "..\Common\XUI\XUI_Controls.h"
#include "..\Common\XUI\XUI_Scene_Win.h"
#include "..\Common\XUI\XUI_HUD.h"
#include "..\Common\XUI\XUI_Scene_Anvil.h"
#include "..\Common\XUI\XUI_Scene_Trading.h"
#include "..\Common\XUI\XUI_Teleport.h"

#ifdef _DEBUG_MENUS_ENABLED
#include "..\Common\XUI\XUI_DebugItemEditor.h"
#endif
#ifndef _CONTENT_PACKAGE
#include "..\Common\XUI\XUI_DebugSchematicCreator.h"
#endif

#include "Xbox_App.h"

CConsoleMinecraftApp app;

LPCWSTR apwstrLocale[10] =
{
	L"", // the default locale
	L"", // English
	L"ja-JP", // Japanese
	L"de-DE", // German
	L"fr-FR", // French
	L"es-ES", // Spanish
	L"it-IT", // Italian
	L"ko-KR", // Korean
	L"zh-CHT",// Traditional Chinese
	L"pt-PT"  // Portuguese
};

WCHAR *CConsoleMinecraftApp::wchTypefaceA[]=
{
	L"Mojangles",
	L"SDBookM",
	L"DFGMaruGothic-Md",
	L"DFHeiMedium-B5",
};

WCHAR *CConsoleMinecraftApp::wchTypefaceLocatorA[]=
{
	L"media/font/Mojangles.ttf",
	L"media/font/KOR/BOKMSD.ttf",
	L"media/font/JPN/DFGMaruGothic-Md.ttf",
	L"media/font/CHT/DFHeiMedium-B5.ttf",
};

WCHAR *CConsoleMinecraftApp::wchSceneA[]=
{
	L"xuiscene_partnernetpassword",
	L"xuiscene_intro",
	L"xuiscene_savemessage",
	L"xuiscene_main",
	L"xuiscene_fullscreenprogress",
	L"xuiscene_pause",
	L"xuiscene_craftingpanel_2x2",
	L"xuiscene_craftingpanel_3x3",
	L"xuiscene_furnace",
	L"xuiscene_container",
	L"xuiscene_container_large",
	L"xuiscene_inventory",
	L"xuiscene_trap",
	L"xuiscene_debug",
	L"xuiScene_DebugTips",
	L"xuiscene_helpandoptions",
	L"xuiscene_howtoplay",
	L"xuiscene_howtoplay_menu",
	L"xuiscene_controls",
	L"xuiscene_settings_options",
	L"xuiscene_settings_audio",
	L"xuiscene_settings_control",
	L"xuiscene_settings_graphics",
	L"xuiscene_settings_UI",
	L"xuiscene_settings_all",
	L"xuiscene_leaderboards",
	L"xuiscene_credits",
	L"xuiscene_death",
	L"xuiscene_tutorialpopup",
	L"xuiscene_multi_create",
	L"xuiscene_multi_joinload",
	L"xuiscene_multi_gameinfo",
	L"xuiscene_signentry",
	L"xuiscene_ingameinfo",
	L"xuiscene_connectingprogress",
	L"xuiscene_DLCOffers",
	L"xuiscene_socialpost",
	L"xuiscene_trialexitupsell",
	L"xuiscene_load_settings",
	L"xuiscene_chat",
	L"xuiscene_reinstall",
	L"xuiscene_skinselect",
	L"xuiscene_text_entry",
	L"xuiscene_ingame_host_options",
	L"xuiscene_ingame_player_options",
	L"xuiscene_inventory_creative",
	L"xuiscene_multi_launch_more_options",

	L"xuiscene_DLCMain",
	L"xuiscene_NewUpdateMessage",
	L"xuiscene_enchant",
	L"xuiscene_brewingstand",
	L"xuiscene_win",
	L"xuiscene_hud",
	L"xuiscene_trading",
	L"xuiscene_anvil",
	L"xuiscene_teleportmenu",
//	L"xuiscene_TransferToXboxOne",

#ifdef _DEBUG_MENUS_ENABLED
	L"xuiscene_debugoverlay",
	L"xuiscene_debug_item_editor",
#endif
#ifndef _CONTENT_PACKAGE
	L"xuiscene_debug_schematic_create",
	L"xuiscene_debug_set_camera",
#endif
};

CConsoleMinecraftApp::CConsoleMinecraftApp() : CMinecraftApp()
{
	ZeroMemory(m_PreviewBuffer,sizeof(XSOCIAL_PREVIEWIMAGE)*XUSER_MAX_COUNT);
	m_bTMSPP_GlobalFileListRead=false;
	m_bRead_TMS_XUIDS_XML=false;
	m_bRead_TMS_Config_XML=false;
	m_bRead_TMS_DLCINFO_XML=false;
	m_pXuidsFileBuffer=NULL;
	m_dwXuidsFileSize=0;
	ZeroMemory(m_ScreenshotBuffer,sizeof(LPD3DXBUFFER)*XUSER_MAX_COUNT);
	m_ThumbnailBuffer=NULL;
#ifdef _DEBUG_MENUS_ENABLED
	debugOverlayCreated = false;
#endif

	for(int i=0;i<XUSER_MAX_COUNT;i++)
	{
		m_iCountDown[i]=0;
		m_bMenuToBeClosed[i]=false;
		m_bMenuDisplayed[i]=false;
		m_bPauseMenuDisplayed[i] = false;
		m_bContainerMenuDisplayed[i]=false;
		m_bIgnoreAutosaveMenuDisplayed[i]=false;
		m_bIgnorePlayerJoinMenuDisplayed[i]=false;
		m_hCurrentScene[i]=NULL;
		m_hFirstScene[i]=NULL;
	}

	m_titleDeploymentType = XTITLE_DEPLOYMENT_DOWNLOAD;
    DWORD dwResult = XTitleGetDeploymentType(&m_titleDeploymentType, NULL);
    if( dwResult == ERROR_SUCCESS )
    {
        switch( m_titleDeploymentType )
        {
        case XTITLE_DEPLOYMENT_OPTICAL_DISC:
			DebugPrintf("Deployment type is: Disc\n");
            // Disc-based title.
            break;
        case XTITLE_DEPLOYMENT_INSTALLED_TO_HDD:
			DebugPrintf("Deployment type is: Installed to HDD\n");
            // Disc-based title has been installed to the hard drive.
            break;
        case XTITLE_DEPLOYMENT_DOWNLOAD:
			DebugPrintf("Deployment type is: Download\n");
            // Title is deployed as a downloadable title.
            break;
        case XTITLE_DEPLOYMENT_OTHER:
            // Drop through.
        default:
			DebugPrintf("Deployment type is: Other/Default\n");
            // Unknown deployment type.
        }
    }

}

void CConsoleMinecraftApp::SetRichPresenceContext(int iPad, int contextId)
{
	ProfileManager.SetRichPresenceContextValue(iPad,CONTEXT_GAME_STATE,contextId);
}


//--------------------------------------------------------------------------------------
// Name: RegisterXuiClasses()
// Desc: Registers all the scene classes.
//--------------------------------------------------------------------------------------
HRESULT CConsoleMinecraftApp::RegisterXuiClasses()
{
	// Register any other classes necessary for the app/scene
	HRESULT hr;

#ifdef _CONTENT_PACKAGE
#ifndef _FINAL_BUILD
	hr= CScene_PartnernetPassword::Register();
	if( FAILED( hr ) ) return hr;
#endif
#endif

	hr = CXuiSceneInventoryCreative::Register();
	if( FAILED( hr ) ) return hr;

	hr = CScene_NewUpdateMessage::Register();
	if( FAILED( hr ) ) return hr;

#ifdef _DEBUG_MENUS_ENABLED
	hr = CScene_DebugItemEditor::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_DebugTips::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_DebugOverlay::Register();
	if( FAILED( hr ) ) return hr;
#endif
#ifndef _CONTENT_PACKAGE
	hr = CScene_DebugSchematicCreator::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_DebugSetCamera::Register();
	if( FAILED( hr) ) return hr;
#endif

	hr = CScene_Reinstall::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_SaveMessage::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_Intro::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_Main::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_Debug::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_HelpAndOptions::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_HowToPlay::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_HowToPlayMenu::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_Controls::Register();
	if( FAILED( hr ) ) return hr;

	// Settings scenes
	hr = CScene_SettingsAll::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_SettingsOptions::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_SettingsAudio::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_SettingsControl::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_SettingsGraphics::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_SettingsUI::Register();
	if( FAILED( hr ) ) return hr;

	hr = CScene_Leaderboards::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_Credits::Register();
	if( FAILED( hr ) ) return hr;
	hr = UIScene_PauseMenu::Register();
	if( FAILED( hr ) ) return hr;
	hr = CXuiSceneCraftingPanel::Register();
	if( FAILED( hr ) ) return hr;
	hr = CXuiSceneBase::Register();
	if( FAILED( hr ) ) return hr;
	hr = CXuiCtrlBurnProgress::Register();
	if( FAILED( hr ) ) return hr;
	hr = CXuiCtrlFireProgress::Register();
	if( FAILED( hr ) ) return hr;
	hr = CXuiCtrlSlotItem::Register();
	if( FAILED( hr ) ) return hr;
	hr = CXuiCtrlSlotList::Register();
	if( FAILED( hr ) ) return hr;
	hr = CXuiCtrlSlotItemListItem::Register();
	if( FAILED( hr ) ) return hr;
	hr = CXuiSceneContainer::Register();
	if( FAILED( hr ) ) return hr;
	hr = CXuiSceneFurnace::Register();
	if( FAILED( hr ) ) return hr;
	hr = CXuiSceneInventory::Register();
	if( FAILED( hr ) ) return hr;
	hr = CXuiSceneTrap::Register();
	if( FAILED( hr ) ) return hr;
	hr = CXuiCtrl4JEdit::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_FullscreenProgress::Register();
	if( FAILED( hr ) ) return hr;
	hr = CXuiCtrlLoadingProgress::Register();
	if( FAILED( hr ) ) return hr;
	hr = CXuiCtrlMinecraftSlot::Register();
	if( FAILED( hr ) ) return hr;
	hr = CXuiCtrlMinecraftPlayer::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_Death::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_TutorialPopup::Register();
	if( FAILED( hr ) ) return hr;
	hr = CXuiCtrl4JList::Register();
	if( FAILED( hr ) ) return hr;
	hr = CXuiCtrlCraftIngredientSlot::Register();
	if( FAILED( hr ) ) return hr;
	hr = CXuiCtrlSliderWrapper::Register();
	if( FAILED( hr ) ) return hr;
	hr = CXuiCtrlPassThroughList::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_MultiGameCreate::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_MultiGameLaunchMoreOptions::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_MultiGameJoinLoad::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_MultiGameInfo::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_SignEntry::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_InGameInfo::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_ConnectingProgress::Register();
	if( FAILED( hr ) ) return hr;
	hr = CXuiSceneBasePlayer::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_DLCOffers::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_SocialPost::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_TrialExitUpsell::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_LoadGameSettings::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_Chat::Register();
	if( FAILED( hr ) ) return hr;
	hr = CXuiCtrlSplashPulser::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_SkinSelect::Register();
	if( FAILED( hr ) ) return hr;
	hr = CXuiCtrlMinecraftSkinPreview::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_TextEntry::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_InGameHostOptions::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_InGamePlayerOptions::Register();
	if( FAILED( hr ) ) return hr;
	hr = CScene_DLCMain::Register();
	if( FAILED( hr) ) return hr;
	hr = CXuiSceneEnchant::Register();
	if( FAILED( hr) ) return hr;
	hr = CXuiCtrlEnchantmentBook::Register();
	if( FAILED( hr) ) return hr;
	hr = CXuiCtrlEnchantmentButton::Register();
	if( FAILED( hr) ) return hr;
	hr = CXuiSceneBrewingStand::Register();
	if( FAILED( hr) ) return hr;
	hr = CXuiCtrlBrewProgress::Register();
	if( FAILED( hr) ) return hr;
	hr = CXuiCtrlBubblesProgress::Register();
	if( FAILED( hr) ) return hr;
	hr = CXuiCtrlEnchantmentButtonText::Register();
	if( FAILED( hr) ) return hr;
	hr = CXuiCtrlMobEffect::Register();
	if( FAILED( hr) ) return hr;
	hr = CScene_Win::Register();
	if( FAILED( hr) ) return hr;
	hr = CXuiSceneHud::Register();
	if( FAILED( hr) ) return hr;
	hr = CXuiSceneAnvil::Register();
	if( FAILED( hr) ) return hr;
	hr = CXuiSceneTrading::Register();
	if( FAILED( hr) ) return hr;
	hr = CScene_Teleport::Register();
	if( FAILED( hr) ) return hr;


	hr = CXuiCtrl4JIcon::Register();
	if( FAILED( hr ) ) return hr;
	hr = XuiHtmlRegister();
	if( FAILED( hr ) ) return hr;

//	hr = CScene_TransferToXboxOne::Register();
	if( FAILED( hr ) ) return hr;


	return S_OK;
}


//--------------------------------------------------------------------------------------
// Name: UnregisterXuiClasses()
// Desc: Unregisters all the scene classes.
//--------------------------------------------------------------------------------------
HRESULT CConsoleMinecraftApp::UnregisterXuiClasses()
{
	XuiHtmlUnregister();

	CScene_TextEntry::Unregister();

#ifndef _CONTENT_PACKAGE
	CScene_DebugSchematicCreator::Unregister();
	CScene_DebugSetCamera::Unregister();
#endif
#ifdef _DEBUG_MENUS_ENABLED
	CScene_DebugItemEditor::Unregister();
	CScene_DebugTips::Unregister();
	CScene_DebugOverlay::Unregister();
#endif
	CScene_Teleport::Unregister();
	CXuiSceneTrading::Unregister();
	CXuiSceneAnvil::Unregister();
//	CScene_TransferToXboxOne::Unregister();
	CXuiSceneHud::Unregister();
	CScene_Win::Unregister();
	CXuiCtrlMobEffect::Unregister();
	CXuiCtrlEnchantmentButtonText::Unregister();
	CXuiCtrlBubblesProgress::Unregister();
	CXuiCtrlBrewProgress::Unregister();
	CXuiSceneBrewingStand::Unregister();
	CXuiCtrlEnchantmentButton::Unregister();
	CXuiCtrlEnchantmentBook::Unregister();
	CXuiSceneEnchant::Unregister();
	CScene_InGamePlayerOptions::Unregister();
	CScene_InGameHostOptions::Unregister();
	CXuiCtrl4JIcon::Unregister();
	CXuiCtrlMinecraftSkinPreview::Unregister();
	CScene_SkinSelect::Unregister();
	CXuiCtrlSplashPulser::Unregister();
	CScene_Chat::Unregister();
	CScene_LoadGameSettings::Unregister();
	CScene_TrialExitUpsell::Unregister();
	CScene_SocialPost::Unregister();
	CScene_DLCOffers::Unregister();
	CXuiSceneBasePlayer::Unregister();
	CScene_ConnectingProgress::Unregister();
	CScene_InGameInfo::Unregister();
	CScene_SignEntry::Unregister();
	CScene_MultiGameInfo::Unregister();
	CScene_MultiGameJoinLoad::Unregister();
	CScene_MultiGameLaunchMoreOptions::Unregister();
	CScene_MultiGameCreate::Unregister();
	CXuiCtrlPassThroughList::Unregister();
	CXuiCtrlSliderWrapper::Unregister();
	CXuiCtrlCraftIngredientSlot::Unregister();
	CXuiCtrl4JList::Unregister();
	CScene_TutorialPopup::Unregister();
	CScene_Death::Unregister();
	CXuiCtrlMinecraftPlayer::Unregister();
	CXuiCtrlMinecraftSlot::Unregister();
	CXuiCtrlLoadingProgress::Unregister();
	CScene_FullscreenProgress::Unregister();
	CXuiCtrl4JEdit::Unregister();
	CXuiCtrlBurnProgress::Unregister();
	CXuiCtrlFireProgress::Unregister();
	CXuiCtrlSlotItem::Unregister();
	CXuiCtrlSlotList::Unregister();
	CXuiCtrlSlotItemListItem::Unregister();
	CXuiSceneContainer::Unregister();
	CXuiSceneFurnace::Unregister();
	CXuiSceneInventory::Unregister();
	CXuiSceneTrap::Unregister();
	CXuiSceneBase::Unregister();
	CXuiSceneCraftingPanel::Unregister();
	UIScene_PauseMenu::Unregister();
	CScene_Credits::Unregister();
	CScene_Leaderboards::Unregister();
	CScene_Controls::Unregister();
	CScene_HowToPlay::Unregister();
	CScene_HowToPlayMenu::Unregister();
	CScene_HelpAndOptions::Unregister();
	CScene_Main::Unregister();
	CScene_Debug::Unregister();
	CScene_Intro::Unregister();
	CScene_SaveMessage::Unregister();
	CScene_Reinstall::Unregister();
	CScene_DLCMain::Unregister();

	CScene_SettingsAll::Unregister();
	CScene_SettingsOptions::Unregister();
	CScene_SettingsAudio::Unregister();
	CScene_SettingsControl::Unregister();
	CScene_SettingsGraphics::Unregister();
	CScene_SettingsUI::Unregister();

	CScene_NewUpdateMessage::Unregister();

#ifdef _CONTENT_PACKAGE
#ifndef _FINAL_BUILD
	CScene_PartnernetPassword::Unregister();
#endif
#endif

	return S_OK;
}



void CConsoleMinecraftApp::GetPreviewImage(int iPad,XSOCIAL_PREVIEWIMAGE *preview)
{
	preview->Pitch = m_PreviewBuffer[iPad].Pitch;
	preview->Width = m_PreviewBuffer[iPad].Width;
	preview->Height = m_PreviewBuffer[iPad].Height;
	preview->Format = m_PreviewBuffer[iPad].Format;
	unsigned int sizeBytes = XGAddress2DTiledExtent(m_PreviewBuffer[iPad].Width, m_PreviewBuffer[iPad].Height, 4) * 4;
	preview->pBytes = (BYTE *)XPhysicalAlloc(sizeBytes, MAXULONG_PTR, 0, PAGE_READWRITE | PAGE_WRITECOMBINE );
	memcpy( (void *)preview->pBytes, (void *)m_PreviewBuffer[iPad].pBytes, sizeBytes );
	XPhysicalFree((LPVOID)m_PreviewBuffer[iPad].pBytes);
	m_PreviewBuffer[iPad].pBytes = NULL;
}

void CConsoleMinecraftApp::CaptureScreenshot(int iPad)
{
	RenderManager.CaptureScreen(&m_ScreenshotBuffer[iPad], &m_PreviewBuffer[iPad]);
}


HRESULT CConsoleMinecraftApp::LoadXuiResources()
{
	m_fontRenderer = new XUI_FontRenderer();

	OverrideFontRenderer(true);

	const DWORD LOCATOR_SIZE = 256; // Use this to allocate space to hold a ResourceLocator string
	WCHAR szResourceLocator[ LOCATOR_SIZE ];
	HRESULT hr;

	// load from the .xzp file
	const ULONG_PTR c_ModuleHandle = (ULONG_PTR)GetModuleHandle(NULL);


	//#ifdef _CONTENT_PACKAGE
	// Get the current language setting from the console
	DWORD dwLanguage = XGetLanguage( );

// 	if( dwLanguage>=sizeof( apwstrLocale )/sizeof( apwstrLocale[0] ) )
// 	{
// 		// Use default locale if out of bounds
// 		dwLanguage = 0;
// 	}
	// force default to English for now
	//dwLanguage=1;
	// Tell XUI what the locale is

	// 4J-PB - we need to load all fonts in an order, and set them as fallbacks of each other - if you save a game in an Asian language, you must be able to switch to English, and still see the Asian characters in the save name

	switch(dwLanguage)
	{

	case XC_LANGUAGE_KOREAN:
		//fallback to Japanese, Chinese
		RegisterFont(eFont_Chinese,eFont_None);
		RegisterFont(eFont_Japanese,eFont_Chinese);
		RegisterFont(eFont_Korean,eFont_Japanese,true);
		XuiSetLocale( L"ko-KR" );
		break;
	case XC_LANGUAGE_JAPANESE:
		//fallback to Korean, Chinese
		RegisterFont(eFont_Chinese,eFont_None);
		RegisterFont(eFont_Korean,eFont_Chinese);
		RegisterFont(eFont_Japanese,eFont_Korean,true);
		XuiSetLocale( L"ja-JP" );
		break;
	case XC_LANGUAGE_TCHINESE:
		//fallback to Korean, Japanese
		RegisterFont(eFont_Japanese,eFont_None);
		RegisterFont(eFont_Korean,eFont_Japanese);
		RegisterFont(eFont_Chinese,eFont_Korean,true);
		XuiSetLocale( L"zh-CHT" );
		break;


	default:
		//fallback to Korean,Japanese, Chinese
		RegisterFont(eFont_Chinese,eFont_None);
		RegisterFont(eFont_Japanese,eFont_Chinese);
		RegisterFont(eFont_Korean,eFont_Japanese);
		RegisterFont(eFont_European,eFont_Korean,true);

		// some locales can override the language setting
		bool bOverrideLanguage=true;

		// 4J-PB - removing all the new languages until a future TU

		switch(XGetLocale())
		{
#ifdef NEW_LANGUAGES
		case XC_LOCALE_CZECH_REPUBLIC:
			XuiSetLocale( L"cs-CZ" );
			break;
		case XC_LOCALE_DENMARK:
			XuiSetLocale( L"da-DK" );
			break;
		case XC_LOCALE_FINLAND:
			XuiSetLocale( L"fi-FI" );
			break;
		case XC_LOCALE_GREECE:
			XuiSetLocale( L"el-GR" );
			break;
		case XC_LOCALE_SLOVAK_REPUBLIC:
			XuiSetLocale( L"sk-SK" );
			break;
#endif
		default:
			bOverrideLanguage=false;
			break;
		}

		if(bOverrideLanguage==false)
		{
			switch(dwLanguage)
			{
			case XC_LANGUAGE_ENGLISH:
				switch(XGetLocale())
				{
#ifdef NEW_LANGUAGES
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
					XuiSetLocale( L"en-GB" );
					break;
#endif
				default:
					XuiSetLocale( L"" );
					break;
				}
				break;
			case XC_LANGUAGE_PORTUGUESE:
				if(XGetLocale()==XC_LOCALE_BRAZIL)
				{
					// Brazilian Portuguese
					XuiSetLocale( L"pt-BR" );
				}
				else
				{
					XuiSetLocale( L"pt-PT" );
				}
				break;
			case XC_LANGUAGE_GERMAN:
				XuiSetLocale( L"de-DE" );
				break;
			case XC_LANGUAGE_FRENCH:
				XuiSetLocale( L"fr-FR" );
				break;
			case XC_LANGUAGE_SPANISH:
				switch(XGetLocale())
				{
#ifdef NEW_LANGUAGES
				case XC_LOCALE_ARGENTINA:
				case XC_LOCALE_CHILE:
				case XC_LOCALE_COLOMBIA:
				case XC_LOCALE_MEXICO:
				case XC_LOCALE_UNITED_STATES:
					XuiSetLocale( L"es-MX" );
					break;
#endif
				default:
					XuiSetLocale( L"es-ES" );
					break;
				}
				break;
			case XC_LANGUAGE_ITALIAN:
				XuiSetLocale( L"it-IT" );
				break;
#ifdef NEW_LANGUAGES
			case XC_LANGUAGE_POLISH:
				XuiSetLocale( L"pl-PL" );
				break;
			case XC_LANGUAGE_RUSSIAN:
				XuiSetLocale( L"ru-RU" );
				break;
			case XC_LANGUAGE_SWEDISH:
				XuiSetLocale( L"sv-SE" );
				break;
			case XC_LANGUAGE_TURKISH:
				XuiSetLocale( L"tr-TR" );
				break;
			case XC_LANGUAGE_BNORWEGIAN:
				XuiSetLocale( L"nb-NO" );
				break;
			case XC_LANGUAGE_DUTCH:
				// 4J-PB - the loc people gave us nl-BE but no nl-NL. They claim they are identical, so use nl-BE.
				//if(XGetLocale()==XC_LOCALE_BELGIUM)
				{
					// Belgian Dutch
					XuiSetLocale( L"nl-BE" );
				}
// 				else
// 				{
// 					XuiSetLocale( L"nl-NL" );
// 				}
				break;
			case XC_LANGUAGE_SCHINESE:
				// fall back to English until we have a font, and Microsoft are happy with the Chinese Simplified text
				//XuiSetLocale( L"zh-CHS" );
				XuiSetLocale( L"" );
				break;
#endif
			}
		}
	}


	//#endif

	XuiSoundXACTRegister();
	XuiSoundXAudioRegister();

	// Register a default typeface
	// 	wsprintfW(szResourceLocator,L"section://%X,%s#%s",c_ModuleHandle,L"media", L"media/font/Mojangles.ttf");
	//  	hr = RegisterDefaultTypeface( L"Mojangles",szResourceLocator);

	// Load the skin file used for the scene.
	wsprintfW(szResourceLocator,L"section://%X,%s#%s",c_ModuleHandle,L"media", L"media/skin_Minecraft.xur");
	hr=LoadSkin( szResourceLocator );
	if( FAILED(hr) ) app.FatalLoadError();

	// Storage strings
	wsprintfW(szResourceLocator,L"section://%X,%s#%s",c_ModuleHandle,L"media", L"media/4J_strings.xus");
	hr=StorageManager.m_pStringTable->Load( szResourceLocator );
	if( FAILED(hr) ) app.FatalLoadError();

	wsprintfW(szResourceLocator,L"section://%X,%s#%s",c_ModuleHandle,L"media", L"media/strings.xus");
	hr=StringTable.Load( szResourceLocator );
	if( FAILED(hr) ) app.FatalLoadError();

	// #ifdef _DEBUG
	// 	// dump out the text
	// 	int iStringC=0;
	// 	LPCWSTR lpTempString;
	//
	// 	while((lpTempString=StringTable.Lookup(iStringC))!=NULL)
	// 	{
	// 		DebugPrintf("STRING %d = ",iStringC);
	// 		OutputDebugStringW(lpTempString);
	// 		DebugPrintf("\n");
	// 		iStringC++;
	// 	}
	// #endif



	// Load the xui scenes

	// 4J Stu - Add the tooltip/container scene to the root object
	// Load the scene.
	ASSERT(m_hObjRoot);
	HXUIOBJ mainBaseScene;
	wsprintfW(szResourceLocator,L"section://%X,%s#%s",c_ModuleHandle,L"media", L"media/");
	if(RenderManager.IsHiDef())
	{
		hr=LoadFirstScene( szResourceLocator, L"xuiscene_base.xur", NULL, &mainBaseScene );
	}
	else
	{
		if(RenderManager.IsWidescreen())
		{
			hr=LoadFirstScene( szResourceLocator, L"xuiscene_base.xur", NULL, &mainBaseScene );
		}
		else
		{
			hr=LoadFirstScene( szResourceLocator, L"xuiscene_base_480.xur", NULL, &mainBaseScene );
		}
	}
	if( FAILED(hr) ) app.FatalLoadError();

	XuiElementGetChildById( mainBaseScene, L"BasePlayer0", &m_PlayerBaseScene[0] );
	XuiElementGetChildById( mainBaseScene, L"BasePlayer1", &m_PlayerBaseScene[1] );
	XuiElementGetChildById( mainBaseScene, L"BasePlayer2", &m_PlayerBaseScene[2] );
	XuiElementGetChildById( mainBaseScene, L"BasePlayer3", &m_PlayerBaseScene[3] );

	for(unsigned int idx = 0; idx < XUSER_MAX_COUNT; ++idx)
	{
		XuiElementGetChildById( m_PlayerBaseScene[idx], L"XuiSceneContainer", &m_hCurrentScene[idx]);
		m_hFirstScene[idx]=m_hCurrentScene[idx];

		XuiElementGetChildById( m_PlayerBaseScene[idx], L"XuiSceneTutorialContainer", &m_hCurrentTutorialScene[idx]);
		m_hFirstTutorialScene[idx]=m_hCurrentTutorialScene[idx];

		XuiElementGetChildById( m_PlayerBaseScene[idx], L"XuiSceneChatRoot", &m_hCurrentChatScene[idx]);
		m_hFirstChatScene[idx]=m_hCurrentChatScene[idx];

		WCHAR szResourceLocator[ LOCATOR_SIZE ];
		wsprintfW(szResourceLocator,L"section://%X,%s#%s",c_ModuleHandle,L"media", L"media/");
		HXUIOBJ hScene;
		HRESULT hr = XuiSceneCreate(szResourceLocator,app.GetSceneName(eUIComponent_Chat, true,false), &idx, &hScene);
		if( FAILED(hr) ) app.FatalLoadError();

		hr = XuiSceneNavigateForward(m_hCurrentChatScene[idx], FALSE, hScene, idx);
		if (FAILED(hr))
		{
			app.DebugPrintf("chat scene navigate fail");
			XuiDestroyObject(hScene);
			return hr;
		}

		m_hCurrentChatScene[idx]=hScene;

		XuiElementGetChildById( m_PlayerBaseScene[idx], L"XuiSceneHudRoot", &m_hCurrentHudScene[idx]);
		m_hFirstHudScene[idx]=m_hCurrentHudScene[idx];

		ReloadHudScene(idx, false, true);
	}

	// get the "Press START to join in" message
	HXUIOBJ hPressStartMessage;

	XuiElementGetChildById( mainBaseScene, L"XuiPressStartMessage", &hPressStartMessage );
	// set the text
	XuiControlSetText(hPressStartMessage, GetString(IDS_PRESS_START_TO_JOIN));

	HXUIOBJ debugContainerObj;
	XuiElementGetChildById( mainBaseScene, L"XuiSceneDebugContainer", &debugContainerObj );
	debugContainerScene.Attach( debugContainerObj );

	SetResourcesLoaded(true);

	return S_OK;

}

HRESULT CConsoleMinecraftApp::RegisterFont(eFont eFontLanguage,eFont eFontFallback, bool bSetAsDefault)
{
	HRESULT hr=S_OK;
	const DWORD LOCATOR_SIZE = 256; // Use this to allocate space to hold a ResourceLocator string
	WCHAR szResourceLocator[ LOCATOR_SIZE ];
	const ULONG_PTR c_ModuleHandle = (ULONG_PTR)GetModuleHandle(NULL);

	wsprintfW(szResourceLocator,L"section://%X,%s#%s",c_ModuleHandle,L"media", wchTypefaceLocatorA[eFontLanguage]);
	// 4J Stu - Check that the font file actually exists
	HXUIRESOURCE fontTempResource;
	BOOL isMemoryResource;
	hr = XuiResourceOpenNoLoc(szResourceLocator, &fontTempResource, &isMemoryResource);
	if( FAILED(hr) ) app.FatalLoadError();
	XuiResourceClose(fontTempResource);

	if(bSetAsDefault)
	{
		if(eFontFallback!=eFont_None)
		{
			hr = RegisterDefaultTypeface( wchTypefaceA[eFontLanguage],szResourceLocator,NULL,0.0f,wchTypefaceA[eFontFallback]);
		}
		else
		{
			hr = RegisterDefaultTypeface( wchTypefaceA[eFontLanguage],szResourceLocator);
		}
	}
	else
	{
		TypefaceDescriptor typefaceDescriptorStruct;
		ZeroMemory(&typefaceDescriptorStruct,sizeof(TypefaceDescriptor));
		typefaceDescriptorStruct.szTypeface=wchTypefaceA[eFontLanguage];
		typefaceDescriptorStruct.szLocator=szResourceLocator;
		if(eFontFallback!=eFont_None)
		{
			typefaceDescriptorStruct.szFallbackTypeface= wchTypefaceA[eFontFallback];
		}
		hr = XuiRegisterTypeface(&typefaceDescriptorStruct,FALSE);
	}
	return hr;
}


// TMS++

void CConsoleMinecraftApp::TMSPP_SetTitleGroupID(LPCSTR szTitleGroupID)
{
	StorageManager.TMSPP_SetTitleGroupID(szTitleGroupID);
}

void CConsoleMinecraftApp::TMSPP_RetrieveFileList(int iPad,C4JStorage::eGlobalStorage eStorageFacility,CHAR *szPath,eTMSAction NextAction)
{
	app.DebugPrintf("CConsoleMinecraftApp::TMSPP_RetrieveFileList\n");

	if(eStorageFacility==C4JStorage::eGlobalStorage_Title)
	{
		if(m_bTMSPP_GlobalFileListRead==false)
		{
			m_bTMSPP_GlobalFileListRead=true;
			StorageManager.TMSPP_ReadFileList(iPad,eStorageFacility,szPath,&CConsoleMinecraftApp::Callback_TMSPPReadFileList,this,NextAction);
		}
		else
		{
			SetTMSAction(iPad,NextAction);
		}
	}
	else
	{
		StorageManager.TMSPP_ReadFileList(iPad,eStorageFacility,szPath,&CConsoleMinecraftApp::Callback_TMSPPReadFileList,this,NextAction);
	}


}

int CConsoleMinecraftApp::Callback_TMSPPReadFileList(void *pParam,int iPad, int iUserData, C4JStorage::PTMSPP_FILE_LIST pTmsFileList)
{
	app.DebugPrintf("CConsoleMinecraftApp::Callback_TMSPPReadFileList\n");
	CConsoleMinecraftApp* pClass = (CConsoleMinecraftApp*)pParam;

#ifdef _DEBUG
	// dump out the file list
	app.DebugPrintf("TMSPP filecount - %d\n",pTmsFileList->iCount);
	for(int i=0;i<pTmsFileList->iCount;i++)
	{
		app.DebugPrintf("TMSPP - %s\n",pTmsFileList->FileDetailsA[i].szFilename);
	}
#endif
	// change the state to the next action
	pClass->SetTMSAction(iPad,(eTMSAction)iUserData);
	return 0;
}

void CConsoleMinecraftApp::TMSPP_ReadXuidsFile(int iPad,eTMSAction NextAction)
{
	app.DebugPrintf("CConsoleMinecraftApp::TMSPP_ReadXuidsFile\n");
	C4JStorage::ETMSStatus eResult;
	// try reading the xuids.xml file (from TMS global) - only allowed to do this once an hour at the most, but we'll just read once each time the game launches

	if(m_bRead_TMS_XUIDS_XML==false)
	{
		m_bRead_TMS_XUIDS_XML=true;

		eResult=StorageManager.TMSPP_ReadFile(iPad,C4JStorage::eGlobalStorage_Title,C4JStorage::TMS_FILETYPE_BINARY,"xuids.xml",&CConsoleMinecraftApp::Callback_TMSPPReadXuidsFile,this,NextAction);

		if(eResult!=C4JStorage::ETMSStatus_ReadInProgress)
		{
			// something went wrong
			app.SetTMSAction(iPad,(eTMSAction)NextAction);
		}
	}
	else
	{
		app.SetTMSAction(iPad,(eTMSAction)NextAction);
	}
}

int CConsoleMinecraftApp::Callback_TMSPPReadXuidsFile(void *pParam,int iPad, int iUserData, C4JStorage::PTMSPP_FILEDATA pFileData, LPCSTR szFilename)
{
	app.DebugPrintf("CConsoleMinecraftApp::Callback_TMSPPReadXuidsFile\n");
	CConsoleMinecraftApp* pClass = (CConsoleMinecraftApp*)pParam;
	if(pFileData)
	{
		ATG::XMLParser xmlParser;
		xmlMojangCallback xmlCallback;

		xmlParser.RegisterSAXCallbackInterface( &xmlCallback );
		xmlParser.ParseXMLBuffer((CHAR *)pFileData->pbData,pFileData->dwSize);
		delete [] pFileData->pbData;
		delete [] pFileData;
	}

	// change the state to the next action
	pClass->SetTMSAction(iPad,(eTMSAction)iUserData);

	return 0;
}

void CConsoleMinecraftApp::TMSPP_ReadConfigFile(int iPad,eTMSAction NextAction)
{
	app.DebugPrintf("CConsoleMinecraftApp::TMSPP_ReadConfigFile\n");
	C4JStorage::ETMSStatus eResult;
	// try reading the xuids.xml file (from TMS global) - only allowed to do this once an hour at the most, but we'll just read once each time the game launches

	if(m_bRead_TMS_Config_XML==false)
	{
		m_bRead_TMS_Config_XML=true;

		eResult=StorageManager.TMSPP_ReadFile(iPad,C4JStorage::eGlobalStorage_Title,C4JStorage::TMS_FILETYPE_BINARY,"Config.xml",&CConsoleMinecraftApp::Callback_TMSPPReadConfigFile,this,NextAction);

		if(eResult!=C4JStorage::ETMSStatus_ReadInProgress)
		{
			// something went wrong - probably no Config.xml file exists in TMS++
			app.SetTMSAction(iPad,(eTMSAction)NextAction);
		}
	}
	else
	{
		app.SetTMSAction(iPad,(eTMSAction)NextAction);
	}
}

int CConsoleMinecraftApp::Callback_TMSPPReadConfigFile(void *pParam,int iPad, int iUserData, C4JStorage::PTMSPP_FILEDATA pFileData, LPCSTR szFilename)
{
	app.DebugPrintf("CConsoleMinecraftApp::Callback_TMSPPReadConfigFile\n");
	CConsoleMinecraftApp* pClass = (CConsoleMinecraftApp*)pParam;
	if(pFileData)
	{
		ATG::XMLParser xmlParser;
		xmlConfigCallback xmlCallback;

		xmlParser.RegisterSAXCallbackInterface( &xmlCallback );
		xmlParser.ParseXMLBuffer((CHAR *)pFileData->pbData,pFileData->dwSize);
		delete [] pFileData->pbData;
		delete [] pFileData;
	}

	// change the state to the next action
	pClass->SetTMSAction(iPad,(eTMSAction)iUserData);

	return 0;
}



bool CConsoleMinecraftApp::TMSPP_ReadBannedList(int iPad,eTMSAction NextAction)
{
	app.DebugPrintf("CConsoleMinecraftApp::TMSPP_ReadBannedList\n");
	C4JStorage::ETMSStatus eResult;
	bool bSendBanFileRetrievedMsg=false;

	if(GetBanListRead(iPad)==false)
	{
		// do we have one in our user filelist?
		if(StorageManager.TMSPP_InFileList(C4JStorage::eGlobalStorage_TitleUser,iPad,L"BannedList"))
		{
			SetBanListRead(iPad,true);
			ClearBanList(iPad);

			eResult=StorageManager.TMSPP_ReadFile(iPad,C4JStorage::eGlobalStorage_TitleUser,C4JStorage::TMS_FILETYPE_BINARY,"BannedList",&CConsoleMinecraftApp::Callback_TMSPPReadBannedList,this,NextAction);
			if(eResult!=C4JStorage::ETMSStatus_ReadInProgress)
			{
				// something went wrong
				app.SetTMSAction(iPad,(eTMSAction)NextAction);
				bSendBanFileRetrievedMsg=true;
			}
		}
		else
		{
			// not in the list - we'll need to fall back to TMS
			return false;
		}

	}
	else
	{
		bSendBanFileRetrievedMsg=true;
	}

	if(bSendBanFileRetrievedMsg)
	{
		ui.HandleTMSBanFileRetrieved(iPad);
	}

	app.SetTMSAction(iPad,(eTMSAction)NextAction);

	return true;
}

int CConsoleMinecraftApp::Callback_TMSPPReadBannedList(void *pParam,int iPad, int iUserData, C4JStorage::PTMSPP_FILEDATA pFileData,LPCSTR szFilename)
{
	app.DebugPrintf("CConsoleMinecraftApp::Callback_TMSPPReadBannedList\n");

	CConsoleMinecraftApp* pClass = (CConsoleMinecraftApp*)pParam;

	if(pFileData)
	{

		// put the entries into the vector
		int iEntries=pFileData->dwSize/sizeof(BANNEDLISTDATA);
		PBANNEDLISTDATA pData=(PBANNEDLISTDATA)pFileData->pbData;

		for(int i=0;i<iEntries;i++)
		{
			pClass->AddLevelToBannedLevelList(iPad,pData[i].xuid,pData[i].pszLevelName, false);
		}
		// mark the level as not checked against banned levels - it'll be checked once the level starts
		app.SetBanListCheck(iPad,false);

		delete [] pFileData->pbData;
		delete [] pFileData;
	}
	ui.HandleTMSBanFileRetrieved(iPad);

	// change the state to the next action
	pClass->SetTMSAction(iPad,(eTMSAction)iUserData);

	return 0;
}

void CConsoleMinecraftApp::TMSPP_ReadDLCFile(int iPad,eTMSAction NextAction)
{
	app.DebugPrintf("CConsoleMinecraftApp::TMSPP_ReadDLCFile\n");
	bool bRetrievedDLCFile=false;
	// try reading the DLC.xml file (from TMS global) - only allowed to do this once an hour at the most, but we'll just read once each time the game launches
	C4JStorage::ETMSStatus eResult;
	if(m_bRead_TMS_DLCINFO_XML==false)
	{
		//m_bRead_TMS_DLCINFO_XML=true;

		eResult=StorageManager.TMSPP_ReadFile(iPad,C4JStorage::eGlobalStorage_Title,C4JStorage::TMS_FILETYPE_BINARY,"DLC.cmp",&CConsoleMinecraftApp::Callback_TMSPPReadDLCFile,this,NextAction);
		if(eResult!=C4JStorage::ETMSStatus_ReadInProgress)
		{
			// something went wrong
			app.SetTMSAction(iPad,(eTMSAction)NextAction);
			bRetrievedDLCFile=true;
		}
	}
	else
	{
		bRetrievedDLCFile=true;
	}

	if(bRetrievedDLCFile)
	{
		ui.HandleTMSDLCFileRetrieved(iPad);

		app.SetTMSAction(iPad,(eTMSAction)NextAction);
	}
}

int CConsoleMinecraftApp::Callback_TMSPPReadDLCFile(void *pParam,int iPad, int iUserData, C4JStorage::PTMSPP_FILEDATA pFileData,LPCSTR szFilename)
{
	app.DebugPrintf("CConsoleMinecraftApp::Callback_TMSPPReadDLCFile\n");
	CConsoleMinecraftApp* pClass = (CConsoleMinecraftApp*)pParam;

	if(pFileData && pFileData->dwSize>0)
	{
		// the DLC.xml file is now compressed

		unsigned int uiDecompSize=*(unsigned int *)pFileData->pbData;
		unsigned int uiCompSize=((unsigned int *)pFileData->pbData)[1];

		BYTE *pDecompressedData = new BYTE [uiDecompSize];

		Compression::getCompression()->Decompress(pDecompressedData,&uiDecompSize,&((unsigned int *)pFileData->pbData)[2],uiCompSize);
		delete [] pFileData->pbData;
		delete [] pFileData;

		ATG::XMLParser xmlParser;
		xmlDLCInfoCallback xmlCallback;

		xmlParser.RegisterSAXCallbackInterface( &xmlCallback );
		xmlParser.ParseXMLBuffer((CHAR *)pDecompressedData,uiDecompSize);

		pClass->m_bRead_TMS_DLCINFO_XML=true;

		delete pDecompressedData;
	}

	ui.HandleTMSDLCFileRetrieved(iPad);

	// change the state to the next action
	pClass->SetTMSAction(iPad,(eTMSAction)iUserData);

	return 0;
}


void CConsoleMinecraftApp::ReadBannedList(int iPad,eTMSAction NextAction, bool bCallback)
{
	// try reading the banned level list file (from TMS User) - only allowed to do this once an hour at the most, but we'll just read once each time the user signs in

	if(GetBanListCheck(iPad)==false)
	{
		SetBanListCheck(iPad,true);
		ClearBanList(iPad);

		//StorageManager.DeleteTMSFile(iPad,C4JStorage::eGlobalStorage_TitleUser,L"BannedList");
		C4JStorage::ETMSStatus eTMSStatus;
		if(bCallback)
		{
			eTMSStatus=StorageManager.ReadTMSFile(iPad,C4JStorage::eGlobalStorage_TitleUser,C4JStorage::eTMS_FileType_Normal,L"BannedList",&BannedListA[iPad].pBannedList, &BannedListA[iPad].dwBytes,&CConsoleMinecraftApp::CallbackBannedListFileFromTMS,this,NextAction);
		}
		else
		{
			eTMSStatus=StorageManager.ReadTMSFile(iPad,C4JStorage::eGlobalStorage_TitleUser,C4JStorage::eTMS_FileType_Normal,L"BannedList",&BannedListA[iPad].pBannedList, &BannedListA[iPad].dwBytes);

			if(eTMSStatus==C4JStorage::ETMSStatus_Idle)
			{
				// put the entries into the vector
				int iEntries=BannedListA[iPad].dwBytes/sizeof(BANNEDLISTDATA);
				PBANNEDLISTDATA pData=(PBANNEDLISTDATA)BannedListA[iPad].pBannedList;

				for(int i=0;i<iEntries;i++)
				{
					AddLevelToBannedLevelList(iPad,pData[i].xuid,pData[i].pszLevelName, false);
				}
				// mark the level as not checked against banned levels - it'll be checked once the level starts
				app.SetBanListCheck(iPad,false);
			}
		}
	}
	else
	{
		app.SetTMSAction(iPad,(eTMSAction)NextAction);
		// Fix for #70412 - TCR 1.00: BAS Game Stability: The title enters an unresponsive state, on the following screen, when the client attempts to start a new game after leaving a hosted game.

		// if bCallback is set and we've already got the ban list (from a join in session previously), we need to send a message out to let the waiting process proceed

		if(bCallback)
		{
			ui.HandleTMSBanFileRetrieved(iPad);
		}
	}
}

int CConsoleMinecraftApp::CallbackBannedListFileFromTMS(LPVOID lpParam, WCHAR *wchFilename, int iPad, bool bResult, int iAction)
{
	CConsoleMinecraftApp* pClass = (CConsoleMinecraftApp*)lpParam;

	if(pClass->BannedListA[iPad].dwBytes>0)
	{
		// put the entries into the vector
		int iEntries=pClass->BannedListA[iPad].dwBytes/sizeof(BANNEDLISTDATA);
		PBANNEDLISTDATA pData=(PBANNEDLISTDATA)pClass->BannedListA[iPad].pBannedList;

		for(int i=0;i<iEntries;i++)
		{
			pClass->AddLevelToBannedLevelList(iPad,pData[i].xuid,pData[i].pszLevelName, false);
		}

		// mark the level as not checked against banned levels - it'll be checked once the level starts
		app.SetBanListCheck(iPad,false);

	}
	app.SetTMSAction(iPad,(eTMSAction)iAction);

	ui.HandleTMSBanFileRetrieved(iPad);

	return 0;
}

void CConsoleMinecraftApp::OverrideFontRenderer(bool set, bool immediate)
{
	// 4J Stu - Only relevant to XUI on Xbox360
#ifdef _XBOX
	// We don't want to even attempt using our font renderer in the asian languages
	switch(XGetLanguage())
	{
	case XC_LANGUAGE_JAPANESE:
	case XC_LANGUAGE_TCHINESE:
	case XC_LANGUAGE_KOREAN:
		return;
		break;
	}

	if(set != m_bFontRendererOverridden)
	{
		if( immediate )
		{
			XuiElementDiscardResources( m_hObjRoot, XUI_DISCARD_FONTS );
			if( set )
			{
				XuiFontSetRenderer( m_fontRenderer );
			}
			else
			{
				XuiFontSetRenderer( NULL );
			}
			m_bFontRendererOverridden = set;

			if( GetResourcesLoaded() )
			{
				// tell the xui scenes the font renderer changed
				XUIMessage xuiMsg;
				CustomMessage_FontRendererChange( &xuiMsg);

				// send the message
				for(int i=0;i<XUSER_MAX_COUNT;i++)
				{
					XuiBroadcastMessage( CXuiSceneBase::GetPlayerBaseScene(i), &xuiMsg );
				}
			}
		}

		m_bOverrideFontRenderer = set;
	}
#endif
}

void CConsoleMinecraftApp::RunFrame()
{
	OverrideFontRenderer( m_bOverrideFontRenderer );
	// Do the actual XUI frame

	CXuiModule::RunFrame();

}

void CConsoleMinecraftApp::CaptureSaveThumbnail()
{
	MemSect(53);
	RenderManager.CaptureThumbnail(&m_ThumbnailBuffer);
	MemSect(0);
}
void CConsoleMinecraftApp::GetSaveThumbnail(PBYTE *pbData,DWORD *pdwSize)
{
	// on a save caused by a create world, the thumbnail capture won't have happened
	if(m_ThumbnailBuffer!=NULL)
	{
		if( pbData )
		{
			*pbData= new BYTE [m_ThumbnailBuffer->GetBufferSize()];
			*pdwSize=m_ThumbnailBuffer->GetBufferSize();
			memcpy(*pbData,m_ThumbnailBuffer->GetBufferPointer(),*pdwSize);
		}
		m_ThumbnailBuffer->Release();
		m_ThumbnailBuffer=NULL;
	}
}
void CConsoleMinecraftApp::ReleaseSaveThumbnail()
{
	if(m_ThumbnailBuffer!=NULL)
	{
		m_ThumbnailBuffer->Release();
		m_ThumbnailBuffer=NULL;
	}
}

void CConsoleMinecraftApp::GetScreenshot(int iPad,PBYTE *pbData,DWORD *pdwSize)
{
	// on a save caused by a create world, the thumbnail capture won't have happened
	if(m_ScreenshotBuffer[iPad]!=NULL)
	{
		*pbData= new BYTE [m_ScreenshotBuffer[iPad]->GetBufferSize()];
		*pdwSize=m_ScreenshotBuffer[iPad]->GetBufferSize();
		memcpy(*pbData,m_ScreenshotBuffer[iPad]->GetBufferPointer(),*pdwSize);
		m_ScreenshotBuffer[iPad]->Release();
		m_ScreenshotBuffer[iPad]=NULL;
	}
}

#ifdef _DEBUG_MENUS_ENABLED
void CConsoleMinecraftApp::EnableDebugOverlay(bool enable,int iPad)
{
	HRESULT hr = S_OK;

	if(enable && !debugOverlayCreated)
	{
		const ULONG_PTR c_ModuleHandle = (ULONG_PTR)GetModuleHandle(NULL);

		const DWORD LOCATOR_SIZE = 256; // Use this to allocate space to hold a ResourceLocator string
		WCHAR szResourceLocator[ LOCATOR_SIZE ];

		wsprintfW(szResourceLocator,L"section://%X,%s#%s",c_ModuleHandle,L"media", L"media/");
		hr = XuiSceneCreate(szResourceLocator, L"xuiscene_debugoverlay.xur", NULL, &m_hDebugOverlay);
		debugContainerScene.AddChild(m_hDebugOverlay);
		debugContainerScene.SetShow(false);

		debugOverlayCreated = true;
	}
	if(enable)
	{
		XuiElementSetUserFocus(m_hDebugOverlay, iPad);
	}
	else
	{
		XuiElementSetUserFocus(m_hCurrentScene[iPad], iPad);
	}
	debugContainerScene.SetShow(enable);
	SetMenuDisplayed(ProfileManager.GetPrimaryPad(),enable);
}
#endif

bool CConsoleMinecraftApp::GetMenuDisplayed(int iPad)
{
	return m_bMenuDisplayed[iPad];
}

void CConsoleMinecraftApp::SetMenuDisplayed(int iPad,bool bVal)
{
	if(iPad==XUSER_INDEX_ANY) return;

	if(bVal)
	{
		if(iPad==XUSER_INDEX_ANY)
		{
			for(int i=0;i<XUSER_MAX_COUNT;i++)
			{
				InputManager.SetMenuDisplayed(i,true);
				m_bMenuDisplayed[i]=true;
				// 4J Stu - Fix for #11018 - Functional: When the controller is unplugged during active gameplay and plugged back in at the resulting pause menu, it will demonstrate dual-functionality.
				m_bMenuToBeClosed[iPad]=false;
				SetChatTextDisplayed(i,false);
			}
		}
		else
		{
			InputManager.SetMenuDisplayed(iPad,true);
			m_bMenuDisplayed[iPad]=true;
			// 4J Stu - Fix for #11018 - Functional: When the controller is unplugged during active gameplay and plugged back in at the resulting pause menu, it will demonstrate dual-functionality.
			m_bMenuToBeClosed[iPad]=false;
			SetChatTextDisplayed(iPad,false);
		}
	}
	else
	{
		if(iPad==XUSER_INDEX_ANY)
		{
			for(int i=0;i<XUSER_MAX_COUNT;i++)
			{
				m_bMenuToBeClosed[i]=true;
				m_iCountDown[i]=4;
			}
		}
		else
		{
			m_bMenuToBeClosed[iPad]=true;
			m_iCountDown[iPad]=4;
		}
	}
}

void CConsoleMinecraftApp::CheckMenuDisplayed()
{
	for(int iPad=0;iPad<4;iPad++)
	{
		if(m_bMenuToBeClosed[iPad])
		{
			if(m_iCountDown[iPad]!=0)
			{
				m_iCountDown[iPad]--;
			}
			else
			{
				m_bMenuToBeClosed[iPad]=false;
				m_bMenuDisplayed[iPad]=false;
				InputManager.SetMenuDisplayed(iPad,false);
				SetChatTextDisplayed(iPad,true);
			}

		}
	}
}
bool CConsoleMinecraftApp::IsSceneInStack(int iPad, EUIScene eScene)
{
	bool inStack = false;
	int idx = 0;
	if( m_bGameStarted )
	{
		// If the game isn't running treat as user 0, otherwise map index directly from pad
		if( ( iPad != 255 ) && ( iPad >= 0 ) ) idx = iPad;
	}
    auto itEnd = m_sceneStack[idx].end();
    for (auto it = m_sceneStack[idx].begin(); it != itEnd; it++)
	{
		if(it->first == eScene)
		{
			inStack = true;
			break;
		}
	}

	return inStack;
}

WCHAR *CConsoleMinecraftApp::GetSceneName(EUIScene eScene,bool bAppendToName,bool bSplitscreenScene)
{
	wcscpy(m_SceneName,wchSceneA[eScene]);
	if(bAppendToName)
	{
		if(RenderManager.IsHiDef())
		{
			if(bSplitscreenScene)
			{
				wcscat(m_SceneName,L"_small");
			}
		}
		else
		{
			// if it's low def, but widescreen, then use the small scenes
			if(!RenderManager.IsWidescreen())
			{
				wcscat(m_SceneName,L"_480");
			}

		}
	}
	wcscat(m_SceneName,L".xur");

#ifdef _DEBUG
#ifndef __PS3__
	OutputDebugStringA("SCENE LOAD - ");
	OutputDebugStringW(m_SceneName);
	app.DebugPrintf("\n");
#endif __PS3__
#endif

	return m_SceneName;
}

HRESULT CConsoleMinecraftApp::NavigateToScene(int iPad,EUIScene eScene, void *initData /* = NULL */, bool forceUsePad /*= false*/, BOOL bStayVisible /* = FALSE */, HXUIOBJ *phResultingScene /*= NULL*/ )
{
	ASSERT(m_bDefaultTypefaceRegistered);
	ASSERT(m_bSkinLoaded);
	static bool bSeenUpdateTextThisSession=false;

	// make sure we've created the main canvas already
	ASSERT(m_hObjRoot);
	if (!m_hObjRoot)
		return E_UNEXPECTED;

	// If you're navigating to the multigamejoinload, and the player hasn't seen the updates message yet, display it now
	// display this message the first 3 times
	// todo: re-enable if we fix this menu, for now its just blank!
	if(false && (eScene==eUIScene_LoadOrJoinMenu) && (bSeenUpdateTextThisSession==false) && ( app.GetGameSettings(ProfileManager.GetPrimaryPad(),eGameSetting_DisplayUpdateMessage)!=0))
	{
		eScene=eUIScene_NewUpdateMessage;
		bSeenUpdateTextThisSession=true;
	}
	// if you're trying to navigate to the inventory,the crafting, pause or game info or any of the trigger scenes and there's already a menu up (because you were pressing a few buttons at the same time) then ignore the navigate
	if(GetMenuDisplayed(iPad))
	{
		switch(eScene)
		{
		case eUIScene_PauseMenu:
		case eUIScene_Crafting2x2Menu:
		case eUIScene_Crafting3x3Menu:
		case eUIScene_FurnaceMenu:
		case eUIScene_ContainerMenu:
		case eUIScene_LargeContainerMenu:
		case eUIScene_InventoryMenu:
		case eUIScene_CreativeMenu:
		case eUIScene_DispenserMenu:
		case eUIScene_SignEntryMenu:
		case eUIScene_InGameInfoMenu:
		case eUIScene_EnchantingMenu:
		case eUIScene_BrewingStandMenu:
		case eUIScene_AnvilMenu:
		case eUIScene_TradingMenu:
			DebugPrintf("IGNORING NAVIGATE - we're trying to navigate to a user selected scene when there's already a scene up: pad:%d, scene:%d\n", iPad, eScene);
			return S_FALSE;
			break;
		}
	}

	// load from the .xzp file
	const ULONG_PTR c_ModuleHandle = (ULONG_PTR)GetModuleHandle(NULL);

	HXUIOBJ hScene;
	HRESULT hr;
	const DWORD LOCATOR_SIZE = 256; // Use this to allocate space to hold a ResourceLocator string
	WCHAR szResourceLocator[ LOCATOR_SIZE ];

	wsprintfW(szResourceLocator,L"section://%X,%s#%s",c_ModuleHandle,L"media", L"media/");

	// If the init data is null, put the player pad in there

	if(initData==NULL)
	{
		initData = &iPad;
	}

	int idx = 0;
	if( m_bGameStarted || forceUsePad )
	{
		// If the game isn't running treat as user 0, otherwise map index directly from pad
		if( ( iPad != 255 ) && ( iPad >= 0 ) ) idx = iPad;
	}
	else
	{
		// The game isn't running, so make sure this scene is fullscreen
		CXuiSceneBase::SetPlayerBaseScenePosition( idx, CXuiSceneBase::e_BaseScene_Fullscreen );
	}

	if( eScene != eUIComponent_TutorialPopup && eScene != eUIComponent_Chat && eScene != eUIScene_HUD )
	{
		SetMenuDisplayed(idx,true);
	}

	PIXBeginNamedEvent(0,"Xui scene create\n");
	MemSect(4);

	bool bIsHiDef=RenderManager.IsHiDef();
	bool bIsSplitscreen = GetLocalPlayerCount()>1;
	BOOL bSceneIsFullscreen = CXuiSceneBase::GetPlayerBasePosition(idx)==CXuiSceneBase::e_BaseScene_Fullscreen;

	switch(eScene)
	{
	case eUIScene_Intro:
	case eUIScene_MainMenu:
	case eUIScene_LeaderboardsMenu:
	case eUIScene_Credits:
	case eUIScene_CreateWorldMenu:
	case eUIScene_LoadOrJoinMenu:
	case eUIScene_JoinMenu:
	case eUIScene_DLCOffersMenu:
	case eUIScene_DLCMainMenu:
	case eUIScene_TrialExitUpsell:
	case eUIScene_SaveMessage:
	case eUIScene_LoadMenu:
	case eUIScene_LaunchMoreOptionsMenu:
		// if we're in low res widescreen, we just want to use the normal scenes for these
		if(!bIsHiDef && RenderManager.IsWidescreen())
		{
			hr = XuiSceneCreate(szResourceLocator,app.GetSceneName(eScene,false, false), initData, &hScene);
		}
		else
		{
			hr = XuiSceneCreate(szResourceLocator,app.GetSceneName(eScene, true,false), initData, &hScene);
		}
		break;
	default:
		// some scenes are forced into fullscreen even though we are insplitscreen mode (saving)
		if(eScene==eUIComponent_TutorialPopup || eScene==eUIComponent_Chat || eScene==eUIScene_HUD)
		{
			hr = XuiSceneCreate(szResourceLocator,app.GetSceneName(eScene,true,bIsSplitscreen), initData, &hScene);
		}
		else
		{
			hr = XuiSceneCreate(szResourceLocator,app.GetSceneName(eScene,true,bIsSplitscreen && !bSceneIsFullscreen), initData, &hScene);
		}
		break;
	}
	if( FAILED(hr) ) app.FatalLoadError();

	MemSect(0);
	PIXEndNamedEvent();
#if 0
	D3DXVECTOR3 test;
	test.x = 0.5f;
	test.y = 0.5f;
	test.z = 1.0f;
	XuiElementSetScale(hScene,&test);
#endif

	if( eScene == eUIComponent_TutorialPopup )
	{
		hr = XuiSceneNavigateForward(m_hCurrentTutorialScene[iPad], FALSE, hScene, XUSER_INDEX_NONE);
		if (FAILED(hr))
		{
			app.DebugPrintf("scene navigate fail");
			XuiDestroyObject(hScene);
			return hr;
		}

		m_hCurrentTutorialScene[iPad]=hScene;
	}
	else if( eScene == eUIComponent_Chat )
	{
		hr = XuiSceneNavigateForward(m_hCurrentChatScene[iPad], FALSE, hScene, XUSER_INDEX_NONE);
		if (FAILED(hr))
		{
			app.DebugPrintf("scene navigate fail");
			XuiDestroyObject(hScene);
			return hr;
		}

		m_hCurrentChatScene[iPad]=hScene;
	}
	else if( eScene == eUIScene_HUD )
	{
		hr = XuiSceneNavigateForward(m_hCurrentHudScene[iPad], FALSE, hScene, XUSER_INDEX_NONE);
		if (FAILED(hr))
		{
			app.DebugPrintf("scene navigate fail");
			XuiDestroyObject(hScene);
			return hr;
		}

		m_hCurrentHudScene[iPad]=hScene;
	}
	else
	{
		MemSect(5);
		hr = XuiSceneNavigateForward(m_hCurrentScene[idx], bStayVisible, hScene, iPad);
		MemSect(0);
		if (FAILED(hr))
		{
			app.DebugPrintf("scene navigate fail");
			XuiDestroyObject(hScene);
			return hr;
		}

		m_hCurrentScene[idx] = hScene;
		m_sceneStack[idx].push_back( SceneStackPair(eScene, hScene) );

		if(eScene==eUIScene_Intro)
		{
			m_bIntroRunning=true;
		}
	}

	switch(eScene)
	{
	case eUIScene_PauseMenu:
		{
			m_bPauseMenuDisplayed[iPad] = true;

			Minecraft *pMinecraft = Minecraft::GetInstance();
			if(pMinecraft != NULL && pMinecraft->localgameModes[iPad] != NULL )
			{
				TutorialMode *gameMode = (TutorialMode *)pMinecraft->localgameModes[iPad];

				// This just allows it to be shown
				gameMode->getTutorial()->showTutorialPopup(false);
			}
		}
		break;
	case eUIScene_Crafting2x2Menu:
	case eUIScene_Crafting3x3Menu:
	case eUIScene_FurnaceMenu:
	case eUIScene_ContainerMenu:
	case eUIScene_LargeContainerMenu:
	case eUIScene_InventoryMenu:
	case eUIScene_CreativeMenu:
	case eUIScene_DispenserMenu:
	case eUIScene_BrewingStandMenu:
	case eUIScene_EnchantingMenu:
	case eUIScene_AnvilMenu:
	case eUIScene_TradingMenu:
		m_bContainerMenuDisplayed[iPad]=true;

		// Intentional fall-through
	case eUIScene_DeathMenu:
	case eUIScene_FullscreenProgress:
	case eUIScene_SignEntryMenu:
	case eUIScene_EndPoem:
		m_bIgnoreAutosaveMenuDisplayed[iPad] = true;
		break;
	}

	switch(eScene)
	{
	case eUIScene_FullscreenProgress:
	case eUIScene_EndPoem:
		m_bIgnorePlayerJoinMenuDisplayed[iPad] = true;
		break;
	}

	if(phResultingScene!=NULL)
	{
		*phResultingScene=hScene;
	}

	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT CConsoleMinecraftApp::NavigateBack(int iPad, bool forceUsePad /*= false*/, EUIScene eScene /*= eUIScene_COUNT*/)
{
	HXUIOBJ hCur;
	HXUIOBJ hBack;

	int idx = 0;
	if( m_bGameStarted || forceUsePad )
	{
		// If the game isn't running treat as user 0, otherwise map index directly from pad
		if( ( iPad != 255 ) && ( iPad >= 0 ) ) idx = iPad;
	}

	hCur=m_hCurrentScene[idx];

	if(eScene == eUIScene_COUNT)
	{
		hBack = XuiSceneGetBackScene(m_hCurrentScene[idx]);
		if (!hBack)
			return S_FALSE;

		m_sceneStack[idx].pop_back();
	}
	else
	{
		for(auto it = m_sceneStack[idx].begin(), end = m_sceneStack[idx].end(); it != end; ++it)
		{
			if(it->first == eScene)
			{
				hBack = it->second;
			}
		}

		if (!hBack)
			return S_FALSE;

		while( !m_sceneStack[idx].empty() && m_sceneStack[idx].back().second != hBack)
		{
			m_sceneStack[idx].pop_back();
		}
	}
	m_hCurrentScene[idx]=hBack;

	if( m_hCurrentScene[idx] == this->m_hFirstScene[idx] ) SetMenuDisplayed(idx,false);

	CXuiSceneBase::PlayUISFX(eSFX_Back);
	return XuiSceneNavigateBack(hCur, hBack,iPad );
}

HRESULT CConsoleMinecraftApp::TutorialSceneNavigateBack(int iPad, bool forceUsePad /*= false*/)
{
	HXUIOBJ hCur;
	HXUIOBJ hBack;

	int idx = 0;
	if( m_bGameStarted || forceUsePad )
	{
		// If the game isn't running treat as user 0, otherwise map index directly from pad
		if( ( iPad != 255 ) && ( iPad >= 0 ) ) idx = iPad;
	}

	hCur=m_hCurrentTutorialScene[idx];
	hBack = XuiSceneGetBackScene(m_hCurrentTutorialScene[idx]);
	if (!hBack)
		return S_FALSE;
	m_hCurrentTutorialScene[idx]=hBack;

	return XuiSceneNavigateBack(hCur, hBack,iPad );
}

HRESULT CConsoleMinecraftApp::CloseXuiScenes(int iPad, bool forceUsePad /*= false*/)
{
	HRESULT hr;

	int idx = 0;
	if( m_bGameStarted || forceUsePad )
	{
		// If the game isn't running treat as user 0, otherwise map index directly from pad
		if( ( iPad != 255 ) && ( iPad >= 0 ) ) idx = iPad;
	}

	hr=XuiSceneNavigateBackToFirst(m_hCurrentScene[idx],iPad);
	m_hCurrentScene[idx]=m_hFirstScene[idx];
	m_sceneStack[idx].clear();

	CXuiSceneBase::ShowBackground(iPad, FALSE );
	CXuiSceneBase::ShowDarkOverlay(iPad, FALSE );
	CXuiSceneBase::ShowLogo(iPad, FALSE );

	m_bPauseMenuDisplayed[iPad] = false;
	m_bContainerMenuDisplayed[iPad] = false;
	m_bIgnoreAutosaveMenuDisplayed[iPad] = false;
	m_bIgnorePlayerJoinMenuDisplayed[iPad] = false;

	ui.SetTooltips( iPad, -1, -1, -1, -1 );
	SetMenuDisplayed(iPad,false);

	// Hide the tutorial popup for this player
	if(m_bGameStarted)
	{
		CScene_TutorialPopup::SetSceneVisible(iPad,FALSE);
	}

	Minecraft *pMinecraft = Minecraft::GetInstance();
	if(pMinecraft != NULL && pMinecraft->localgameModes[iPad] != NULL )
	{
		TutorialMode *gameMode = (TutorialMode *)pMinecraft->localgameModes[iPad];

		// This just allows it to be shown
		gameMode->getTutorial()->showTutorialPopup(true);
	}

	// update the rich presence
	// Are we offline or online, and how many players are there
	if(g_NetworkManager.GetPlayerCount()>1)
	{
		for(int i=0;i<XUSER_MAX_COUNT;i++)
		{
			if(pMinecraft->localplayers[i])
			{
				if(g_NetworkManager.IsLocalGame())
				{
					ProfileManager.SetCurrentGameActivity(i,CONTEXT_PRESENCE_MULTIPLAYEROFFLINE,false);
				}
				else
				{
					ProfileManager.SetCurrentGameActivity(i,CONTEXT_PRESENCE_MULTIPLAYER,false);
				}
			}
		}
	}
	else
	{
		if(m_bGameStarted && pMinecraft->localplayers[idx])
		{
			if(g_NetworkManager.IsLocalGame())
			{
				ProfileManager.SetCurrentGameActivity(iPad,CONTEXT_PRESENCE_MULTIPLAYER_1POFFLINE,false);
			}
			else
			{
				ProfileManager.SetCurrentGameActivity(iPad,CONTEXT_PRESENCE_MULTIPLAYER_1P,false);
			}
		}
	}
	// because a player leaving can cause other players close scenes to be called, and they may have been crafting, I'll run
	// the presence update for all players

	for(int i=0;i<XUSER_MAX_COUNT;i++)
	{
		if(pMinecraft->localplayers[i])
		{
			pMinecraft->localplayers[i]->updateRichPresence();
		}
	}

	return hr;
}

HRESULT	CConsoleMinecraftApp::CloseAllPlayersXuiScenes()
{
	HRESULT hr = S_OK;

	// 4J Stu - Fix for #11211 - TCR 015 BAS Sign In Changes: One time-occurrence where users were unable to access any in-game menus after several sign-in changes
	// We only hide them during a save, but this could be interrupted
	CXuiSceneBase::ShowOtherPlayersBaseScene(0, true);

	for(unsigned int idx = 0; idx < XUSER_MAX_COUNT; ++idx)
	{
		CloseXuiScenes(idx, true);
		ReloadChatScene(idx, false, true);
		ReloadHudScene(idx, false, true);
	}

	return hr;
}

HRESULT CConsoleMinecraftApp::CloseXuiScenesAndNavigateToScene(int iPad,EUIScene eScene, void *initData, bool forceUsePad /*= false*/)
{
	HRESULT hr;

	int idx = 0;
	if( m_bGameStarted || forceUsePad)
	{
		// If the game isn't running treat as user 0, otherwise map index directly from pad
		if( ( iPad != 255 ) && ( iPad >= 0 ) ) idx = iPad;
	}

	hr=XuiSceneNavigateBackToFirst(m_hCurrentScene[idx],iPad);
	m_hCurrentScene[idx]=m_hFirstScene[idx];
	m_sceneStack[idx].clear();

	CXuiSceneBase::ShowBackground(iPad, FALSE );
	CXuiSceneBase::ShowDarkOverlay(iPad, FALSE );
	CXuiSceneBase::ShowLogo(iPad, FALSE );

	m_bPauseMenuDisplayed[iPad] = false;
	m_bContainerMenuDisplayed[iPad] = false;
	m_bIgnoreAutosaveMenuDisplayed[iPad] = false;
	m_bIgnorePlayerJoinMenuDisplayed[iPad] = false;

	ui.SetTooltips( iPad, -1, -1, -1, -1 );
	SetMenuDisplayed(iPad,false);

	NavigateToScene(iPad,eScene,initData);

	return hr;
}

HRESULT CConsoleMinecraftApp::RemoveBackScene(int iPad)
{
	int idx = 0;
	if( m_bGameStarted)
	{
		// If the game isn't running treat as user 0, otherwise map index directly from pad
		if( ( iPad != 255 ) && ( iPad >= 0 ) ) idx = iPad;
	}

	if( m_hCurrentScene[idx] != m_hFirstScene[idx] )
	{
		HXUIOBJ hBack = XuiSceneGetBackScene(m_hCurrentScene[idx]);

		if( hBack != m_hFirstScene[idx] )
		{
			HXUIOBJ hNewBack = XuiSceneGetBackScene( hBack );

			if( hNewBack != m_hFirstScene[idx] )
			{
				XuiSceneSetBackScene(m_hCurrentScene[idx], hNewBack);

				for(auto it = m_sceneStack[idx].begin(), end = m_sceneStack[idx].end(); it != end; ++it)
				{
					if(it->second == hBack)
					{
						m_sceneStack[idx].erase(it);
						break;
					}
				}

				XuiSceneSetBackScene(hBack, NULL);
				XuiDestroyObject( hBack );
			}
		}
	}

	return S_OK;
}

HRESULT CConsoleMinecraftApp::NavigateToHomeMenu()
{
	HRESULT hr = S_OK;

	for(unsigned int idx = 0; idx < XUSER_MAX_COUNT; ++idx)
	{
		if( m_hFirstTutorialScene[idx] != m_hCurrentTutorialScene[idx] )
		{
			XuiSceneNavigateBack(m_hCurrentTutorialScene[idx], m_hFirstTutorialScene[idx],XUSER_INDEX_ANY);
			m_hCurrentTutorialScene[idx] = m_hFirstTutorialScene[idx];
		}
	}

	hr = CloseAllPlayersXuiScenes();

	Minecraft *pMinecraft = Minecraft::GetInstance();

	// 4J-PB - just about to switched to the default texture pack , so clean up anything texture pack related here

	// unload any texture pack audio
	// if there is audio in use, clear out the audio, and unmount the pack
	TexturePack *pTexPack=Minecraft::GetInstance()->skins->getSelected();
	DLCTexturePack *pDLCTexPack=NULL;

	if(pTexPack->hasAudio())
	{
		// get the dlc texture pack, and store it
		pDLCTexPack=(DLCTexturePack *)pTexPack;
	}

	// change to the default texture pack
	pMinecraft->skins->selectTexturePackById(TexturePackRepository::DEFAULT_TEXTURE_PACK_ID);

	if(pTexPack->hasAudio())
	{
		// need to stop the streaming audio - by playing streaming audio from the default texture pack now
		pMinecraft->soundEngine->playStreaming(L"", 0, 0, 0, 0, 0);

		if(pDLCTexPack->m_pStreamedWaveBank!=NULL)
		{
			pDLCTexPack->m_pStreamedWaveBank->Destroy();
		}
		if(pDLCTexPack->m_pSoundBank!=NULL)
		{
			pDLCTexPack->m_pSoundBank->Destroy();
		}
		DWORD result = StorageManager.UnmountInstalledDLC("TPACK");
		app.DebugPrintf("Unmount result is %d\n",result);
	}

	g_NetworkManager.ForceFriendsSessionRefresh();

	hr = NavigateToScene(XUSER_INDEX_ANY,eUIScene_MainMenu,NULL);
	return hr;
}

D3DXVECTOR3 CConsoleMinecraftApp::GetElementScreenPosition(HXUIOBJ hObj)
{
	D3DXMATRIX matrix;
	D3DXVECTOR3 elementPosition;
	XuiElementGetFullXForm( hObj, &matrix);

	elementPosition.x = matrix._41;
	elementPosition.y = matrix._42;
	elementPosition.z = matrix._43;

	return elementPosition;
}

void CConsoleMinecraftApp::SetChatTextDisplayed(int iPad, bool bVal)
{
	XuiElementSetShow( m_hCurrentChatScene[iPad] ,bVal );
}

void CConsoleMinecraftApp::ReloadChatScene(int iPad, bool bJoining /*= false*/, bool bForce /*= false*/)
{
	if(m_hFirstChatScene[iPad] == NULL || m_hCurrentChatScene[iPad] == NULL) return;

	// Re-create the chat scene so it is the correct size. It starts without any visible lines.
	BOOL chatSceneVisible = FALSE;
	if( m_hFirstChatScene[iPad] != m_hCurrentChatScene[iPad] )
	{
		chatSceneVisible = XuiElementIsShown(m_hCurrentChatScene[iPad]);
	}

	int iLocalPlayerC=GetLocalPlayerCount();
	// we need to decide whether to close the xui scene or just reposition it

	if( bForce ||
		(bJoining && iLocalPlayerC==2) ||
		(!bJoining && iLocalPlayerC==1) ||
		m_hFirstChatScene[iPad] == m_hCurrentChatScene[iPad])
	{
		if( m_hFirstChatScene[iPad] != m_hCurrentChatScene[iPad] ) XuiSceneNavigateBack(m_hCurrentChatScene[iPad], m_hFirstChatScene[iPad],iPad);
		m_hCurrentChatScene[iPad] = m_hFirstChatScene[iPad];
		app.NavigateToScene(iPad,eUIComponent_Chat,NULL,true);

		XuiElementSetShow( m_hCurrentChatScene[iPad], chatSceneVisible);
	}

	D3DXVECTOR3 vPosition;
	vPosition.x = vPosition.y = vPosition.z = 0.0f;

	float textXOffset = 0;
	float sceneWidth = (float)Minecraft::GetInstance()->width;

	// For 2-player splitscreen we need to move it around a bit
	switch( CXuiSceneBase::GetPlayerBasePosition(iPad) )
	{
	case CXuiSceneBase::e_BaseScene_Top:
	case CXuiSceneBase::e_BaseScene_Bottom:
		{
			vPosition.x -= Minecraft::GetInstance()->width / 4;
			textXOffset = SAFEZONE_HALF_WIDTH;
		}
		break;
	case CXuiSceneBase::e_BaseScene_Left:
		textXOffset = SAFEZONE_HALF_WIDTH;
	case CXuiSceneBase::e_BaseScene_Right:
		vPosition.y += Minecraft::GetInstance()->height / 4;
		sceneWidth = XUI_BASE_SCENE_WIDTH_HALF;
		break;
	case CXuiSceneBase::e_BaseScene_Top_Left:
		textXOffset = SAFEZONE_HALF_WIDTH;
		sceneWidth = XUI_BASE_SCENE_WIDTH_HALF;
		break;
	case CXuiSceneBase::e_BaseScene_Top_Right:
		textXOffset = 0;
		sceneWidth = XUI_BASE_SCENE_WIDTH_HALF;
		break;
	case CXuiSceneBase::e_BaseScene_Bottom_Left:
		textXOffset = SAFEZONE_HALF_WIDTH;
		sceneWidth = XUI_BASE_SCENE_WIDTH_HALF;
		break;
	case CXuiSceneBase::e_BaseScene_Bottom_Right:
		textXOffset = 0;
		sceneWidth = XUI_BASE_SCENE_WIDTH_HALF;
		break;
	default:
		textXOffset = SAFEZONE_HALF_WIDTH;
		break;
	}
	float fWidth, fHeight;
	XuiElementGetBounds( m_hCurrentChatScene[iPad], &fWidth, &fHeight );
	XuiElementSetBounds(m_hCurrentChatScene[iPad],sceneWidth, fHeight );

	CScene_Chat* chatScene;
	VOID *pObj;
	XuiObjectFromHandle( m_hCurrentChatScene[iPad], &pObj );
	chatScene = (CScene_Chat *)pObj;
	chatScene->OffsetTextPosition( textXOffset );

	XuiElementSetPosition( m_hCurrentChatScene[iPad], &vPosition );
}

void CConsoleMinecraftApp::ReloadHudScene(int iPad, bool bJoining /*= false*/, bool bForce /*= false*/)
{
	if(m_hFirstHudScene[iPad] == NULL || m_hCurrentHudScene[iPad] == NULL) return;

	// Re-create the hud scene so it is the correct size. It starts without any visible lines.
	BOOL hudSceneVisible = FALSE;
	if( m_hFirstHudScene[iPad] != m_hCurrentHudScene[iPad] )
	{
		hudSceneVisible = XuiElementIsShown(m_hCurrentHudScene[iPad]);
	}

	int iLocalPlayerC=GetLocalPlayerCount();
	// we need to decide whether to close the xui scene or just reposition it

	if( bForce ||
		(bJoining && iLocalPlayerC==2) ||
		(!bJoining && iLocalPlayerC==1) ||
		m_hFirstHudScene[iPad] == m_hCurrentHudScene[iPad])
	{
		if( m_hFirstHudScene[iPad] != m_hCurrentHudScene[iPad] ) XuiSceneNavigateBack(m_hCurrentHudScene[iPad], m_hFirstHudScene[iPad],iPad);
		m_hCurrentHudScene[iPad] = m_hFirstHudScene[iPad];
		app.NavigateToScene(iPad,eUIScene_HUD,NULL,true);

		XuiElementSetShow( m_hCurrentHudScene[iPad], hudSceneVisible);
	}
}

void CConsoleMinecraftApp::AdjustSplitscreenScene(HXUIOBJ hScene,D3DXVECTOR3 *pvOriginalPosition, int iPad, bool bAdjustXForSafeArea)
{
	D3DXVECTOR3 vec;

	Minecraft *pMinecraft = Minecraft::GetInstance();

	// store this for any change of screensection
	XuiElementGetPosition(hScene,pvOriginalPosition);
	vec=*pvOriginalPosition;

	if( pMinecraft->localplayers[iPad] != NULL )
	{
		switch( pMinecraft->localplayers[iPad]->m_iScreenSection)
		{
		case C4JRender::VIEWPORT_TYPE_SPLIT_TOP:
			// move the scene down
			vec.y+=fSafeZoneY;
			break;
		case C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM:
			break;
		case C4JRender::VIEWPORT_TYPE_SPLIT_LEFT:
			break;
		case C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT:
			// move the scene left
			if(bAdjustXForSafeArea) vec.x-=fSafeZoneX;
			break;
		case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT:
			// move the scene down
			vec.y+=fSafeZoneY;
			break;
		case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_RIGHT:
			// move the scene down
			vec.y+=fSafeZoneY;
			// move the scene left
			if(bAdjustXForSafeArea) vec.x-=fSafeZoneX;
			break;
		case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_LEFT:
			break;
		case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT:
			// move the scene left
			if(bAdjustXForSafeArea) vec.x-=fSafeZoneX;
			break;
		}
	}
	XuiElementSetPosition(hScene,&vec);
}

void CConsoleMinecraftApp::AdjustSplitscreenScene(HXUIOBJ hScene,D3DXVECTOR3 *pvOriginalPosition, int iPad, float fXAdjust)
{
	D3DXVECTOR3 vec;

	Minecraft *pMinecraft = Minecraft::GetInstance();

	// store this for any change of screensection

	XuiElementGetPosition(hScene,pvOriginalPosition);

	vec=*pvOriginalPosition;

	if( pMinecraft->localplayers[iPad] != NULL )
	{
		switch( pMinecraft->localplayers[iPad]->m_iScreenSection)
		{
		case C4JRender::VIEWPORT_TYPE_SPLIT_TOP:
			// move the scene down
			vec.y+=fSafeZoneY;
			// 4J-PB - don't adjust things in horizontal splitscreen
			//vec.x+=fXAdjust;
			break;
		case C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM:
			// 4J-PB - don't adjust things in horizontal splitscreen
			//vec.x+=fXAdjust;
			break;
		case C4JRender::VIEWPORT_TYPE_SPLIT_LEFT:
			vec.x+=fXAdjust;
			break;
		case C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT:
			// move the scene left
			vec.x-=fSafeZoneX-fXAdjust;
			break;
		case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT:
			// move the scene down
			vec.y+=fSafeZoneY;
			vec.x+=fXAdjust;
			break;
		case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_RIGHT:
			// move the scene down
			vec.y+=fSafeZoneY;
			// move the scene left
			vec.x-=fSafeZoneX-fXAdjust;
			break;
		case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_LEFT:
			vec.x+=fXAdjust;
			break;
		case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT:
			// move the scene left
			vec.x-=fSafeZoneX-fXAdjust;
			break;
		}
	}

	XuiElementSetPosition(hScene,&vec);
}

HRESULT CConsoleMinecraftApp::AdjustSplitscreenScene_PlayerChanged(HXUIOBJ hScene,D3DXVECTOR3 *pvOriginalPosition, int iPad, bool bJoining, bool bAdjustXForSafeArea)
{
	int iLocalPlayerC=GetLocalPlayerCount();
	Minecraft *pMinecraft=Minecraft::GetInstance();

	// we need to decide whether to close the xui scene or just reposition it

	if((bJoining && iLocalPlayerC==2) ||
		(!bJoining && iLocalPlayerC==1))
	{
		// Fix for #101078 - TCR #15: BAS Sign-in Changes: TU12: Signing out the secondary player's profile during the End Poem in local splitscreen game causes Host invincibility and corruption of End Portal.
		if(!IsSceneInStack(iPad,eUIScene_EndPoem))
		{
			// close the scene!
			ui.SetTooltips(iPad, -1);
			// kill the xui
			ui.CloseUIScenes(iPad);
		}

		// 4J Stu - Return S_FALSE to inidicate that the scene has been closed
		return S_FALSE;
	}
	else if ( pMinecraft->localplayers[iPad] != NULL )
	{
		// we need to reposition the scenes since the players will have moved around

		D3DXVECTOR3 vec=*pvOriginalPosition;

		switch( pMinecraft->localplayers[iPad]->m_iScreenSection)
		{
		case C4JRender::VIEWPORT_TYPE_SPLIT_TOP:
			// move the scene down
			vec.y+=fSafeZoneY;
			break;
		case C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM:
			break;
		case C4JRender::VIEWPORT_TYPE_SPLIT_LEFT:
			break;
		case C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT:
			// move the scene left
			if(bAdjustXForSafeArea) vec.x-=fSafeZoneX;
			break;
		case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT:
			// move the scene down
			vec.y+=fSafeZoneY;
			break;
		case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_RIGHT:
			// move the scene down
			vec.y+=fSafeZoneY;
			// move the scene left
			if(bAdjustXForSafeArea) vec.x-=fSafeZoneX;
			break;
		case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_LEFT:
			break;
		case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT:
			// move the scene left
			if(bAdjustXForSafeArea) vec.x-=fSafeZoneX;
			break;
		}
		XuiElementSetPosition(hScene,&vec);
	}

	return S_OK;
}

HRESULT CConsoleMinecraftApp::AdjustSplitscreenScene_PlayerChanged(HXUIOBJ hScene,D3DXVECTOR3 *pvOriginalPosition, int iPad, bool bJoining, float fXAdjust)
{
	int iLocalPlayerC=GetLocalPlayerCount();
	Minecraft *pMinecraft=Minecraft::GetInstance();

	// we need to decide whether to close the xui scene or just reposition it

	if((bJoining && iLocalPlayerC==2) ||
		(!bJoining && iLocalPlayerC==1))
	{
		// close the scene!
		ui.SetTooltips(iPad, -1);
		// kill the xui
		ui.CloseUIScenes(iPad);

		// 4J Stu - Return S_FALSE to inidicate that the scene has been closed
		return S_FALSE;
	}
	else if ( pMinecraft->localplayers[iPad] != NULL )
	{
		// we need to reposition the scenes since the players will have moved around

		D3DXVECTOR3 vec=*pvOriginalPosition;

		switch( pMinecraft->localplayers[iPad]->m_iScreenSection)
		{
		case C4JRender::VIEWPORT_TYPE_SPLIT_TOP:
			// move the scene down
			vec.y+=fSafeZoneY;
			vec.x+=fXAdjust;
			break;
		case C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM:
			vec.x+=fXAdjust;
			break;
		case C4JRender::VIEWPORT_TYPE_SPLIT_LEFT:
			vec.x+=fXAdjust;
			break;
		case C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT:
			// move the scene left
			vec.x-=fSafeZoneX-fXAdjust;
			break;
		case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT:
			// move the scene down
			vec.y+=fSafeZoneY;
			vec.x+=fXAdjust;
			break;
		case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_RIGHT:
			// move the scene down
			vec.y+=fSafeZoneY;
			// move the scene left
			vec.x-=fSafeZoneX-fXAdjust;
			break;
		case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_LEFT:
			vec.x+=fXAdjust;
			break;
		case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT:
			// move the scene left
			vec.x-=fSafeZoneX-fXAdjust;
			break;
		}
		XuiElementSetPosition(hScene,&vec);
	}

	return S_OK;
}

void CConsoleMinecraftApp::StoreLaunchData()
{
	LD_DEMO* pDemoData = NULL;


	DWORD dwStatus = XGetLaunchDataSize( &m_dwLaunchDataSize );
	if( dwStatus == ERROR_SUCCESS )
	{
		m_pLaunchData = new BYTE [ m_dwLaunchDataSize ];
		dwStatus = XGetLaunchData( m_pLaunchData, m_dwLaunchDataSize );
		pDemoData = (LD_DEMO*)( m_pLaunchData );
		if( pDemoData->dwID != LAUNCH_DATA_DEMO_ID )
		{
			// custom launch data; must be handled separately
		}
	}
}

void CConsoleMinecraftApp::ExitGame()
{
	if(m_pLaunchData!=NULL)
	{
		LD_DEMO* pDemoData = (LD_DEMO*)( m_pLaunchData );
		XSetLaunchData( pDemoData, m_dwLaunchDataSize );
		XLaunchNewImage( pDemoData->szLauncherXEX, 0 );
	}
	else
	{
		XLaunchNewImage( XLAUNCH_KEYWORD_DASH_ARCADE, 0 );
	}
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
void CConsoleMinecraftApp::FatalLoadError(void)
{
	XOVERLAPPED MessageBoxOverlap;

	memset(&MessageBoxOverlap, 0, sizeof(MessageBoxOverlap));

	//HANDLE messageBoxThread = CreateThread(NULL, 0, &CMinecraftApp::ShowFatalLoadMessageBoxThreadProc, &MessageBoxOverlap, 0, NULL);
	//
	//WaitForSingleObjectEx(messageBoxThread, // handle to object
	//	20000, // time-out interval
	//    FALSE // alertable option
	//);
	//XOVERLAPPED *MessageBoxOverlap = (XOVERLAPPED *)lpParameter;

	wchar_t *aStrings[3];
	LPCWSTR wszButtons[1];

	// IDS_FATAL_ERROR_TITLE
	// IDS_FATAL_ERROR_TEXT
	// IDS_EXIT_GAME

	switch (XGetLanguage())
	{
	case XC_LANGUAGE_GERMAN:
		aStrings[0] = L"Ladefehler";
		aStrings[1] = L"Minecraft: Xbox 360 Edition konnte nicht geladen werden und kann daher nicht fortgesetzt werden.";
		aStrings[2] = L"Spiel verlassen";
		break;
	case XC_LANGUAGE_SPANISH:
		aStrings[0] = L"Error al cargar";
		aStrings[1] = L"Minecraft: Xbox 360 Edition ha experimentado un error al cargar y no puede continuar.";
		aStrings[2] = L"Salir del juego";
		break;
	case XC_LANGUAGE_FRENCH:
		aStrings[0] = L"Échec du chargement";
		aStrings[1] = L"Le chargement de Minecraft: Xbox 360 Edition a échoué : impossible de continuer.";
		aStrings[2] = L"Quitter le jeu";
		break;
	case XC_LANGUAGE_ITALIAN:
		aStrings[0] = L"Errore caricamento";
		aStrings[1] = L"Caricamento \"Minecraft: Xbox 360 Edition\" non riuscito, impossibile continuare.";
		aStrings[2] = L"Esci dal gioco";
		break;
	case XC_LANGUAGE_JAPANESE:
		aStrings[0] = L"ロード エラー";
		aStrings[1] = L"Minecraft Xbox 360 版のロードに失敗しました。続行できません";
		aStrings[2] = L"ゲームを終了";
		break;
	case XC_LANGUAGE_KOREAN:
		aStrings[0] = L"불러오기 오류";
		aStrings[1] = L"Minecraft: Xbox 360 Edition을 불러오는 중에 오류가 발생하여 계속할 수 없습니다.";
		aStrings[2] = L"게임 나가기";
		break;
	case XC_LANGUAGE_PORTUGUESE:
		if(XGetLocale()==XC_LOCALE_BRAZIL)
		{
			// Brazilian Portuguese
			aStrings[0] = L"Erro de carregamento";
			aStrings[1] = L"\"Minecraft: Xbox 360 Edition\" falhou ao carregar e não é possível continuar.";
			aStrings[2] = L"Sair do Jogo";
		}
		else
		{
			aStrings[0] = L"Erro de Carregamento";
			aStrings[1] = L"Não foi possível carregar \"Minecraft: Edição Xbox 360\" e não é possível continuar.";
			aStrings[2] = L"Sair do Jogo";
		}
		break;
	case XC_LANGUAGE_TCHINESE:
		aStrings[0] = L"載入錯誤";
		aStrings[1] = L"無法載入 Minecraft: Xbox 360 Edition，因此無法繼續。";
		aStrings[2] = L"離開遊戲";
		break;
	default:
		aStrings[0] = L"Loading Error";
		aStrings[1] = L"\"Minecraft: Xbox 360 Edition\" has failed to load, and cannot continue.";
		aStrings[2] = L"Exit Game";
		break;
	}
	wszButtons[0] = aStrings[2];

	MESSAGEBOX_RESULT MessageResult;

	while (XShowMessageBoxUI(XUSER_INDEX_ANY, aStrings[0], aStrings[1], 1, wszButtons, 0, XMB_ALERTICON, &MessageResult, &MessageBoxOverlap) == ERROR_ACCESS_DENIED)
		Sleep(250);
	while (!XHasOverlappedIoCompleted(&MessageBoxOverlap))
	{
		RenderManager.SetClearColour(D3DCOLOR_RGBA(0,0,0,255));
		RenderManager.Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		RenderManager.Present();
		Sleep(250);
	}

	ExitGame();
}

WCHAR *CConsoleMinecraftApp::wchExt[MAX_EXTENSION_TYPES]=
{
	L"png",
	L"inf",
	L"dat",
};

int CConsoleMinecraftApp::GetLocalTMSFileIndex(WCHAR *wchTMSFile,bool bFilenameIncludesExtension,eFileExtensionType eEXT)
{
	WCHAR wchTemp[ 256 ];
	WCHAR wchInFile[ 256 ];

	if(bFilenameIncludesExtension)
	{
		swprintf(wchInFile, 256, L"%ls",wchTMSFile);
	}
	else
	{
		swprintf(wchInFile, 256, L"%ls.%ls",wchTMSFile,wchExt[eEXT]);
	}

	for(int i=0;i<TMS_COUNT;i++)
	{
		swprintf(wchTemp, 256, L"%ls.%ls",TMSFileA[i].wchFilename,wchExt[TMSFileA[i].eEXT]);
		if(wcscmp(wchTemp,wchInFile)==0)
		{
			return i;
		}
	}

	return -1;
}

int CConsoleMinecraftApp::RetrieveTMSFileListIndex(WCHAR *wchTMSFile)
{
	eFileExtensionType eEXT;
	WCHAR wTemp[256];

	int iIndex=0;
	// split the file name into name and extension
	while(wchTMSFile[iIndex]!=L'.')
	{
		wTemp[iIndex]=wchTMSFile[iIndex];
		iIndex++;
	}
	wTemp[iIndex]=0;

	for(int i=0;i<MAX_EXTENSION_TYPES;i++)
	{
		if(wcscmp(&wchTMSFile[iIndex+1],wchExt[i])==0)
		{
			eEXT=(eFileExtensionType)i;
		}
	}

	return GetLocalTMSFileIndex(wTemp,false,eEXT);
}

int CConsoleMinecraftApp::LoadLocalTMSFile(WCHAR *wchTMSFile, eFileExtensionType eExt)
{
	WCHAR wcTemp[256];

	wsprintfW(wcTemp,L"%s.%s",wchTMSFile,wchExt[eExt]);
	return LoadLocalTMSFile(wcTemp);
}

int CConsoleMinecraftApp::LoadLocalTMSFile(WCHAR *wchTMSFile)
{
	const DWORD LOCATOR_SIZE = 256; // Use this to allocate space to hold a ResourceLocator string
	WCHAR szResourceLocator[ LOCATOR_SIZE ];
	HRESULT hr;
	int iTMSFileIndex;

	// split the file name into file and extension and find it in out tms file list

	iTMSFileIndex=RetrieveTMSFileListIndex(wchTMSFile);

	if(iTMSFileIndex!=-1)
	{
		// can we find the tms file in our xzp?
		if(TMSFileA[iTMSFileIndex].pbData==NULL) // if we haven't already loaded it
		{
			swprintf(szResourceLocator, LOCATOR_SIZE, L"%ls#TMSFiles/%ls",m_wchTMSXZP, wchTMSFile);

			hr = XuiResourceLoadAllNoLoc(szResourceLocator, &TMSFileA[iTMSFileIndex].pbData, &TMSFileA[iTMSFileIndex].uiSize);
		}
	}

	return iTMSFileIndex;
}


void CConsoleMinecraftApp::FreeLocalTMSFiles(eTMSFileType eType)
{
	for(int i=0;i<TMS_COUNT;i++)
	{
		if((eType==eTMSFileType_All) ||(eType==TMSFileA[i].eTMSType))
		{
			if(TMSFileA[i].pbData!=NULL)
			{
				XuiFree(TMSFileA[i].pbData);
				TMSFileA[i].pbData=NULL;
				TMSFileA[i].uiSize=0;
			}
		}
	}
}

// 4J-PB - If any new DLC items are added to the TMSFiles, this array needs updated
TMS_FILE CConsoleMinecraftApp::TMSFileA[TMS_COUNT] =
{
	// skin packs
	{ L"SP1", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0 , 0},
	{ L"SP2", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0 , 0},
	{ L"SP3", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0 , 0},
	{ L"SP4", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0 , 0},
	{ L"SP5", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0 , 0},
	{ L"SP6", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0 , 0},
	{ L"SPF", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0 , 0},
	{ L"SPB", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0 , 0},
	{ L"SPC", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0 , 0},
	{ L"SPZ", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0 , 0},
	{ L"SPM", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0 , 0},
	{ L"SPI", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0 , 0},
	{ L"SPG", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0 , 0},

	 //themes
	{ L"ThSt", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0 , 0},
	{ L"ThIr", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0 , 0},
	{ L"ThGo", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0 , 0},
	{ L"ThDi", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0 , 0},
	{ L"ThAw", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0 , 0},

	 //gamerpics
	{ L"GPAn", eFileExtensionType_PNG , eTMSFileType_MinecraftStore, NULL, 0, 0},
	{ L"GPCo", eFileExtensionType_PNG , eTMSFileType_MinecraftStore, NULL, 0, 0},
	{ L"GPEn", eFileExtensionType_PNG , eTMSFileType_MinecraftStore, NULL, 0, 0},
	{ L"GPFo", eFileExtensionType_PNG , eTMSFileType_MinecraftStore, NULL, 0, 0},
	{ L"GPTo", eFileExtensionType_PNG , eTMSFileType_MinecraftStore, NULL, 0, 0},
	{ L"GPBA", eFileExtensionType_PNG , eTMSFileType_MinecraftStore, NULL, 0, 0},
	{ L"GPFa", eFileExtensionType_PNG , eTMSFileType_MinecraftStore, NULL, 0, 0},
	{ L"GPME", eFileExtensionType_PNG , eTMSFileType_MinecraftStore, NULL, 0, 0},
	{ L"GPMF", eFileExtensionType_PNG , eTMSFileType_MinecraftStore, NULL, 0, 0},
	{ L"GPMM", eFileExtensionType_PNG , eTMSFileType_MinecraftStore, NULL, 0, 0},
	{ L"GPSE", eFileExtensionType_PNG , eTMSFileType_MinecraftStore, NULL, 0, 0},

	{ L"GPOr", eFileExtensionType_PNG , eTMSFileType_MinecraftStore, NULL, 0, 0},
	{ L"GPMi", eFileExtensionType_PNG , eTMSFileType_MinecraftStore, NULL, 0, 0},
	{ L"GPMB", eFileExtensionType_PNG , eTMSFileType_MinecraftStore, NULL, 0, 0},
	{ L"GPBr", eFileExtensionType_PNG , eTMSFileType_MinecraftStore, NULL, 0, 0},

	{ L"GPM1", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0},
	{ L"GPM2", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0},
	{ L"GPM3", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0},

//avatar items
	{ L"AH_0001", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AH_0002", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AH_0003", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AH_0004", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AH_0005", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AH_0006", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AH_0007", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AH_0008", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AH_0009", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AH_0010", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AH_0011", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AH_0012", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AH_0013", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },

	{ L"AT_0001", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AT_0002", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AT_0003", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AT_0004", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AT_0005", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AT_0006", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AT_0007", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AT_0008", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AT_0009", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AT_0010", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AT_0011", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AT_0012", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AT_0013", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AT_0014", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AT_0015", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AT_0016", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AT_0017", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AT_0018", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AT_0019", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AT_0020", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AT_0021", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AT_0022", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AT_0023", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AT_0024", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AT_0025", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AT_0026", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },

	{ L"AP_0001", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AP_0002", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AP_0003", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AP_0004", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AP_0005", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AP_0006", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AP_0007", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AP_0009", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AP_0010", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AP_0011", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AP_0012", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AP_0013", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AP_0014", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AP_0015", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AP_0016", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AP_0017", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AP_0018", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },

	{ L"AP_0019", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AP_0020", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AP_0021", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AP_0022", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AP_0023", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AP_0024", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AP_0025", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AP_0026", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AP_0027", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AP_0028", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AP_0029", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AP_0030", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AP_0031", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AP_0032", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"AP_0033", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },

	{ L"AA_0001", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0 , 0 },

	// Mash-up Packs
	{ L"MPMA", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"MPMA", eFileExtensionType_DAT, eTMSFileType_TexturePack, NULL, 0, 1024 },
	{ L"MPSR", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"MPSR", eFileExtensionType_DAT, eTMSFileType_TexturePack, NULL, 0, 1025 },
	{ L"MPHA", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"MPHA", eFileExtensionType_DAT, eTMSFileType_TexturePack, NULL, 0, 1026 },

	// Texture Packs
	{ L"TP01", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"TP01", eFileExtensionType_DAT, eTMSFileType_TexturePack, NULL, 0, 2049 },
	{ L"TP02", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"TP02", eFileExtensionType_DAT, eTMSFileType_TexturePack, NULL, 0, 2053 },
	{ L"TP04", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"TP04", eFileExtensionType_DAT, eTMSFileType_TexturePack, NULL, 0, 2051 },
	{ L"TP05", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"TP05", eFileExtensionType_DAT, eTMSFileType_TexturePack, NULL, 0, 2054 },
	{ L"TP06", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"TP06", eFileExtensionType_DAT, eTMSFileType_TexturePack, NULL, 0, 2050 },
	{ L"TP07", eFileExtensionType_PNG, eTMSFileType_MinecraftStore, NULL, 0, 0 },
	{ L"TP07", eFileExtensionType_DAT, eTMSFileType_TexturePack, NULL, 0, 2055 },

};

#ifndef _CONTENT_PACKAGE
WCHAR CConsoleMinecraftApp::m_wchTMSXZP[] = L"file://game:/TMSFiles.xzp";
#else
WCHAR CConsoleMinecraftApp::m_wchTMSXZP[] = L"file://UPDATE:/res/TMS/TMSFiles.xzp";
#endif


void CConsoleMinecraftApp::GetFileFromTPD(eTPDFileType eType,PBYTE pbData,DWORD dwBytes,PBYTE *ppbData,DWORD *pdwBytes )
{
	PBYTE pbPos=pbData;
	// icon is the second thing in the file
	if(pbData && dwBytes>0)
	{
		//First is the language

		unsigned int uiDecompSize=*(unsigned int *)pbPos;
		unsigned int uiCompSize=((unsigned int *)pbPos)[1];

		if(eType==eTPDFileType_Loc)
		{
			*pdwBytes= uiDecompSize;
			*ppbData = new BYTE [uiDecompSize];

			Compression::getCompression()->Decompress(*ppbData,(UINT *)pdwBytes,&((unsigned int *)pbPos)[2],uiCompSize);
			return;
		}
		else
		{
			// skip over the data
			pbPos=(PBYTE)&((unsigned int *)pbPos)[2];
			pbPos+=uiCompSize;
		}

		uiDecompSize=*(unsigned int *)pbPos;
		uiCompSize=((unsigned int *)pbPos)[1];

		// second is the icon
		if(eType==eTPDFileType_Icon)
		{
			*pdwBytes= uiDecompSize;
			*ppbData = new BYTE [uiDecompSize];

			Compression::getCompression()->Decompress(*ppbData,(UINT *)pdwBytes,&((unsigned int *)pbPos)[2],uiCompSize);
			return;
		}
		else
		{
			// skip over the data
			pbPos=(PBYTE)&((unsigned int *)pbPos)[2];
			pbPos+=uiCompSize;
		}

		uiDecompSize=*(unsigned int *)pbPos;
		uiCompSize=((unsigned int *)pbPos)[1];

		// third is the comparison
		if(eType==eTPDFileType_Comparison)
		{
			*pdwBytes= uiDecompSize;
			*ppbData = new BYTE [uiDecompSize];

			Compression::getCompression()->Decompress(*ppbData,(UINT *)pdwBytes,&((unsigned int *)pbPos)[2],uiCompSize);
			return;
		}
	}

	return;
}


LPCWSTR	CConsoleMinecraftApp::GetString(int iID)
{
	return StringTable.Lookup(iID);
}

CXuiStringTable *CConsoleMinecraftApp::GetStringTable()
{
	return &StringTable;
}
