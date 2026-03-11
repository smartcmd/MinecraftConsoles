set(BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/PS3/")

set(_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_AUDIO
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/Audio/SoundEngine.cpp"
)
source_group("Common/Source Files/Audio" FILES ${_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_AUDIO})

set(_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_LEADERBOARDS
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/Leaderboards/SonyLeaderboardManager.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/Leaderboards/SonyLeaderboardManager.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/Leaderboards/base64.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/Leaderboards/base64.h"
)
source_group("Common/Source Files/Leaderboards" FILES ${_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_LEADERBOARDS})

set(_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_NETWORK_SONY
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/Network/Sony/NetworkPlayerSony.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/Network/Sony/NetworkPlayerSony.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/Network/Sony/PlatformNetworkManagerSony.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/Network/Sony/PlatformNetworkManagerSony.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/Network/Sony/SQRNetworkManager.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/Network/Sony/SQRNetworkManager.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/Network/Sony/SQRNetworkPlayer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/Network/Sony/SQRNetworkPlayer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/Network/Sony/SonyCommerce.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/Network/Sony/SonyHttp.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/Network/Sony/SonyHttp.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/Network/Sony/SonyRemoteStorage.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/Network/Sony/SonyRemoteStorage.h"
)
source_group("Common/Source Files/Network/Sony" FILES ${_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_NETWORK_SONY})

set(_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_UI
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
source_group("Common/Source Files/UI" FILES ${_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_UI})

set(_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_UI_COMPONENTS
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
source_group("Common/Source Files/UI/Components" FILES ${_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_UI_COMPONENTS})

set(_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_UI_CONTROLS
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
source_group("Common/Source Files/UI/Controls" FILES ${_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_UI_CONTROLS})

set(_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_UI_SCENES
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
source_group("Common/Source Files/UI/Scenes" FILES ${_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_UI_SCENES})

set(_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_UI_SCENES_DEBUG
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_DebugCreateSchematic.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_DebugCreateSchematic.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_DebugOptions.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_DebugOptions.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_DebugOverlay.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_DebugOverlay.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_DebugSetCamera.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_DebugSetCamera.h"
)
source_group("Common/Source Files/UI/Scenes/Debug" FILES ${_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_UI_SCENES_DEBUG})

set(_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_UI_SCENES_FRONTEND_MENU_SCREENS
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
source_group("Common/Source Files/UI/Scenes/Frontend Menu screens" FILES ${_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_UI_SCENES_FRONTEND_MENU_SCREENS})

set(_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_UI_SCENES_HELP__OPTIONS
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
source_group("Common/Source Files/UI/Scenes/Help & Options" FILES ${_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_UI_SCENES_HELP__OPTIONS})

set(_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_UI_SCENES_INGAME_MENU_SCREENS
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
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_PauseMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_PauseMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_SignEntryMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_SignEntryMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_TeleportMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene_TeleportMenu.h"
)
source_group("Common/Source Files/UI/Scenes/In-Game Menu Screens" FILES ${_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_UI_SCENES_INGAME_MENU_SCREENS})

set(_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_UI_SCENES_INGAME_MENU_SCREENS_CONTAINERS
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
source_group("Common/Source Files/UI/Scenes/In-Game Menu Screens/Containers" FILES ${_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_UI_SCENES_INGAME_MENU_SCREENS_CONTAINERS})

set(_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_ZLIB
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
source_group("Common/Source Files/zlib" FILES ${_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_ZLIB})

set(_MINECRAFT_CLIENT_PS3_DURANGO
  "${CMAKE_CURRENT_SOURCE_DIR}/Durango/Durango_UIController.h"
)
source_group("Durango" FILES ${_MINECRAFT_CLIENT_PS3_DURANGO})

set(_MINECRAFT_CLIENT_PS3_DURANGO_IGGY_GDRAW
  "${CMAKE_CURRENT_SOURCE_DIR}/Durango/Iggy/gdraw/gdraw_d3d10_shaders.inl"
  "${CMAKE_CURRENT_SOURCE_DIR}/Durango/Iggy/gdraw/gdraw_d3d11.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Durango/Iggy/gdraw/gdraw_d3d1x_shared.inl"
  "${CMAKE_CURRENT_SOURCE_DIR}/Durango/Iggy/gdraw/gdraw_shared.inl"
)
source_group("Durango/Iggy/gdraw" FILES ${_MINECRAFT_CLIENT_PS3_DURANGO_IGGY_GDRAW})

set(_MINECRAFT_CLIENT_PS3_DURANGO_IGGY_INCLUDE
  "${CMAKE_CURRENT_SOURCE_DIR}/Durango/Iggy/include/gdraw.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Durango/Iggy/include/iggy.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Durango/Iggy/include/iggyexpruntime.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Durango/Iggy/include/iggyperfmon.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Durango/Iggy/include/rrCore.h"
)
source_group("Durango/Iggy/include" FILES ${_MINECRAFT_CLIENT_PS3_DURANGO_IGGY_INCLUDE})

set(_MINECRAFT_CLIENT_PS3_PS3
  "${BASE_DIR}/PS3Extras/C4JSpursJob.cpp"
  "${BASE_DIR}/PS3Extras/C4JSpursJob.h"
  "${BASE_DIR}/PS3_App.cpp"
  "${BASE_DIR}/PS3_App.h"
  "${BASE_DIR}/PS3_UIController.cpp"
  "${BASE_DIR}/PS3_UIController.h"
  "${BASE_DIR}/Passphrase/ps3__np_conf.h"
)
source_group("PS3" FILES ${_MINECRAFT_CLIENT_PS3_PS3})

set(_MINECRAFT_CLIENT_PS3_PS3_4JLIBS
  "${BASE_DIR}/4JLibs/STO_TitleSmallStorage.cpp"
  "${BASE_DIR}/4JLibs/STO_TitleSmallStorage.h"
)
source_group("PS3/4JLibs" FILES ${_MINECRAFT_CLIENT_PS3_PS3_4JLIBS})

set(_MINECRAFT_CLIENT_PS3_PS3_4JLIBS_INC
  "${BASE_DIR}/4JLibs/inc/4J_Input.h"
  "${BASE_DIR}/4JLibs/inc/4J_Profile.h"
  "${BASE_DIR}/4JLibs/inc/4J_Render.h"
  "${BASE_DIR}/4JLibs/inc/4J_Storage.h"
)
source_group("PS3/4JLibs/inc" FILES ${_MINECRAFT_CLIENT_PS3_PS3_4JLIBS_INC})

set(_MINECRAFT_CLIENT_PS3_PS3_CHUNKREBUILD_SPU
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/BedTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/BookshelfTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/BrewingStandTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/Bush_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/ButtonTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/CactusTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/CakeTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/CauldronTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/ChestTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/ChunkRebuildData.cpp"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/ChunkRebuildData.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/CocoaTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/CropTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/DetectorRailTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/DiodeTile_SPU.cpp"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/DiodeTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/Direction_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/DirectionalTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/DirtTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/DispenserTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/DoorTile_SPU.cpp"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/DoorTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/EggTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/EnchantmentTableTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/EntityTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/Facing_SPU.cpp"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/Facing_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/FarmTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/FenceGateTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/FenceTile_SPU.cpp"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/FenceTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/FireTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/FurnaceTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/GlassTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/GrassTile_SPU.cpp"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/GrassTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/HalfSlabTile_SPU.cpp"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/HalfSlabTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/HalfTransparentTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/HugeMushroomTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/IceTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/Icon_SPU.cpp"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/Icon_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/LadderTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/LeafTile_SPU.cpp"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/LeafTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/LeverTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/LiquidTile_SPU.cpp"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/LiquidTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/Material_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/MelonTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/Mushroom_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/MycelTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/NetherStalkTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/PistonBaseTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/PistonExtensionTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/PistonMovingPiece_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/PortalTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/PressurePlateTile_SPU.cpp"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/PressurePlateTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/PumpkinTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/RailTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/RecordPlayerTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/RedStoneDustTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/RedlightTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/ReedTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/SandStoneTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/Sapling_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/SignTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/SmoothStoneBrickTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/StairTile_SPU.cpp"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/StairTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/StemTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/StoneMonsterTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/TallGrass_SPU.cpp"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/TallGrass_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/Tesselator_SPU.cpp"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/Tesselator_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/TheEndPortalFrameTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/TheEndPortal_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/ThinFenceTile_SPU.cpp"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/ThinFenceTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/TileRenderer_SPU.cpp"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/TileRenderer_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/Tile_SPU.cpp"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/Tile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/TntTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/TopSnowTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/TorchTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/TrapDoorTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/TreeTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/VineTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/WaterLilyTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/WebTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/WoodTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/WorkbenchTile_SPU.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/stdafx.h"
  "${BASE_DIR}/SPU_Tasks/ChunkUpdate/stubs_SPU.h"
)
source_group("PS3/ChunkRebuild_SPU" FILES ${_MINECRAFT_CLIENT_PS3_PS3_CHUNKREBUILD_SPU})

set(_MINECRAFT_CLIENT_PS3_PS3_COMPRESSEDTILE_SPU
  "${BASE_DIR}/SPU_Tasks/CompressedTile/CompressedTileStorage_SPU.cpp"
  "${BASE_DIR}/SPU_Tasks/CompressedTile/CompressedTileStorage_SPU.h"
)
source_group("PS3/CompressedTile_SPU" FILES ${_MINECRAFT_CLIENT_PS3_PS3_COMPRESSEDTILE_SPU})

set(_MINECRAFT_CLIENT_PS3_PS3_IGGY_GDRAW
  "${BASE_DIR}/Iggy/gdraw/gdraw_ps3gcm.cpp"
  "${BASE_DIR}/Iggy/gdraw/gdraw_ps3gcm.h"
  "${BASE_DIR}/Iggy/gdraw/gdraw_ps3gcm_shaders.inl"
  "${BASE_DIR}/Iggy/gdraw/gdraw_shared.inl"
)
source_group("PS3/Iggy/gdraw" FILES ${_MINECRAFT_CLIENT_PS3_PS3_IGGY_GDRAW})

set(_MINECRAFT_CLIENT_PS3_PS3_IGGY_INCLUDE
  "${BASE_DIR}/Iggy/include/gdraw.h"
  "${BASE_DIR}/Iggy/include/iggy.h"
  "${BASE_DIR}/Iggy/include/iggyexpruntime.h"
  "${BASE_DIR}/Iggy/include/iggyperfmon.h"
  "${BASE_DIR}/Iggy/include/iggyperfmon_ps3.h"
  "${BASE_DIR}/Iggy/include/rrCore.h"
)
source_group("PS3/Iggy/include" FILES ${_MINECRAFT_CLIENT_PS3_PS3_IGGY_INCLUDE})

set(_MINECRAFT_CLIENT_PS3_PS3_MILES_SOUND_SYSTEM_INCLUDE
  "${BASE_DIR}/Miles/include/mss.h"
  "${BASE_DIR}/Miles/include/rrCore.h"
)
source_group("PS3/Miles Sound System/include" FILES ${_MINECRAFT_CLIENT_PS3_PS3_MILES_SOUND_SYSTEM_INCLUDE})

set(_MINECRAFT_CLIENT_PS3_PS3_MILES_SOUND_SYSTEM_LIB
  "${BASE_DIR}/Miles/lib/audps3.a"
  "${BASE_DIR}/Miles/lib/fltps3.a"
  "${BASE_DIR}/Miles/lib/mssps3.a"
)
source_group("PS3/Miles Sound System/lib" FILES ${_MINECRAFT_CLIENT_PS3_PS3_MILES_SOUND_SYSTEM_LIB})

set(_MINECRAFT_CLIENT_PS3_PS3_MILES_SOUND_SYSTEM_LIB_SPU
  "${BASE_DIR}/Miles/lib/spu/binkaspu.a"
  "${BASE_DIR}/Miles/lib/spu/mssppu_raw.a"
  "${BASE_DIR}/Miles/lib/spu/mssppu_spurs.a"
  "${BASE_DIR}/Miles/lib/spu/mssppu_sputhreads.a"
  "${BASE_DIR}/Miles/lib/spu/mssspu.a"
  "${BASE_DIR}/Miles/lib/spu/mssspu_raw.a"
  "${BASE_DIR}/Miles/lib/spu/mssspu_spurs.a"
  "${BASE_DIR}/Miles/lib/spu/mssspu_sputhreads.a"
)
source_group("PS3/Miles Sound System/lib/spu" FILES ${_MINECRAFT_CLIENT_PS3_PS3_MILES_SOUND_SYSTEM_LIB_SPU})

set(_MINECRAFT_CLIENT_PS3_PS3_PS3EXTRAS
  "${BASE_DIR}/PS3Extras/C4JThread_SPU.cpp"
  "${BASE_DIR}/PS3Extras/C4JThread_SPU.h"
  "${BASE_DIR}/PS3Extras/EdgeZLib.cpp"
  "${BASE_DIR}/PS3Extras/EdgeZLib.h"
  "${BASE_DIR}/PS3Extras/PS3Maths.h"
  "${BASE_DIR}/PS3Extras/PS3Strings.cpp"
  "${BASE_DIR}/PS3Extras/PS3Strings.h"
  "${BASE_DIR}/PS3Extras/Ps3Stubs.cpp"
  "${BASE_DIR}/PS3Extras/Ps3Stubs.h"
  "${BASE_DIR}/PS3Extras/Ps3Types.h"
  "${BASE_DIR}/PS3Extras/ShutdownManager.cpp"
  "${BASE_DIR}/PS3Extras/ShutdownManager.h"
  "${BASE_DIR}/PS3Extras/TLSStorage.cpp"
  "${BASE_DIR}/PS3Extras/TLSStorage.h"
  "${BASE_DIR}/PS3Extras/winerror.h"
)
source_group("PS3/PS3Extras" FILES ${_MINECRAFT_CLIENT_PS3_PS3_PS3EXTRAS})

set(_MINECRAFT_CLIENT_PS3_PS3_SOURCE_FILES
  "${BASE_DIR}/Minecraft_Macros.h"
  "${BASE_DIR}/PS3_Minecraft.cpp"
  "${BASE_DIR}/PS3_PlayerUID.cpp"
  "${BASE_DIR}/PS3_PlayerUID.h"
)
source_group("PS3/Source Files" FILES ${_MINECRAFT_CLIENT_PS3_PS3_SOURCE_FILES})

set(_MINECRAFT_CLIENT_PS3_PS3_SOURCE_FILES_AUDIO
  "${BASE_DIR}/Audio/PS3_SoundEngine.cpp"
)
source_group("PS3/Source Files/Audio" FILES ${_MINECRAFT_CLIENT_PS3_PS3_SOURCE_FILES_AUDIO})

set(_MINECRAFT_CLIENT_PS3_PS3_SOURCE_FILES_LEADERBOARDS
  "${BASE_DIR}/Leaderboards/PS3LeaderboardManager.cpp"
  "${BASE_DIR}/Leaderboards/PS3LeaderboardManager.h"
)
source_group("PS3/Source Files/Leaderboards" FILES ${_MINECRAFT_CLIENT_PS3_PS3_SOURCE_FILES_LEADERBOARDS})

set(_MINECRAFT_CLIENT_PS3_PS3_SOURCE_FILES_NETWORK
  "${BASE_DIR}/Network/SQRNetworkManager_PS3.cpp"
  "${BASE_DIR}/Network/SQRNetworkManager_PS3.h"
  "${BASE_DIR}/Network/SonyCommerce_PS3.cpp"
  "${BASE_DIR}/Network/SonyCommerce_PS3.h"
  "${BASE_DIR}/Network/SonyHttp_PS3.cpp"
  "${BASE_DIR}/Network/SonyHttp_PS3.h"
  "${BASE_DIR}/Network/SonyRemoteStorage_PS3.cpp"
  "${BASE_DIR}/Network/SonyRemoteStorage_PS3.h"
  "${BASE_DIR}/Network/SonyVoiceChat.cpp"
  "${BASE_DIR}/Network/SonyVoiceChat.h"
)
source_group("PS3/Source Files/Network" FILES ${_MINECRAFT_CLIENT_PS3_PS3_SOURCE_FILES_NETWORK})

set(_MINECRAFT_CLIENT_PS3_PS3_SOURCE_FILES_SENTIENT
  "${BASE_DIR}/Sentient/DynamicConfigurations.h"
  "${BASE_DIR}/Sentient/MinecraftTelemetry.h"
  "${BASE_DIR}/Sentient/SentientManager.h"
  "${BASE_DIR}/Sentient/SentientStats.h"
  "${BASE_DIR}/Sentient/SentientTelemetryCommon.h"
  "${BASE_DIR}/Sentient/TelemetryEnum.h"
)
source_group("PS3/Source Files/Sentient" FILES ${_MINECRAFT_CLIENT_PS3_PS3_SOURCE_FILES_SENTIENT})

set(_MINECRAFT_CLIENT_PS3_PS3_SOURCE_FILES_SOCIAL
  "${BASE_DIR}/Social/SocialManager.h"
)
source_group("PS3/Source Files/Social" FILES ${_MINECRAFT_CLIENT_PS3_PS3_SOURCE_FILES_SOCIAL})

set(_MINECRAFT_CLIENT_PS3_SOURCE_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/Extrax64Stubs.cpp"
)
source_group("Source Files" FILES ${_MINECRAFT_CLIENT_PS3_SOURCE_FILES})

set(_MINECRAFT_CLIENT_PS3_WINDOWS
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/MinecraftWindows.rc"
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/Resource.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/targetver.h"
)
source_group("Windows" FILES ${_MINECRAFT_CLIENT_PS3_WINDOWS})

set(_MINECRAFT_CLIENT_PS3_WINDOWS64
  "${CMAKE_CURRENT_SOURCE_DIR}/Windows64/Resource.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Windows64/Windows64_App.h"
)
source_group("Windows64" FILES ${_MINECRAFT_CLIENT_PS3_WINDOWS64})

set(_MINECRAFT_CLIENT_PS3_WINDOWS64_4JLIBS_INC
  "${CMAKE_CURRENT_SOURCE_DIR}/Windows64/4JLibs/inc/4J_Input.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Windows64/4JLibs/inc/4J_Profile.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Windows64/4JLibs/inc/4J_Render.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Windows64/4JLibs/inc/4J_Storage.h"
)
source_group("Windows64/4JLibs/inc" FILES ${_MINECRAFT_CLIENT_PS3_WINDOWS64_4JLIBS_INC})

set(_MINECRAFT_CLIENT_PS3_WINDOWS64_GAMECONFIG
  "${CMAKE_CURRENT_SOURCE_DIR}/Windows64/GameConfig/Minecraft.gameconfig"
  "${CMAKE_CURRENT_SOURCE_DIR}/Windows64/GameConfig/Minecraft.spa"
  "${CMAKE_CURRENT_SOURCE_DIR}/Windows64/GameConfig/Minecraft.spa.h"
)
source_group("Windows64/GameConfig" FILES ${_MINECRAFT_CLIENT_PS3_WINDOWS64_GAMECONFIG})

set(_MINECRAFT_CLIENT_PS3_WINDOWS64_XML
  "${CMAKE_CURRENT_SOURCE_DIR}/Windows64/XML/ATGXmlParser.h"
)
source_group("Windows64/XML" FILES ${_MINECRAFT_CLIENT_PS3_WINDOWS64_XML})

set(_MINECRAFT_CLIENT_PS3_NET_MINECRAFT_CLIENT_MULTIPLAYER
  "${CMAKE_CURRENT_SOURCE_DIR}/ConnectScreen.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ConnectScreen.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/DisconnectedScreen.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/DisconnectedScreen.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/PlayerInfo.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ReceivingLevelScreen.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ReceivingLevelScreen.h"
)
source_group("net/minecraft/client/multiplayer" FILES ${_MINECRAFT_CLIENT_PS3_NET_MINECRAFT_CLIENT_MULTIPLAYER})

set(_MINECRAFT_CLIENT_PS3_NET_MINECRAFT_STATS
  "${CMAKE_CURRENT_SOURCE_DIR}/StatsCounter.h"
)
source_group("net/minecraft/stats" FILES ${_MINECRAFT_CLIENT_PS3_NET_MINECRAFT_STATS})

set(MINECRAFT_CLIENT_PS3
  ${_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_AUDIO}
  ${_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_LEADERBOARDS}
  ${_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_NETWORK_SONY}
  ${_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_UI}
  ${_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_UI_COMPONENTS}
  ${_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_UI_CONTROLS}
  ${_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_UI_SCENES}
  ${_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_UI_SCENES_DEBUG}
  ${_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_UI_SCENES_FRONTEND_MENU_SCREENS}
  ${_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_UI_SCENES_HELP__OPTIONS}
  ${_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_UI_SCENES_INGAME_MENU_SCREENS}
  ${_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_UI_SCENES_INGAME_MENU_SCREENS_CONTAINERS}
  ${_MINECRAFT_CLIENT_PS3_COMMON_SOURCE_FILES_ZLIB}
  ${_MINECRAFT_CLIENT_PS3_DURANGO}
  ${_MINECRAFT_CLIENT_PS3_DURANGO_IGGY_GDRAW}
  ${_MINECRAFT_CLIENT_PS3_DURANGO_IGGY_INCLUDE}
  ${_MINECRAFT_CLIENT_PS3_PS3}
  ${_MINECRAFT_CLIENT_PS3_PS3_4JLIBS}
  ${_MINECRAFT_CLIENT_PS3_PS3_4JLIBS_INC}
  ${_MINECRAFT_CLIENT_PS3_PS3_CHUNKREBUILD_SPU}
  ${_MINECRAFT_CLIENT_PS3_PS3_COMPRESSEDTILE_SPU}
  ${_MINECRAFT_CLIENT_PS3_PS3_IGGY_GDRAW}
  ${_MINECRAFT_CLIENT_PS3_PS3_IGGY_INCLUDE}
  ${_MINECRAFT_CLIENT_PS3_PS3_MILES_SOUND_SYSTEM_INCLUDE}
  ${_MINECRAFT_CLIENT_PS3_PS3_MILES_SOUND_SYSTEM_LIB}
  ${_MINECRAFT_CLIENT_PS3_PS3_MILES_SOUND_SYSTEM_LIB_SPU}
  ${_MINECRAFT_CLIENT_PS3_PS3_PS3EXTRAS}
  ${_MINECRAFT_CLIENT_PS3_PS3_SOURCE_FILES}
  ${_MINECRAFT_CLIENT_PS3_PS3_SOURCE_FILES_AUDIO}
  ${_MINECRAFT_CLIENT_PS3_PS3_SOURCE_FILES_LEADERBOARDS}
  ${_MINECRAFT_CLIENT_PS3_PS3_SOURCE_FILES_NETWORK}
  ${_MINECRAFT_CLIENT_PS3_PS3_SOURCE_FILES_SENTIENT}
  ${_MINECRAFT_CLIENT_PS3_PS3_SOURCE_FILES_SOCIAL}
  ${_MINECRAFT_CLIENT_PS3_SOURCE_FILES}
  ${_MINECRAFT_CLIENT_PS3_WINDOWS}
  ${_MINECRAFT_CLIENT_PS3_WINDOWS64}
  ${_MINECRAFT_CLIENT_PS3_WINDOWS64_4JLIBS_INC}
  ${_MINECRAFT_CLIENT_PS3_WINDOWS64_GAMECONFIG}
  ${_MINECRAFT_CLIENT_PS3_WINDOWS64_XML}
  ${_MINECRAFT_CLIENT_PS3_NET_MINECRAFT_CLIENT_MULTIPLAYER}
  ${_MINECRAFT_CLIENT_PS3_NET_MINECRAFT_STATS}
)
