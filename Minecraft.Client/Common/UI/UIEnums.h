#pragma once

// Defines the fixed groups for UI (lower numbers ticked first, rendered last (ie on top))
enum EUIGroup
{
	eUIGroup_Fullscreen,
	eUIGroup_Player1,
#ifndef __PSVITA__
	eUIGroup_Player2,
	eUIGroup_Player3,
	eUIGroup_Player4,
#endif

	eUIGroup_COUNT,

	eUIGroup_PAD, // Special case to determine the group from the pad (default)
};

// Defines the layers in a UI group (lower numbers ticked first, rendered last (ie on top))
enum EUILayer
{
#ifndef _CONTENT_PACKAGE
	eUILayer_Debug,
#endif
	eUILayer_Tooltips,
	eUILayer_Error,
	eUILayer_Alert,
	eUILayer_Fullscreen, // Note: Fullscreen in this context doesn't necessarily mean fill the whole screen, but fill the whole viewport for this group. Enables processes that don't interefere with normal scene stack
	eUILayer_Popup,
	eUILayer_Scene,
	//eUILayer_Chat,
	eUILayer_HUD,

	eUILayer_COUNT,
};

// Defines the scenes and components that can be added to a layer
// If you add to the enums below, you need to add the scene name in the right place in CConsoleMinecraftApp::wchSceneA
enum EUIScene
{
	eUIScene_PartnernetPassword = 0,
	eUIScene_Intro,
	eUIScene_SaveMessage,
	eUIScene_MainMenu,
	eUIScene_FullscreenProgress,
	eUIScene_PauseMenu,
	eUIScene_Crafting2x2Menu,
	eUIScene_Crafting3x3Menu,
	eUIScene_FurnaceMenu,
	eUIScene_ContainerMenu,
	eUIScene_LargeContainerMenu,// for splitscreen
	eUIScene_InventoryMenu,
	eUIScene_DispenserMenu,
	eUIScene_DebugOptions,
	eUIScene_DebugTips,
	eUIScene_HelpAndOptionsMenu,
	eUIScene_HowToPlay,
	eUIScene_HowToPlayMenu,
	eUIScene_ControlsMenu,
	eUIScene_SettingsOptionsMenu,
	eUIScene_SettingsAudioMenu,
	eUIScene_SettingsControlMenu,
	eUIScene_SettingsGraphicsMenu,
	eUIScene_SettingsUIMenu,
	eUIScene_SettingsMenu,		
	eUIScene_LeaderboardsMenu,
	eUIScene_Credits,
	eUIScene_DeathMenu,
	eUIComponent_TutorialPopup,
	eUIScene_CreateWorldMenu,
	eUIScene_LoadOrJoinMenu,
	eUIScene_JoinMenu,
	eUIScene_SignEntryMenu,
	eUIScene_InGameInfoMenu,
	eUIScene_ConnectingProgress,
	eUIScene_DLCOffersMenu,
	eUIScene_SocialPost,
	eUIScene_TrialExitUpsell,
	eUIScene_LoadMenu,
	eUIComponent_Chat,
	eUIScene_ReinstallMenu,
	eUIScene_SkinSelectMenu,
	eUIScene_TextEntry,
	eUIScene_InGameHostOptionsMenu,
	eUIScene_InGamePlayerOptionsMenu,
	eUIScene_CreativeMenu,
	eUIScene_LaunchMoreOptionsMenu,
	eUIScene_DLCMainMenu,
	eUIScene_NewUpdateMessage,
	eUIScene_EnchantingMenu,
	eUIScene_BrewingStandMenu,
	eUIScene_EndPoem,
	eUIScene_HUD,
	eUIScene_TradingMenu,
	eUIScene_AnvilMenu,
	eUIScene_TeleportMenu,
	eUIScene_HopperMenu,
	eUIScene_BeaconMenu,
	eUIScene_HorseMenu,
	eUIScene_FireworksMenu,

#ifdef _XBOX
//	eUIScene_TransferToXboxOne,
#endif

	// ****************************************
	// ****************************************
	// **********  IMPORTANT ******************
	// ****************************************
	// ****************************************
	// When adding new scenes here, you must also update the switches in CConsoleMinecraftApp::NavigateToScene
	// There are quite a few so you need to check them all
	// Also update UILayer::updateFocusState

#ifndef _XBOX
	// Anything non-xbox should be added here. The ordering of scenes above is required for sentient reporting on xbox 360 to continue to be accurate
	eUIComponent_Panorama,
	eUIComponent_Logo,
	eUIComponent_DebugUIConsole,
	eUIComponent_DebugUIMarketingGuide,
	eUIComponent_Tooltips,
	eUIComponent_PressStartToPlay,
	eUIComponent_MenuBackground,
	eUIScene_Keyboard,
	eUIScene_QuadrantSignin,
	eUIScene_MessageBox,
	eUIScene_Timer,
	eUIScene_EULA,
	eUIScene_InGameSaveManagementMenu,
	eUIScene_LanguageSelector,
#endif // ndef _XBOX

#ifdef _DEBUG_MENUS_ENABLED
	eUIScene_DebugOverlay,
	eUIScene_DebugItemEditor,
#endif
#ifndef _CONTENT_PACKAGE
	eUIScene_DebugCreateSchematic,
	eUIScene_DebugSetCamera,
#endif

	eUIScene_COUNT,
};

// Used by the fullscreen progress scene to decide what to do when a thread finishes
enum ProgressionCompletionType
{
	e_ProgressCompletion_NoAction,
	e_ProgressCompletion_NavigateBack,
	e_ProgressCompletion_CloseUIScenes,
	e_ProgressCompletion_CloseAllPlayersUIScenes,
	e_ProgressCompletion_NavigateToHomeMenu,
	e_ProgressCompletion_AutosaveNavigateBack,
	e_ProgressCompletion_NavigateBackToScene,
};

enum EToolTipButton
{
	eToolTipButtonA = 0,
	eToolTipButtonB,
	eToolTipButtonX,
	eToolTipButtonY,
	eToolTipButtonLT,
	eToolTipButtonRT,
	eToolTipButtonLB,
	eToolTipButtonRB,
	eToolTipButtonLS,
	eToolTipButtonRS,
	eToolTipButtonBack,
	eToolTipNumButtons
};

enum EToolTipItem
{
	eToolTipNone = -1,
	eToolTipPickupPlace_OLD = 0,	// To support existing menus.
	eToolTipExit,
	eToolTipPickUpGeneric,
	eToolTipPickUpAll,
	eToolTipPickUpHalf,
	eToolTipPlaceGeneric,
	eToolTipPlaceOne,
	eToolTipPlaceAll,
	eToolTipDropGeneric,
	eToolTipDropOne,
	eToolTipDropAll,
	eToolTipSwap,
	eToolTipQuickMove,
	eToolTipQuickMoveIngredient,
	eToolTipQuickMoveFuel,
	eToolTipWhatIsThis,
	eToolTipEquip,
	eToolTipClearQuickSelect,
	eToolTipQuickMoveTool,
	eToolTipQuickMoveArmor,
	eToolTipQuickMoveWeapon,
	eToolTipDye,
	eToolTipRepair,
	eNumToolTips
};

enum EHowToPlayPage
{
	eHowToPlay_WhatsNew = 0,
	eHowToPlay_Basics,
	eHowToPlay_Multiplayer,
	eHowToPlay_HUD,
	eHowToPlay_Creative,
	eHowToPlay_Inventory,
	eHowToPlay_Chest,
	eHowToPlay_LargeChest,
	eHowToPlay_Enderchest,
	eHowToPlay_InventoryCrafting,
	eHowToPlay_CraftTable,
	eHowToPlay_Furnace,
	eHowToPlay_Dispenser,

	eHowToPlay_Brewing,
	eHowToPlay_Enchantment,
	eHowToPlay_Anvil,
	eHowToPlay_FarmingAnimals,
	eHowToPlay_Breeding,
	eHowToPlay_Trading,

	eHowToPlay_Horses,
	eHowToPlay_Beacons,
	eHowToPlay_Fireworks,
	eHowToPlay_Hoppers,
	eHowToPlay_Droppers,

	eHowToPlay_NetherPortal,
	eHowToPlay_TheEnd,
#ifdef _XBOX
	eHowToPlay_SocialMedia,
	eHowToPlay_BanList,
#endif
	eHowToPlay_HostOptions,
	eHowToPlay_NumPages
};

// Credits
enum ECreditTextTypes
{
	eExtraLargeText = 0,
	eLargeText,
	eMediumText,
	eSmallText,
	eNumTextTypes
};

enum EUIMessage
{
	eUIMessage_InventoryUpdated,

	eUIMessage_COUNT,
};

#define NO_TRANSLATED_STRING	( -1 )			// String ID used to indicate that we are using non localised string.

#define CONNECTING_PROGRESS_CHECK_TIME 500
