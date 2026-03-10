set(BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Durango/")

set(_MINECRAFT_CLIENT_DURANGO_COMMON_SOURCE_FILES_AUDIO
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/Audio/SoundEngine.cpp"
)
source_group("Common/Source Files/Audio" FILES ${_MINECRAFT_CLIENT_DURANGO_COMMON_SOURCE_FILES_AUDIO})

set(_MINECRAFT_CLIENT_DURANGO_COMMON_SOURCE_FILES_UI
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UI.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIBitmapFont.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIBitmapFont.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIController.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIController.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIGroup.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIGroup.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UILayer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UILayer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UITTFFont.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UITTFFont.h"
)
source_group("Common/Source Files/UI" FILES ${_MINECRAFT_CLIENT_DURANGO_COMMON_SOURCE_FILES_UI})

set(_MINECRAFT_CLIENT_DURANGO_COMMON_SOURCE_FILES_UI_COMPONENTS
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIComponent_Chat.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIComponent_Chat.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIComponent_DebugUIConsole.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIComponent_DebugUIConsole.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIComponent_DebugUIMarketingGuide.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIComponent_DebugUIMarketingGuide.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIComponent_Logo.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIComponent_Logo.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIComponent_MenuBackground.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIComponent_MenuBackground.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIComponent_Panorama.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIComponent_Panorama.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIComponent_PressStartToPlay.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIComponent_PressStartToPlay.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIComponent_Tooltips.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIComponent_Tooltips.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIComponent_TutorialPopup.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIComponent_TutorialPopup.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_HUD.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_HUD.h"
)
source_group("Common/Source Files/UI/Components" FILES ${_MINECRAFT_CLIENT_DURANGO_COMMON_SOURCE_FILES_UI_COMPONENTS})

set(_MINECRAFT_CLIENT_DURANGO_COMMON_SOURCE_FILES_UI_CONTROLS
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_Base.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_Base.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_BeaconEffectButton.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_BeaconEffectButton.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_BitmapIcon.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_BitmapIcon.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_Button.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_Button.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_ButtonList.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_ButtonList.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_CheckBox.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_CheckBox.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_Cursor.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_Cursor.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_DLCList.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_DLCList.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_DynamicLabel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_DynamicLabel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_EnchantmentBook.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_EnchantmentBook.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_EnchantmentButton.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_EnchantmentButton.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_HTMLLabel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_HTMLLabel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_Label.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_Label.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_LeaderboardList.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_LeaderboardList.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_MinecraftHorse.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_MinecraftHorse.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_MinecraftPlayer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_MinecraftPlayer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_PlayerList.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_PlayerList.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_PlayerSkinPreview.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_PlayerSkinPreview.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_Progress.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_Progress.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_SaveList.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_SaveList.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_Slider.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_Slider.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_SlotList.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_SlotList.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_SpaceIndicatorBar.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_SpaceIndicatorBar.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_TextInput.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_TextInput.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_TexturePackList.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIControl_TexturePackList.h"
)
source_group("Common/Source Files/UI/Controls" FILES ${_MINECRAFT_CLIENT_DURANGO_COMMON_SOURCE_FILES_UI_CONTROLS})

set(_MINECRAFT_CLIENT_DURANGO_COMMON_SOURCE_FILES_UI_SCENES
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_ConnectingProgress.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_ConnectingProgress.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_FullscreenProgress.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_FullscreenProgress.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_Keyboard.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_Keyboard.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_MessageBox.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_MessageBox.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_QuadrantSignin.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_QuadrantSignin.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_Timer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_Timer.h"
)
source_group("Common/Source Files/UI/Scenes" FILES ${_MINECRAFT_CLIENT_DURANGO_COMMON_SOURCE_FILES_UI_SCENES})

set(_MINECRAFT_CLIENT_DURANGO_COMMON_SOURCE_FILES_UI_SCENES_DEBUG
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_DebugCreateSchematic.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_DebugCreateSchematic.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_DebugOptions.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_DebugOptions.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_DebugOverlay.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_DebugOverlay.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_DebugSetCamera.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_DebugSetCamera.h"
)
source_group("Common/Source Files/UI/Scenes/Debug" FILES ${_MINECRAFT_CLIENT_DURANGO_COMMON_SOURCE_FILES_UI_SCENES_DEBUG})

set(_MINECRAFT_CLIENT_DURANGO_COMMON_SOURCE_FILES_UI_SCENES_FRONTEND_MENU_SCREENS
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/IUIScene_StartGame.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_CreateWorldMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_CreateWorldMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_DLCMainMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_DLCMainMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_DLCOffersMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_DLCOffersMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_EULA.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_EULA.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_Intro.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_Intro.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_JoinMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_JoinMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_LaunchMoreOptionsMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_LaunchMoreOptionsMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_LeaderboardsMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_LeaderboardsMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_LoadMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_LoadMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_LoadOrJoinMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_LoadOrJoinMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_MainMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_MainMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_NewUpdateMessage.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_NewUpdateMessage.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_SaveMessage.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_SaveMessage.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_TrialExitUpsell.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_TrialExitUpsell.h"
)
source_group("Common/Source Files/UI/Scenes/Frontend Menu screens" FILES ${_MINECRAFT_CLIENT_DURANGO_COMMON_SOURCE_FILES_UI_SCENES_FRONTEND_MENU_SCREENS})

set(_MINECRAFT_CLIENT_DURANGO_COMMON_SOURCE_FILES_UI_SCENES_HELP__OPTIONS
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_ControlsMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_ControlsMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_Credits.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_Credits.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_HelpAndOptionsMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_HelpAndOptionsMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_HowToPlay.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_HowToPlay.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_HowToPlayMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_HowToPlayMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_LanguageSelector.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_LanguageSelector.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_ReinstallMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_ReinstallMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_SettingsAudioMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_SettingsAudioMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_SettingsControlMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_SettingsControlMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_SettingsGraphicsMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_SettingsGraphicsMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_SettingsMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_SettingsMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_SettingsOptionsMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_SettingsOptionsMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_SettingsUIMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_SettingsUIMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_SkinSelectMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_SkinSelectMenu.h"
)
source_group("Common/Source Files/UI/Scenes/Help & Options" FILES ${_MINECRAFT_CLIENT_DURANGO_COMMON_SOURCE_FILES_UI_SCENES_HELP__OPTIONS})

set(_MINECRAFT_CLIENT_DURANGO_COMMON_SOURCE_FILES_UI_SCENES_INGAME_MENU_SCREENS
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_CraftingMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_CraftingMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_DeathMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_DeathMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_EndPoem.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_EndPoem.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_InGameHostOptionsMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_InGameHostOptionsMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_InGameInfoMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_InGameInfoMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_InGamePlayerOptionsMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_InGamePlayerOptionsMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_InGameSaveManagementMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_InGameSaveManagementMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_PauseMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_PauseMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_SignEntryMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_SignEntryMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_TeleportMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_TeleportMenu.h"
)
source_group("Common/Source Files/UI/Scenes/In-Game Menu Screens" FILES ${_MINECRAFT_CLIENT_DURANGO_COMMON_SOURCE_FILES_UI_SCENES_INGAME_MENU_SCREENS})

set(_MINECRAFT_CLIENT_DURANGO_COMMON_SOURCE_FILES_UI_SCENES_INGAME_MENU_SCREENS_CONTAINERS
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_AbstractContainerMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_AbstractContainerMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_AnvilMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_AnvilMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_BeaconMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_BeaconMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_BrewingStandMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_BrewingStandMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_ContainerMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_ContainerMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_CreativeMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_CreativeMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_DispenserMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_DispenserMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_EnchantingMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_EnchantingMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_FireworksMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_FireworksMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_FurnaceMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_FurnaceMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_HopperMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_HopperMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_HorseInventoryMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_HorseInventoryMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_InventoryMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_InventoryMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_TradingMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_TradingMenu.h"
)
source_group("Common/Source Files/UI/Scenes/In-Game Menu Screens/Containers" FILES ${_MINECRAFT_CLIENT_DURANGO_COMMON_SOURCE_FILES_UI_SCENES_INGAME_MENU_SCREENS_CONTAINERS})

set(_MINECRAFT_CLIENT_DURANGO_COMMON_SOURCE_FILES_ZLIB
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/adler32.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/compress.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/crc32.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/crc32.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/deflate.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/deflate.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/gzclose.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/gzguts.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/gzlib.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/gzread.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/gzwrite.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/infback.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/inffast.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/inffast.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/inffixed.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/inflate.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/inflate.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/inftrees.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/inftrees.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/trees.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/trees.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/uncompr.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/zconf.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/zlib.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/zutil.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/zutil.h"
)
source_group("Common/Source Files/zlib" FILES ${_MINECRAFT_CLIENT_DURANGO_COMMON_SOURCE_FILES_ZLIB})

set(_MINECRAFT_CLIENT_DURANGO_DURANGO
  "${BASE_DIR}/Durango_App.cpp"
  "${BASE_DIR}/Durango_App.h"
  "${BASE_DIR}/Durango_UIController.cpp"
  "${BASE_DIR}/Durango_UIController.h"
  "${BASE_DIR}/Resource.h"
  "${BASE_DIR}/SmallLogo.png"
  "${BASE_DIR}/SplashScreen.png"
  "${BASE_DIR}/StoreLogo.png"
)
source_group("Durango" FILES ${_MINECRAFT_CLIENT_DURANGO_DURANGO})

set(_MINECRAFT_CLIENT_DURANGO_DURANGO_4JLIBS_INC
  "${BASE_DIR}/4JLibs/inc/4J_Input.h"
  "${BASE_DIR}/4JLibs/inc/4J_Profile.h"
  "${BASE_DIR}/4JLibs/inc/4J_Render.h"
  "${BASE_DIR}/4JLibs/inc/4J_Storage.h"
)
source_group("Durango/4JLibs/inc" FILES ${_MINECRAFT_CLIENT_DURANGO_DURANGO_4JLIBS_INC})

set(_MINECRAFT_CLIENT_DURANGO_DURANGO_DURANGOEXTRAS
  "${BASE_DIR}/DurangoExtras/DurangoStubs.cpp"
  "${BASE_DIR}/DurangoExtras/DurangoStubs.h"
)
source_group("Durango/DurangoExtras" FILES ${_MINECRAFT_CLIENT_DURANGO_DURANGO_DURANGOEXTRAS})

set(_MINECRAFT_CLIENT_DURANGO_DURANGO_IGGY_GDRAW
  "${BASE_DIR}/Iggy/gdraw/gdraw_d3d10_shaders.inl"
  "${BASE_DIR}/Iggy/gdraw/gdraw_d3d11.cpp"
  "${BASE_DIR}/Iggy/gdraw/gdraw_d3d11.h"
  "${BASE_DIR}/Iggy/gdraw/gdraw_d3d1x_shared.inl"
  "${BASE_DIR}/Iggy/gdraw/gdraw_shared.inl"
)
source_group("Durango/Iggy/gdraw" FILES ${_MINECRAFT_CLIENT_DURANGO_DURANGO_IGGY_GDRAW})

set(_MINECRAFT_CLIENT_DURANGO_DURANGO_IGGY_INCLUDE
  "${BASE_DIR}/Iggy/include/gdraw.h"
  "${BASE_DIR}/Iggy/include/iggy.h"
  "${BASE_DIR}/Iggy/include/iggyexpruntime.h"
  "${BASE_DIR}/Iggy/include/iggyperfmon.h"
  "${BASE_DIR}/Iggy/include/rrCore.h"
)
source_group("Durango/Iggy/include" FILES ${_MINECRAFT_CLIENT_DURANGO_DURANGO_IGGY_INCLUDE})

set(_MINECRAFT_CLIENT_DURANGO_DURANGO_MILES_SOUND_SYSTEM_INCLUDE
  "${BASE_DIR}/Miles/include/mss.h"
  "${BASE_DIR}/Miles/include/rrCore.h"
)
source_group("Durango/Miles Sound System/include" FILES ${_MINECRAFT_CLIENT_DURANGO_DURANGO_MILES_SOUND_SYSTEM_INCLUDE})

set(_MINECRAFT_CLIENT_DURANGO_DURANGO_NETWORK
  "${BASE_DIR}/Network/ChatIntegrationLayer.cpp"
  "${BASE_DIR}/Network/ChatIntegrationLayer.h"
  "${BASE_DIR}/Network/DQRNetworkManager.cpp"
  "${BASE_DIR}/Network/DQRNetworkManager.h"
  "${BASE_DIR}/Network/DQRNetworkManager_FriendSessions.cpp"
  "${BASE_DIR}/Network/DQRNetworkManager_Log.cpp"
  "${BASE_DIR}/Network/DQRNetworkManager_SendReceive.cpp"
  "${BASE_DIR}/Network/DQRNetworkManager_XRNSEvent.cpp"
  "${BASE_DIR}/Network/DQRNetworkPlayer.cpp"
  "${BASE_DIR}/Network/DQRNetworkPlayer.h"
  "${BASE_DIR}/Network/NetworkPlayerDurango.cpp"
  "${BASE_DIR}/Network/NetworkPlayerDurango.h"
  "${BASE_DIR}/Network/PartyController.cpp"
  "${BASE_DIR}/Network/PartyController.h"
  "${BASE_DIR}/Network/PlatformNetworkManagerDurango.cpp"
  "${BASE_DIR}/Network/PlatformNetworkManagerDurango.h"
  "${BASE_DIR}/Network/base64.cpp"
  "${BASE_DIR}/Network/base64.h"
)
source_group("Durango/Network" FILES ${_MINECRAFT_CLIENT_DURANGO_DURANGO_NETWORK})

set(_MINECRAFT_CLIENT_DURANGO_DURANGO_SOURCE_FILES
  "${BASE_DIR}/ApplicationView.cpp"
  "${BASE_DIR}/ApplicationView.h"
  "${BASE_DIR}/Durango_Minecraft.cpp"
  "${BASE_DIR}/Minecraft_Macros.h"
)
source_group("Durango/Source Files" FILES ${_MINECRAFT_CLIENT_DURANGO_DURANGO_SOURCE_FILES})

set(_MINECRAFT_CLIENT_DURANGO_DURANGO_SOURCE_FILES_ACHIEVEMENTS
  "${BASE_DIR}/Achievements/AchievementManager.cpp"
)
source_group("Durango/Source Files/Achievements" FILES ${_MINECRAFT_CLIENT_DURANGO_DURANGO_SOURCE_FILES_ACHIEVEMENTS})

set(_MINECRAFT_CLIENT_DURANGO_DURANGO_SOURCE_FILES_LEADERBOARDS
  "${BASE_DIR}/Leaderboards/DurangoLeaderboardManager.cpp"
  "${BASE_DIR}/Leaderboards/DurangoLeaderboardManager.h"
  "${BASE_DIR}/Leaderboards/DurangoStatsDebugger.cpp"
  "${BASE_DIR}/Leaderboards/DurangoStatsDebugger.h"
  "${BASE_DIR}/Leaderboards/GameProgress.cpp"
  "${BASE_DIR}/Leaderboards/GameProgress.h"
)
source_group("Durango/Source Files/Leaderboards" FILES ${_MINECRAFT_CLIENT_DURANGO_DURANGO_SOURCE_FILES_LEADERBOARDS})

set(_MINECRAFT_CLIENT_DURANGO_DURANGO_SOURCE_FILES_SENTIENT
  "${BASE_DIR}/Sentient/DurangoTelemetry.cpp"
  "${BASE_DIR}/Sentient/DurangoTelemetry.h"
  "${BASE_DIR}/Sentient/DynamicConfigurations.h"
  "${BASE_DIR}/Sentient/MinecraftTelemetry.h"
  "${BASE_DIR}/Sentient/SentientManager.h"
  "${BASE_DIR}/Sentient/SentientStats.h"
  "${BASE_DIR}/Sentient/SentientTelemetryCommon.h"
  "${BASE_DIR}/Sentient/TelemetryEnum.h"
)
source_group("Durango/Source Files/Sentient" FILES ${_MINECRAFT_CLIENT_DURANGO_DURANGO_SOURCE_FILES_SENTIENT})

set(_MINECRAFT_CLIENT_DURANGO_DURANGO_SOURCE_FILES_SOCIAL
  "${BASE_DIR}/Social/SocialManager.h"
)
source_group("Durango/Source Files/Social" FILES ${_MINECRAFT_CLIENT_DURANGO_DURANGO_SOURCE_FILES_SOCIAL})

set(_MINECRAFT_CLIENT_DURANGO_DURANGO_XML
  "${BASE_DIR}/XML/ATGXmlParser.cpp"
  "${BASE_DIR}/XML/ATGXmlParser.h"
  "${BASE_DIR}/XML/xmlFilesCallback.h"
)
source_group("Durango/XML" FILES ${_MINECRAFT_CLIENT_DURANGO_DURANGO_XML})

set(_MINECRAFT_CLIENT_DURANGO_PS3
  "${CMAKE_CURRENT_SOURCE_DIR}/PS3/Passphrase/ps3__np_conf.h"
)
source_group("PS3" FILES ${_MINECRAFT_CLIENT_DURANGO_PS3})

set(_MINECRAFT_CLIENT_DURANGO_PS3_IGGY_INCLUDE
  "${CMAKE_CURRENT_SOURCE_DIR}/PS3/Iggy/include/gdraw.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/PS3/Iggy/include/iggy.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/PS3/Iggy/include/iggyexpruntime.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/PS3/Iggy/include/iggyperfmon.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/PS3/Iggy/include/iggyperfmon_ps3.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/PS3/Iggy/include/rrCore.h"
)
source_group("PS3/Iggy/include" FILES ${_MINECRAFT_CLIENT_DURANGO_PS3_IGGY_INCLUDE})

set(_MINECRAFT_CLIENT_DURANGO_PS3_PS3EXTRAS
  "${CMAKE_CURRENT_SOURCE_DIR}/PS3/PS3Extras/ShutdownManager.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/PS3/PS3Extras/ShutdownManager.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/PS3/PS3Extras/winerror.h"
)
source_group("PS3/PS3Extras" FILES ${_MINECRAFT_CLIENT_DURANGO_PS3_PS3EXTRAS})

set(_MINECRAFT_CLIENT_DURANGO_SOURCE_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/Extrax64Stubs.cpp"
)
source_group("Source Files" FILES ${_MINECRAFT_CLIENT_DURANGO_SOURCE_FILES})

set(_MINECRAFT_CLIENT_DURANGO_XBOX_SENTIENTLIBS_INC
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/Sentient/Include/SenClientAvatar.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/Sentient/Include/SenClientBoxArt.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/Sentient/Include/SenClientConfig.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/Sentient/Include/SenClientCore.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/Sentient/Include/SenClientCulture.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/Sentient/Include/SenClientCultureBackCompat_SenBoxArt.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/Sentient/Include/SenClientCultureBackCompat_SenClientUGC.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/Sentient/Include/SenClientCultureBackCompat_SenCore.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/Sentient/Include/SenClientCultureBackCompat_SenNews.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/Sentient/Include/SenClientCultureBackCompat_SenSuperstars.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/Sentient/Include/SenClientDynamicConfig.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/Sentient/Include/SenClientFame.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/Sentient/Include/SenClientFile.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/Sentient/Include/SenClientHelp.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/Sentient/Include/SenClientMain.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/Sentient/Include/SenClientMarkers.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/Sentient/Include/SenClientNews.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/Sentient/Include/SenClientPackage.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/Sentient/Include/SenClientRawData.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/Sentient/Include/SenClientResource.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/Sentient/Include/SenClientStats.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/Sentient/Include/SenClientSuperstars.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/Sentient/Include/SenClientSys.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/Sentient/Include/SenClientTypes.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/Sentient/Include/SenClientUGC.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/Sentient/Include/SenClientUGCLeaderboards.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/Sentient/Include/SenClientUGCTypes.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/Sentient/Include/SenClientUser.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/Sentient/Include/SenClientUtil.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/Sentient/Include/SenClientXML.h"
)
source_group("Xbox/SentientLibs/inc" FILES ${_MINECRAFT_CLIENT_DURANGO_XBOX_SENTIENTLIBS_INC})

set(_MINECRAFT_CLIENT_DURANGO_XBOX_SOURCE_FILES_SENTIENT_DYNAMICCONF
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/Sentient/trialConfigv1.bin"
)
source_group("Xbox/Source Files/Sentient/DynamicConf" FILES ${_MINECRAFT_CLIENT_DURANGO_XBOX_SOURCE_FILES_SENTIENT_DYNAMICCONF})

set(_MINECRAFT_CLIENT_DURANGO_XBOX_RES
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/res/terrain_MipmapLevel2.bmp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/res/terrain_MipmapLevel3.bmp"
)
source_group("Xbox/res" FILES ${_MINECRAFT_CLIENT_DURANGO_XBOX_RES})

set(_MINECRAFT_CLIENT_DURANGO_XBOX_RES_AUDIO
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/res/audio/minecraft.xsb"
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/res/audio/resident.xwb"
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/res/audio/streamed.xwb"
)
source_group("Xbox/res/audio" FILES ${_MINECRAFT_CLIENT_DURANGO_XBOX_RES_AUDIO})

set(MINECRAFT_CLIENT_DURANGO
  ${_MINECRAFT_CLIENT_DURANGO_COMMON_SOURCE_FILES_AUDIO}
  ${_MINECRAFT_CLIENT_DURANGO_COMMON_SOURCE_FILES_UI}
  ${_MINECRAFT_CLIENT_DURANGO_COMMON_SOURCE_FILES_UI_COMPONENTS}
  ${_MINECRAFT_CLIENT_DURANGO_COMMON_SOURCE_FILES_UI_CONTROLS}
  ${_MINECRAFT_CLIENT_DURANGO_COMMON_SOURCE_FILES_UI_SCENES}
  ${_MINECRAFT_CLIENT_DURANGO_COMMON_SOURCE_FILES_UI_SCENES_DEBUG}
  ${_MINECRAFT_CLIENT_DURANGO_COMMON_SOURCE_FILES_UI_SCENES_FRONTEND_MENU_SCREENS}
  ${_MINECRAFT_CLIENT_DURANGO_COMMON_SOURCE_FILES_UI_SCENES_HELP__OPTIONS}
  ${_MINECRAFT_CLIENT_DURANGO_COMMON_SOURCE_FILES_UI_SCENES_INGAME_MENU_SCREENS}
  ${_MINECRAFT_CLIENT_DURANGO_COMMON_SOURCE_FILES_UI_SCENES_INGAME_MENU_SCREENS_CONTAINERS}
  ${_MINECRAFT_CLIENT_DURANGO_COMMON_SOURCE_FILES_ZLIB}
  ${_MINECRAFT_CLIENT_DURANGO_DURANGO}
  ${_MINECRAFT_CLIENT_DURANGO_DURANGO_4JLIBS_INC}
  ${_MINECRAFT_CLIENT_DURANGO_DURANGO_DURANGOEXTRAS}
  ${_MINECRAFT_CLIENT_DURANGO_DURANGO_IGGY_GDRAW}
  ${_MINECRAFT_CLIENT_DURANGO_DURANGO_IGGY_INCLUDE}
  ${_MINECRAFT_CLIENT_DURANGO_DURANGO_MILES_SOUND_SYSTEM_INCLUDE}
  ${_MINECRAFT_CLIENT_DURANGO_DURANGO_NETWORK}
  ${_MINECRAFT_CLIENT_DURANGO_DURANGO_SOURCE_FILES}
  ${_MINECRAFT_CLIENT_DURANGO_DURANGO_SOURCE_FILES_ACHIEVEMENTS}
  ${_MINECRAFT_CLIENT_DURANGO_DURANGO_SOURCE_FILES_LEADERBOARDS}
  ${_MINECRAFT_CLIENT_DURANGO_DURANGO_SOURCE_FILES_SENTIENT}
  ${_MINECRAFT_CLIENT_DURANGO_DURANGO_SOURCE_FILES_SOCIAL}
  ${_MINECRAFT_CLIENT_DURANGO_DURANGO_XML}
  ${_MINECRAFT_CLIENT_DURANGO_PS3}
  ${_MINECRAFT_CLIENT_DURANGO_PS3_IGGY_INCLUDE}
  ${_MINECRAFT_CLIENT_DURANGO_PS3_PS3EXTRAS}
  ${_MINECRAFT_CLIENT_DURANGO_SOURCE_FILES}
  ${_MINECRAFT_CLIENT_DURANGO_XBOX_SENTIENTLIBS_INC}
  ${_MINECRAFT_CLIENT_DURANGO_XBOX_SOURCE_FILES_SENTIENT_DYNAMICCONF}
  ${_MINECRAFT_CLIENT_DURANGO_XBOX_RES}
  ${_MINECRAFT_CLIENT_DURANGO_XBOX_RES_AUDIO}
)
