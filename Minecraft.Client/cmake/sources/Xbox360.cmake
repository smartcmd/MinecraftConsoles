set(BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/")

set(_MINECRAFT_CLIENT_XBOX360_DURANGO
  "${CMAKE_CURRENT_SOURCE_DIR}/Durango/Durango_UIController.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Durango/Resource.h"
)
source_group("Durango" FILES ${_MINECRAFT_CLIENT_XBOX360_DURANGO})

set(_MINECRAFT_CLIENT_XBOX360_DURANGO_IGGY_GDRAW
  "${CMAKE_CURRENT_SOURCE_DIR}/Durango/Iggy/gdraw/gdraw_d3d10_shaders.inl"
  "${CMAKE_CURRENT_SOURCE_DIR}/Durango/Iggy/gdraw/gdraw_d3d11.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Durango/Iggy/gdraw/gdraw_d3d1x_shared.inl"
  "${CMAKE_CURRENT_SOURCE_DIR}/Durango/Iggy/gdraw/gdraw_shared.inl"
)
source_group("Durango/Iggy/gdraw" FILES ${_MINECRAFT_CLIENT_XBOX360_DURANGO_IGGY_GDRAW})

set(_MINECRAFT_CLIENT_XBOX360_DURANGO_IGGY_INCLUDE
  "${CMAKE_CURRENT_SOURCE_DIR}/Durango/Iggy/include/gdraw.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Durango/Iggy/include/iggy.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Durango/Iggy/include/iggyexpruntime.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Durango/Iggy/include/iggyperfmon.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Durango/Iggy/include/rrCore.h"
)
source_group("Durango/Iggy/include" FILES ${_MINECRAFT_CLIENT_XBOX360_DURANGO_IGGY_INCLUDE})

set(_MINECRAFT_CLIENT_XBOX360_DURANGO_SOURCE_FILES_SENTIENT
  "${CMAKE_CURRENT_SOURCE_DIR}/Durango/Sentient/DynamicConfigurations.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Durango/Sentient/MinecraftTelemetry.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Durango/Sentient/SentientManager.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Durango/Sentient/SentientStats.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Durango/Sentient/SentientTelemetryCommon.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Durango/Sentient/TelemetryEnum.h"
)
source_group("Durango/Source Files/Sentient" FILES ${_MINECRAFT_CLIENT_XBOX360_DURANGO_SOURCE_FILES_SENTIENT})

set(_MINECRAFT_CLIENT_XBOX360_DURANGO_SOURCE_FILES_SOCIAL
  "${CMAKE_CURRENT_SOURCE_DIR}/Durango/Social/SocialManager.h"
)
source_group("Durango/Source Files/Social" FILES ${_MINECRAFT_CLIENT_XBOX360_DURANGO_SOURCE_FILES_SOCIAL})

set(_MINECRAFT_CLIENT_XBOX360_DURANGO_XML
  "${CMAKE_CURRENT_SOURCE_DIR}/Durango/XML/ATGXmlParser.h"
)
source_group("Durango/XML" FILES ${_MINECRAFT_CLIENT_XBOX360_DURANGO_XML})

set(_MINECRAFT_CLIENT_XBOX360_PS3
  "${CMAKE_CURRENT_SOURCE_DIR}/PS3/Passphrase/ps3__np_conf.h"
)
source_group("PS3" FILES ${_MINECRAFT_CLIENT_XBOX360_PS3})

set(_MINECRAFT_CLIENT_XBOX360_PS3_IGGY_INCLUDE
  "${CMAKE_CURRENT_SOURCE_DIR}/PS3/Iggy/include/gdraw.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/PS3/Iggy/include/iggy.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/PS3/Iggy/include/iggyexpruntime.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/PS3/Iggy/include/iggyperfmon.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/PS3/Iggy/include/iggyperfmon_ps3.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/PS3/Iggy/include/rrCore.h"
)
source_group("PS3/Iggy/include" FILES ${_MINECRAFT_CLIENT_XBOX360_PS3_IGGY_INCLUDE})

set(_MINECRAFT_CLIENT_XBOX360_PS3_PS3EXTRAS
  "${CMAKE_CURRENT_SOURCE_DIR}/PS3/PS3Extras/ShutdownManager.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/PS3/PS3Extras/ShutdownManager.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/PS3/PS3Extras/winerror.h"
)
source_group("PS3/PS3Extras" FILES ${_MINECRAFT_CLIENT_XBOX360_PS3_PS3EXTRAS})

set(_MINECRAFT_CLIENT_XBOX360_WINDOWS
  "${BASE_DIR}/Resource.h"
)
source_group("Windows" FILES ${_MINECRAFT_CLIENT_XBOX360_WINDOWS})

set(_MINECRAFT_CLIENT_XBOX360_XBOX_4JLIBS_INC
  "${BASE_DIR}/4JLibs/inc/4J_Input.h"
  "${BASE_DIR}/4JLibs/inc/4J_Profile.h"
  "${BASE_DIR}/4JLibs/inc/4J_Render.h"
  "${BASE_DIR}/4JLibs/inc/4J_Storage.h"
)
source_group("Xbox/4JLibs/inc" FILES ${_MINECRAFT_CLIENT_XBOX360_XBOX_4JLIBS_INC})

set(_MINECRAFT_CLIENT_XBOX360_XBOX_GAMECONFIG
  "${BASE_DIR}/GameConfig/Minecraft.gameconfig"
  "${BASE_DIR}/GameConfig/Minecraft.spa"
  "${BASE_DIR}/GameConfig/Minecraft.spa.h"
)
source_group("Xbox/GameConfig" FILES ${_MINECRAFT_CLIENT_XBOX360_XBOX_GAMECONFIG})

set(_MINECRAFT_CLIENT_XBOX360_XBOX_SENTIENTLIBS_INC
  "${BASE_DIR}/Sentient/Include/SenClientAvatar.h"
  "${BASE_DIR}/Sentient/Include/SenClientBoxArt.h"
  "${BASE_DIR}/Sentient/Include/SenClientConfig.h"
  "${BASE_DIR}/Sentient/Include/SenClientCore.h"
  "${BASE_DIR}/Sentient/Include/SenClientCulture.h"
  "${BASE_DIR}/Sentient/Include/SenClientCultureBackCompat_SenBoxArt.h"
  "${BASE_DIR}/Sentient/Include/SenClientCultureBackCompat_SenClientUGC.h"
  "${BASE_DIR}/Sentient/Include/SenClientCultureBackCompat_SenCore.h"
  "${BASE_DIR}/Sentient/Include/SenClientCultureBackCompat_SenNews.h"
  "${BASE_DIR}/Sentient/Include/SenClientCultureBackCompat_SenSuperstars.h"
  "${BASE_DIR}/Sentient/Include/SenClientDynamicConfig.h"
  "${BASE_DIR}/Sentient/Include/SenClientFame.h"
  "${BASE_DIR}/Sentient/Include/SenClientFile.h"
  "${BASE_DIR}/Sentient/Include/SenClientHelp.h"
  "${BASE_DIR}/Sentient/Include/SenClientMain.h"
  "${BASE_DIR}/Sentient/Include/SenClientMarkers.h"
  "${BASE_DIR}/Sentient/Include/SenClientNews.h"
  "${BASE_DIR}/Sentient/Include/SenClientPackage.h"
  "${BASE_DIR}/Sentient/Include/SenClientRawData.h"
  "${BASE_DIR}/Sentient/Include/SenClientResource.h"
  "${BASE_DIR}/Sentient/Include/SenClientStats.h"
  "${BASE_DIR}/Sentient/Include/SenClientSuperstars.h"
  "${BASE_DIR}/Sentient/Include/SenClientSys.h"
  "${BASE_DIR}/Sentient/Include/SenClientTypes.h"
  "${BASE_DIR}/Sentient/Include/SenClientUGC.h"
  "${BASE_DIR}/Sentient/Include/SenClientUGCLeaderboards.h"
  "${BASE_DIR}/Sentient/Include/SenClientUGCTypes.h"
  "${BASE_DIR}/Sentient/Include/SenClientUser.h"
  "${BASE_DIR}/Sentient/Include/SenClientUtil.h"
  "${BASE_DIR}/Sentient/Include/SenClientXML.h"
)
source_group("Xbox/SentientLibs/inc" FILES ${_MINECRAFT_CLIENT_XBOX360_XBOX_SENTIENTLIBS_INC})

set(_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES
  "${BASE_DIR}/Xbox_App.cpp"
  "${BASE_DIR}/Xbox_App.h"
  "${BASE_DIR}/Xbox_Minecraft.cpp"
  "${BASE_DIR}/Xbox_UIController.cpp"
  "${BASE_DIR}/Xbox_UIController.h"
)
source_group("Xbox/Source Files" FILES ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES})

set(_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_AUDIO
  "${BASE_DIR}/Audio/SoundEngine.cpp"
)
source_group("Xbox/Source Files/Audio" FILES ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_AUDIO})

set(_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_FONT
  "${BASE_DIR}/Font/XUI_Font.cpp"
  "${BASE_DIR}/Font/XUI_Font.h"
  "${BASE_DIR}/Font/XUI_FontData.cpp"
  "${BASE_DIR}/Font/XUI_FontData.h"
  "${BASE_DIR}/Font/XUI_FontRenderer.cpp"
  "${BASE_DIR}/Font/XUI_FontRenderer.h"
)
source_group("Xbox/Source Files/Font" FILES ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_FONT})

set(_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_LEADERBOARDS
  "${BASE_DIR}/Leaderboards/XboxLeaderboardManager.cpp"
  "${BASE_DIR}/Leaderboards/XboxLeaderboardManager.h"
)
source_group("Xbox/Source Files/Leaderboards" FILES ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_LEADERBOARDS})

set(_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_NETWORK
  "${BASE_DIR}/Network/NetworkPlayerXbox.cpp"
  "${BASE_DIR}/Network/NetworkPlayerXbox.h"
  "${BASE_DIR}/Network/PlatformNetworkManagerXbox.cpp"
  "${BASE_DIR}/Network/PlatformNetworkManagerXbox.h"
)
source_group("Xbox/Source Files/Network" FILES ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_NETWORK})

set(_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_SENTIENT
  "${BASE_DIR}/Sentient/SentientManager.cpp"
  "${BASE_DIR}/Sentient/SentientManager.h"
)
source_group("Xbox/Source Files/Sentient" FILES ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_SENTIENT})

set(_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_SENTIENT_DYNAMICCONF
  "${BASE_DIR}/Sentient/DynamicConfigurations.cpp"
  "${BASE_DIR}/Sentient/DynamicConfigurations.h"
  "${BASE_DIR}/Sentient/trialConfigv1.bin"
)
source_group("Xbox/Source Files/Sentient/DynamicConf" FILES ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_SENTIENT_DYNAMICCONF})

set(_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_SENTIENT_TELEMETRY
  "${BASE_DIR}/Sentient/MinecraftTelemetry.h"
  "${BASE_DIR}/Sentient/MinecraftTelemetry.xml"
  "${BASE_DIR}/Sentient/SentientStats.cpp"
  "${BASE_DIR}/Sentient/SentientStats.h"
  "${BASE_DIR}/Sentient/SentientTelemetryCommon.h"
  "${BASE_DIR}/Sentient/TelemetryEnum.h"
)
source_group("Xbox/Source Files/Sentient/Telemetry" FILES ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_SENTIENT_TELEMETRY})

set(_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_SOCIAL
  "${BASE_DIR}/Social/SocialManager.cpp"
  "${BASE_DIR}/Social/SocialManager.h"
)
source_group("Xbox/Source Files/Social" FILES ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_SOCIAL})

set(_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_CustomMessages.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Reinstall.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Reinstall.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_TextEntry.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_TextEntry.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_XZP_Icons.h"
)
source_group("Xbox/Source Files/XUI" FILES ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI})

set(_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_BASE_SCENE
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_BasePlayer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_BasePlayer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Chat.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Chat.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_HUD.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_HUD.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Scene_Base.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Scene_Base.h"
)
source_group("Xbox/Source Files/XUI/Base Scene" FILES ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_BASE_SCENE})

set(_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_CONTAINERS
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Scene_AbstractContainer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Scene_AbstractContainer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Scene_Anvil.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Scene_Anvil.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Scene_Beacon.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Scene_Beacon.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Scene_BrewingStand.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Scene_BrewingStand.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Scene_Container.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Scene_Container.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Scene_CraftingPanel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Scene_CraftingPanel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Scene_Enchant.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Scene_Enchant.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Scene_Fireworks.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Scene_Fireworks.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Scene_Furnace.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Scene_Furnace.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Scene_Hopper.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Scene_Hopper.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Scene_HorseInventory.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Scene_HorseInventory.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Scene_Inventory.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Scene_Inventory.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Scene_Inventory_Creative.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Scene_Inventory_Creative.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Scene_Trading.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Scene_Trading.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Scene_Trap.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Scene_Trap.h"
)
source_group("Xbox/Source Files/XUI/Containers" FILES ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_CONTAINERS})

set(_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_CONTROLS
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Controls.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_4JEdit.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_4JEdit.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_4JIcon.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_4JIcon.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_4JList.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_4JList.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_BeaconButton.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_BeaconButton.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_BrewProgress.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_BrewProgress.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_BubblesProgress.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_BubblesProgress.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_BurnProgress.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_BurnProgress.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_CraftIngredientSlot.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_CraftIngredientSlot.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_EnchantButton.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_EnchantButton.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_EnchantmentBook.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_EnchantmentBook.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_EnchantmentButtonText.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_EnchantmentButtonText.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_FireProgress.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_FireProgress.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_LoadingProgress.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_LoadingProgress.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_MinecraftHorse.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_MinecraftHorse.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_MinecraftPlayer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_MinecraftPlayer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_MinecraftSkinPreview.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_MinecraftSkinPreview.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_MinecraftSlot.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_MinecraftSlot.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_MobEffect.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_MobEffect.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_PassThroughList.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_PassthroughList.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_ProgressCtrlBase.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_ProgressCtrlBase.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_SliderWrapper.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_SliderWrapper.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_SlotItem.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_SlotItemCtrlBase.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_SlotItemCtrlBase.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_SlotItemListItem.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_SlotList.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_SlotList.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_SplashPulser.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Ctrl_SplashPulser.h"
)
source_group("Xbox/Source Files/XUI/Controls" FILES ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_CONTROLS})

set(_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_MENU_SCREENS
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_ConnectingProgress.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_ConnectingProgress.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_DLCOffers.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_DLCOffers.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Death.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Death.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_FullscreenProgress.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_FullscreenProgress.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Helper.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_InGameHostOptions.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_InGameHostOptions.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_InGameInfo.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_InGameInfo.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_InGamePlayerOptions.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_InGamePlayerOptions.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Intro.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Intro.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_LoadSettings.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_LoadSettings.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_MainMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_MainMenu.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_MultiGameCreate.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_MultiGameCreate.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_MultiGameInfo.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_MultiGameInfo.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_MultiGameJoinLoad.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_MultiGameJoinLoad.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_MultiGameLaunchMoreOptions.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_MultiGameLaunchMoreOptions.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_NewUpdateMessage.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_NewUpdateMessage.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_PartnernetPassword.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_PartnernetPassword.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_SaveMessage.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_SaveMessage.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Scene_Win.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Scene_Win.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_SignEntry.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_SignEntry.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Teleport.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Teleport.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_TransferToXboxOne.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_TrialExitUpsell.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_TrialExitUpsell.h"
)
source_group("Xbox/Source Files/XUI/Menu screens" FILES ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_MENU_SCREENS})

set(_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_DEBUG
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Debug.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_DebugItemEditor.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_DebugItemEditor.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_DebugOverlay.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_DebugOverlay.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_DebugSchematicCreator.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_DebugSchematicCreator.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_DebugSetCamera.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_DebugSetCamera.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_DebugTips.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_DebugTips.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_debug.cpp"
)
source_group("Xbox/Source Files/XUI/Menu screens/Debug" FILES ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_DEBUG})

set(_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_HELP__OPTIONS
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_HelpAndOptions.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_HelpAndOptions.h"
)
source_group("Xbox/Source Files/XUI/Menu screens/Help & Options" FILES ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_HELP__OPTIONS})

set(_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_HELP__OPTIONS_CONTROLS
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_HelpControls.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_HelpControls.h"
)
source_group("Xbox/Source Files/XUI/Menu screens/Help & Options/Controls" FILES ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_HELP__OPTIONS_CONTROLS})

set(_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_HELP__OPTIONS_CREDITS
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_HelpCredits.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_HelpCredits.h"
)
source_group("Xbox/Source Files/XUI/Menu screens/Help & Options/Credits" FILES ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_HELP__OPTIONS_CREDITS})

set(_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_HELP__OPTIONS_HOW_TO_PLAY
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_HelpHowToPlay.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_HelpHowToPlay.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_HowToPlayMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_HowToPlayMenu.h"
)
source_group("Xbox/Source Files/XUI/Menu screens/Help & Options/How To Play" FILES ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_HELP__OPTIONS_HOW_TO_PLAY})

set(_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_HELP__OPTIONS_SETTINGS
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_SettingsAll.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_SettingsAll.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_SettingsAudio.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_SettingsAudio.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_SettingsControl.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_SettingsControl.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_SettingsGraphics.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_SettingsGraphics.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_SettingsOptions.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_SettingsOptions.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_SettingsUI.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_SettingsUI.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_SkinSelect.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_SkinSelect.h"
)
source_group("Xbox/Source Files/XUI/Menu screens/Help & Options/Settings" FILES ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_HELP__OPTIONS_SETTINGS})

set(_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_LEADERBOARDS
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Leaderboards.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_Leaderboards.h"
)
source_group("Xbox/Source Files/XUI/Menu screens/Leaderboards" FILES ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_LEADERBOARDS})

set(_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_PAUSE
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_PauseMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_PauseMenu.h"
)
source_group("Xbox/Source Files/XUI/Menu screens/Pause" FILES ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_PAUSE})

set(_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_SOCIAL
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_SocialPost.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_SocialPost.h"
)
source_group("Xbox/Source Files/XUI/Menu screens/Social" FILES ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_SOCIAL})

set(_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_TUTORIAL
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_TutorialPopup.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/XUI/XUI_TutorialPopup.h"
)
source_group("Xbox/Source Files/XUI/Menu screens/Tutorial" FILES ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_TUTORIAL})

set(_MINECRAFT_CLIENT_XBOX360_XBOX_XML
  "${BASE_DIR}/XML/ATGXmlParser.cpp"
  "${BASE_DIR}/XML/ATGXmlParser.h"
  "${BASE_DIR}/XML/xmlFilesCallback.h"
)
source_group("Xbox/XML" FILES ${_MINECRAFT_CLIENT_XBOX360_XBOX_XML})

set(_MINECRAFT_CLIENT_XBOX360_XBOX_RES_AUDIO
  "${BASE_DIR}/res/audio/minecraft.xsb"
  "${BASE_DIR}/res/audio/resident.xwb"
  "${BASE_DIR}/res/audio/streamed.xwb"
)
source_group("Xbox/res/audio" FILES ${_MINECRAFT_CLIENT_XBOX360_XBOX_RES_AUDIO})

set(_MINECRAFT_CLIENT_XBOX360_XBOX_XEXXML
  "${BASE_DIR}/xex-dev.xml"
  "${BASE_DIR}/xex.xml"
)
source_group("Xbox/xexxml" FILES ${_MINECRAFT_CLIENT_XBOX360_XBOX_XEXXML})

set(_MINECRAFT_CLIENT_XBOX360_NET_MINECRAFT_CLIENT_MULTIPLAYER
  "${CMAKE_CURRENT_SOURCE_DIR}/ConnectScreen.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ConnectScreen.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/DisconnectedScreen.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/DisconnectedScreen.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/PlayerInfo.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ReceivingLevelScreen.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ReceivingLevelScreen.h"
)
source_group("net/minecraft/client/multiplayer" FILES ${_MINECRAFT_CLIENT_XBOX360_NET_MINECRAFT_CLIENT_MULTIPLAYER})

set(_MINECRAFT_CLIENT_XBOX360_NET_MINECRAFT_STATS
  "${CMAKE_CURRENT_SOURCE_DIR}/StatsCounter.h"
)
source_group("net/minecraft/stats" FILES ${_MINECRAFT_CLIENT_XBOX360_NET_MINECRAFT_STATS})

set(MINECRAFT_CLIENT_XBOX360
  ${_MINECRAFT_CLIENT_XBOX360_DURANGO}
  ${_MINECRAFT_CLIENT_XBOX360_DURANGO_IGGY_GDRAW}
  ${_MINECRAFT_CLIENT_XBOX360_DURANGO_IGGY_INCLUDE}
  ${_MINECRAFT_CLIENT_XBOX360_DURANGO_SOURCE_FILES_SENTIENT}
  ${_MINECRAFT_CLIENT_XBOX360_DURANGO_SOURCE_FILES_SOCIAL}
  ${_MINECRAFT_CLIENT_XBOX360_DURANGO_XML}
  ${_MINECRAFT_CLIENT_XBOX360_PS3}
  ${_MINECRAFT_CLIENT_XBOX360_PS3_IGGY_INCLUDE}
  ${_MINECRAFT_CLIENT_XBOX360_PS3_PS3EXTRAS}
  ${_MINECRAFT_CLIENT_XBOX360_WINDOWS}
  ${_MINECRAFT_CLIENT_XBOX360_XBOX_4JLIBS_INC}
  ${_MINECRAFT_CLIENT_XBOX360_XBOX_GAMECONFIG}
  ${_MINECRAFT_CLIENT_XBOX360_XBOX_SENTIENTLIBS_INC}
  ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES}
  ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_AUDIO}
  ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_FONT}
  ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_LEADERBOARDS}
  ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_NETWORK}
  ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_SENTIENT}
  ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_SENTIENT_DYNAMICCONF}
  ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_SENTIENT_TELEMETRY}
  ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_SOCIAL}
  ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI}
  ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_BASE_SCENE}
  ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_CONTAINERS}
  ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_CONTROLS}
  ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_MENU_SCREENS}
  ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_DEBUG}
  ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_HELP__OPTIONS}
  ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_HELP__OPTIONS_CONTROLS}
  ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_HELP__OPTIONS_CREDITS}
  ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_HELP__OPTIONS_HOW_TO_PLAY}
  ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_HELP__OPTIONS_SETTINGS}
  ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_LEADERBOARDS}
  ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_PAUSE}
  ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_SOCIAL}
  ${_MINECRAFT_CLIENT_XBOX360_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_TUTORIAL}
  ${_MINECRAFT_CLIENT_XBOX360_XBOX_XML}
  ${_MINECRAFT_CLIENT_XBOX360_XBOX_RES_AUDIO}
  ${_MINECRAFT_CLIENT_XBOX360_XBOX_XEXXML}
  ${_MINECRAFT_CLIENT_XBOX360_NET_MINECRAFT_CLIENT_MULTIPLAYER}
  ${_MINECRAFT_CLIENT_XBOX360_NET_MINECRAFT_STATS}
)
