set(_SOURCES_ROOT
  "ClassDiagram.cd"
  "Orbis/GameConfig/Minecraft.spa"
  "ReadMe.txt"
)
source_group("" FILES ${_SOURCES_ROOT})

set(_SOURCES_COMMON
  "Common/App_Defines.h"
  "Common/App_enums.h"
  "Common/App_structs.h"
  "Common/Consoles_App.cpp"
  "Common/Consoles_App.h"
  "Common/PostProcesser.h"
  "Common/Potion_Macros.h"
)
source_group("Common" FILES ${_SOURCES_COMMON})

set(_SOURCES_COMMON_SOURCE_FILES
  "Common/ConsoleGameMode.cpp"
  "Common/ConsoleGameMode.h"
  "Common/Console_Awards_enum.h"
  "Common/Console_Debug_enum.h"
  "Common/Console_Utils.cpp"
)
source_group("Common/Source Files" FILES ${_SOURCES_COMMON_SOURCE_FILES})

set(_SOURCES_COMMON_SOURCE_FILES_AUDIO
  "Common/Audio/Consoles_SoundEngine.cpp"
  "Common/Audio/Consoles_SoundEngine.h"
  "Common/Audio/SoundEngine.cpp"
  "Common/Audio/SoundEngine.h"
  "Common/Audio/SoundNames.cpp"
  "Common/Audio/miniaudio.h"
  "Common/Audio/stb_vorbis.h"
)
source_group("Common/Source Files/Audio" FILES ${_SOURCES_COMMON_SOURCE_FILES_AUDIO})

set(_SOURCES_COMMON_SOURCE_FILES_BUILDVER
  "Common/BuildVer.h"
)
source_group("Common/Source Files/BuildVer" FILES ${_SOURCES_COMMON_SOURCE_FILES_BUILDVER})

set(_SOURCES_COMMON_SOURCE_FILES_COLOURS
  "Common/Colours/ColourTable.cpp"
  "Common/Colours/ColourTable.h"
)
source_group("Common/Source Files/Colours" FILES ${_SOURCES_COMMON_SOURCE_FILES_COLOURS})

set(_SOURCES_COMMON_SOURCE_FILES_DLC
  "Common/DLC/DLCAudioFile.cpp"
  "Common/DLC/DLCAudioFile.h"
  "Common/DLC/DLCCapeFile.cpp"
  "Common/DLC/DLCCapeFile.h"
  "Common/DLC/DLCColourTableFile.cpp"
  "Common/DLC/DLCColourTableFile.h"
  "Common/DLC/DLCFile.cpp"
  "Common/DLC/DLCFile.h"
  "Common/DLC/DLCGameRules.h"
  "Common/DLC/DLCGameRulesFile.cpp"
  "Common/DLC/DLCGameRulesFile.h"
  "Common/DLC/DLCGameRulesHeader.cpp"
  "Common/DLC/DLCGameRulesHeader.h"
  "Common/DLC/DLCLocalisationFile.cpp"
  "Common/DLC/DLCLocalisationFile.h"
  "Common/DLC/DLCManager.cpp"
  "Common/DLC/DLCManager.h"
  "Common/DLC/DLCPack.cpp"
  "Common/DLC/DLCPack.h"
  "Common/DLC/DLCSkinFile.cpp"
  "Common/DLC/DLCSkinFile.h"
  "Common/DLC/DLCTextureFile.cpp"
  "Common/DLC/DLCTextureFile.h"
  "Common/DLC/DLCUIDataFile.cpp"
  "Common/DLC/DLCUIDataFile.h"
)
source_group("Common/Source Files/DLC" FILES ${_SOURCES_COMMON_SOURCE_FILES_DLC})

set(_SOURCES_COMMON_SOURCE_FILES_FILESYSTEM
  "Common/Filesystem/Filesystem.cpp"
  "Common/Filesystem/Filesystem.h"
)
source_group("Common/Source Files/Filesystem" FILES ${_SOURCES_COMMON_SOURCE_FILES_FILESYSTEM})

set(_SOURCES_COMMON_SOURCE_FILES_GAMERULES
  "Common/GameRules/ConsoleGameRules.h"
  "Common/GameRules/ConsoleGameRulesConstants.h"
  "Common/GameRules/GameRuleManager.cpp"
  "Common/GameRules/GameRuleManager.h"
  "WstringLookup.cpp"
  "WstringLookup.h"
)
source_group("Common/Source Files/GameRules" FILES ${_SOURCES_COMMON_SOURCE_FILES_GAMERULES})

set(_SOURCES_COMMON_SOURCE_FILES_GAMERULES_LEVELGENERATION
  "Common/GameRules/ApplySchematicRuleDefinition.cpp"
  "Common/GameRules/ApplySchematicRuleDefinition.h"
  "Common/GameRules/BiomeOverride.cpp"
  "Common/GameRules/BiomeOverride.h"
  "Common/GameRules/ConsoleGenerateStructure.cpp"
  "Common/GameRules/ConsoleGenerateStructure.h"
  "Common/GameRules/ConsoleGenerateStructureAction.h"
  "Common/GameRules/ConsoleSchematicFile.cpp"
  "Common/GameRules/ConsoleSchematicFile.h"
  "Common/GameRules/LevelGenerationOptions.cpp"
  "Common/GameRules/LevelGenerationOptions.h"
  "Common/GameRules/LevelGenerators.cpp"
  "Common/GameRules/LevelGenerators.h"
  "Common/GameRules/StartFeature.cpp"
  "Common/GameRules/StartFeature.h"
)
source_group("Common/Source Files/GameRules/LevelGeneration" FILES ${_SOURCES_COMMON_SOURCE_FILES_GAMERULES_LEVELGENERATION})

set(_SOURCES_COMMON_SOURCE_FILES_GAMERULES_LEVELGENERATION_STRUCTUREACTIONS
  "Common/GameRules/XboxStructureActionGenerateBox.cpp"
  "Common/GameRules/XboxStructureActionGenerateBox.h"
  "Common/GameRules/XboxStructureActionPlaceBlock.cpp"
  "Common/GameRules/XboxStructureActionPlaceBlock.h"
  "Common/GameRules/XboxStructureActionPlaceContainer.cpp"
  "Common/GameRules/XboxStructureActionPlaceContainer.h"
  "Common/GameRules/XboxStructureActionPlaceSpawner.cpp"
  "Common/GameRules/XboxStructureActionPlaceSpawner.h"
)
source_group("Common/Source Files/GameRules/LevelGeneration/StructureActions" FILES ${_SOURCES_COMMON_SOURCE_FILES_GAMERULES_LEVELGENERATION_STRUCTUREACTIONS})

set(_SOURCES_COMMON_SOURCE_FILES_GAMERULES_LEVELRULES
  "Common/GameRules/LevelRules.cpp"
  "Common/GameRules/LevelRules.h"
)
source_group("Common/Source Files/GameRules/LevelRules" FILES ${_SOURCES_COMMON_SOURCE_FILES_GAMERULES_LEVELRULES})

set(_SOURCES_COMMON_SOURCE_FILES_GAMERULES_LEVELRULES_RULEDEFINITIONS
  "Common/GameRules/AddEnchantmentRuleDefinition.cpp"
  "Common/GameRules/AddEnchantmentRuleDefinition.h"
  "Common/GameRules/AddItemRuleDefinition.cpp"
  "Common/GameRules/AddItemRuleDefinition.h"
  "Common/GameRules/CollectItemRuleDefinition.cpp"
  "Common/GameRules/CollectItemRuleDefinition.h"
  "Common/GameRules/CompleteAllRuleDefinition.cpp"
  "Common/GameRules/CompleteAllRuleDefinition.h"
  "Common/GameRules/CompoundGameRuleDefinition.cpp"
  "Common/GameRules/CompoundGameRuleDefinition.h"
  "Common/GameRules/GameRuleDefinition.cpp"
  "Common/GameRules/GameRuleDefinition.h"
  "Common/GameRules/LevelRuleset.cpp"
  "Common/GameRules/LevelRuleset.h"
  "Common/GameRules/NamedAreaRuleDefinition.cpp"
  "Common/GameRules/NamedAreaRuleDefinition.h"
  "Common/GameRules/UpdatePlayerRuleDefinition.cpp"
  "Common/GameRules/UpdatePlayerRuleDefinition.h"
  "Common/GameRules/UseTileRuleDefinition.cpp"
  "Common/GameRules/UseTileRuleDefinition.h"
)
source_group("Common/Source Files/GameRules/LevelRules/RuleDefinitions" FILES ${_SOURCES_COMMON_SOURCE_FILES_GAMERULES_LEVELRULES_RULEDEFINITIONS})

set(_SOURCES_COMMON_SOURCE_FILES_GAMERULES_LEVELRULES_RULES
  "Common/GameRules/GameRule.cpp"
  "Common/GameRules/GameRule.h"
  "Common/GameRules/GameRulesInstance.h"
)
source_group("Common/Source Files/GameRules/LevelRules/Rules" FILES ${_SOURCES_COMMON_SOURCE_FILES_GAMERULES_LEVELRULES_RULES})

set(_SOURCES_COMMON_SOURCE_FILES_LEADERBOARDS
  "Common/Leaderboards/LeaderboardInterface.cpp"
  "Common/Leaderboards/LeaderboardInterface.h"
  "Common/Leaderboards/LeaderboardManager.cpp"
  "Common/Leaderboards/LeaderboardManager.h"
  "Common/Leaderboards/SonyLeaderboardManager.cpp"
  "Common/Leaderboards/SonyLeaderboardManager.h"
  "Common/Leaderboards/base64.cpp"
  "Common/Leaderboards/base64.h"
)
source_group("Common/Source Files/Leaderboards" FILES ${_SOURCES_COMMON_SOURCE_FILES_LEADERBOARDS})

set(_SOURCES_COMMON_SOURCE_FILES_LOCALISATION
  "StringTable.cpp"
  "StringTable.h"
)
source_group("Common/Source Files/Localisation" FILES ${_SOURCES_COMMON_SOURCE_FILES_LOCALISATION})

set(_SOURCES_COMMON_SOURCE_FILES_NETWORK
  "Common/Network/GameNetworkManager.cpp"
  "Common/Network/GameNetworkManager.h"
  "Common/Network/NetworkPlayerInterface.h"
  "Common/Network/PlatformNetworkManagerInterface.h"
  "Common/Network/PlatformNetworkManagerStub.cpp"
  "Common/Network/PlatformNetworkManagerStub.h"
  "Common/Network/SessionInfo.h"
  "Network Implementation Notes.txt"
)
source_group("Common/Source Files/Network" FILES ${_SOURCES_COMMON_SOURCE_FILES_NETWORK})

set(_SOURCES_COMMON_SOURCE_FILES_NETWORK_SONY
  "Common/Network/Sony/NetworkPlayerSony.cpp"
  "Common/Network/Sony/NetworkPlayerSony.h"
  "Common/Network/Sony/PlatformNetworkManagerSony.cpp"
  "Common/Network/Sony/PlatformNetworkManagerSony.h"
  "Common/Network/Sony/SQRNetworkManager.cpp"
  "Common/Network/Sony/SQRNetworkManager.h"
  "Common/Network/Sony/SQRNetworkPlayer.cpp"
  "Common/Network/Sony/SQRNetworkPlayer.h"
  "Common/Network/Sony/SonyCommerce.h"
  "Common/Network/Sony/SonyHttp.cpp"
  "Common/Network/Sony/SonyHttp.h"
  "Common/Network/Sony/SonyRemoteStorage.cpp"
  "Common/Network/Sony/SonyRemoteStorage.h"
)
source_group("Common/Source Files/Network/Sony" FILES ${_SOURCES_COMMON_SOURCE_FILES_NETWORK_SONY})

set(_SOURCES_COMMON_SOURCE_FILES_TELEMETRY
  "Common/Telemetry/TelemetryManager.cpp"
  "Common/Telemetry/TelemetryManager.h"
)
source_group("Common/Source Files/Telemetry" FILES ${_SOURCES_COMMON_SOURCE_FILES_TELEMETRY})

set(_SOURCES_COMMON_SOURCE_FILES_TRIAL
  "Common/Trial/TrialMode.cpp"
  "Common/Trial/TrialMode.h"
)
source_group("Common/Source Files/Trial" FILES ${_SOURCES_COMMON_SOURCE_FILES_TRIAL})

set(_SOURCES_COMMON_SOURCE_FILES_TUTORIAL
  "Common/Tutorial/FullTutorial.cpp"
  "Common/Tutorial/FullTutorial.h"
  "Common/Tutorial/FullTutorialMode.cpp"
  "Common/Tutorial/FullTutorialMode.h"
  "Common/Tutorial/Tutorial.cpp"
  "Common/Tutorial/Tutorial.h"
  "Common/Tutorial/TutorialEnum.h"
  "Common/Tutorial/TutorialMessage.cpp"
  "Common/Tutorial/TutorialMessage.h"
  "Common/Tutorial/TutorialMode.cpp"
  "Common/Tutorial/TutorialMode.h"
)
source_group("Common/Source Files/Tutorial" FILES ${_SOURCES_COMMON_SOURCE_FILES_TUTORIAL})

set(_SOURCES_COMMON_SOURCE_FILES_TUTORIAL_CONSTRAINTS
  "Common/Tutorial/AreaConstraint.cpp"
  "Common/Tutorial/AreaConstraint.h"
  "Common/Tutorial/ChangeStateConstraint.cpp"
  "Common/Tutorial/ChangeStateConstraint.h"
  "Common/Tutorial/InputConstraint.cpp"
  "Common/Tutorial/InputConstraint.h"
  "Common/Tutorial/TutorialConstraint.h"
  "Common/Tutorial/TutorialConstraints.h"
)
source_group("Common/Source Files/Tutorial/Constraints" FILES ${_SOURCES_COMMON_SOURCE_FILES_TUTORIAL_CONSTRAINTS})

set(_SOURCES_COMMON_SOURCE_FILES_TUTORIAL_HINTS
  "Common/Tutorial/AreaHint.cpp"
  "Common/Tutorial/AreaHint.h"
  "Common/Tutorial/DiggerItemHint.cpp"
  "Common/Tutorial/DiggerItemHint.h"
  "Common/Tutorial/LookAtEntityHint.cpp"
  "Common/Tutorial/LookAtEntityHint.h"
  "Common/Tutorial/LookAtTileHint.cpp"
  "Common/Tutorial/LookAtTileHint.h"
  "Common/Tutorial/TakeItemHint.cpp"
  "Common/Tutorial/TakeItemHint.h"
  "Common/Tutorial/TutorialHint.cpp"
  "Common/Tutorial/TutorialHint.h"
  "Common/Tutorial/TutorialHints.h"
)
source_group("Common/Source Files/Tutorial/Hints" FILES ${_SOURCES_COMMON_SOURCE_FILES_TUTORIAL_HINTS})

set(_SOURCES_COMMON_SOURCE_FILES_TUTORIAL_TASKS
  "Common/Tutorial/AreaTask.cpp"
  "Common/Tutorial/AreaTask.h"
  "Common/Tutorial/ChoiceTask.cpp"
  "Common/Tutorial/ChoiceTask.h"
  "Common/Tutorial/CompleteUsingItemTask.cpp"
  "Common/Tutorial/CompleteUsingItemTask.h"
  "Common/Tutorial/ControllerTask.cpp"
  "Common/Tutorial/ControllerTask.h"
  "Common/Tutorial/CraftTask.cpp"
  "Common/Tutorial/CraftTask.h"
  "Common/Tutorial/EffectChangedTask.cpp"
  "Common/Tutorial/EffectChangedTask.h"
  "Common/Tutorial/FullTutorialActiveTask.cpp"
  "Common/Tutorial/FullTutorialActiveTask.h"
  "Common/Tutorial/HorseChoiceTask.cpp"
  "Common/Tutorial/HorseChoiceTask.h"
  "Common/Tutorial/InfoTask.cpp"
  "Common/Tutorial/InfoTask.h"
  "Common/Tutorial/PickupTask.cpp"
  "Common/Tutorial/PickupTask.h"
  "Common/Tutorial/ProcedureCompoundTask.cpp"
  "Common/Tutorial/ProcedureCompoundTask.h"
  "Common/Tutorial/ProgressFlagTask.cpp"
  "Common/Tutorial/ProgressFlagTask.h"
  "Common/Tutorial/RideEntityTask.cpp"
  "Common/Tutorial/RideEntityTask.h"
  "Common/Tutorial/StatTask.cpp"
  "Common/Tutorial/StatTask.h"
  "Common/Tutorial/StateChangeTask.h"
  "Common/Tutorial/TutorialTask.cpp"
  "Common/Tutorial/TutorialTask.h"
  "Common/Tutorial/TutorialTasks.h"
  "Common/Tutorial/UseItemTask.cpp"
  "Common/Tutorial/UseItemTask.h"
  "Common/Tutorial/UseTileTask.cpp"
  "Common/Tutorial/UseTileTask.h"
  "Common/Tutorial/XuiCraftingTask.cpp"
  "Common/Tutorial/XuiCraftingTask.h"
)
source_group("Common/Source Files/Tutorial/Tasks" FILES ${_SOURCES_COMMON_SOURCE_FILES_TUTORIAL_TASKS})

set(_SOURCES_COMMON_SOURCE_FILES_UI
  "Common/UI/UI.h"
  "Common/UI/UIBitmapFont.cpp"
  "Common/UI/UIBitmapFont.h"
  "Common/UI/UIController.cpp"
  "Common/UI/UIController.h"
  "Common/UI/UIFontData.cpp"
  "Common/UI/UIFontData.h"
  "Common/UI/UIGroup.cpp"
  "Common/UI/UIGroup.h"
  "Common/UI/UILayer.cpp"
  "Common/UI/UILayer.h"
  "Common/UI/UIScene.cpp"
  "Common/UI/UIScene.h"
  "Common/UI/UIString.cpp"
  "Common/UI/UIString.h"
  "Common/UI/UITTFFont.cpp"
  "Common/UI/UITTFFont.h"
)
source_group("Common/Source Files/UI" FILES ${_SOURCES_COMMON_SOURCE_FILES_UI})

set(_SOURCES_COMMON_SOURCE_FILES_UI_ALL_PLATFORMS
  "ArchiveFile.cpp"
  "ArchiveFile.h"
  "Common/UI/IUIController.h"
  "Common/UI/IUIScene_AbstractContainerMenu.cpp"
  "Common/UI/IUIScene_AbstractContainerMenu.h"
  "Common/UI/IUIScene_AnvilMenu.cpp"
  "Common/UI/IUIScene_AnvilMenu.h"
  "Common/UI/IUIScene_BeaconMenu.cpp"
  "Common/UI/IUIScene_BeaconMenu.h"
  "Common/UI/IUIScene_BrewingMenu.cpp"
  "Common/UI/IUIScene_BrewingMenu.h"
  "Common/UI/IUIScene_CommandBlockMenu.cpp"
  "Common/UI/IUIScene_CommandBlockMenu.h"
  "Common/UI/IUIScene_ContainerMenu.cpp"
  "Common/UI/IUIScene_ContainerMenu.h"
  "Common/UI/IUIScene_CraftingMenu.cpp"
  "Common/UI/IUIScene_CraftingMenu.h"
  "Common/UI/IUIScene_CreativeMenu.cpp"
  "Common/UI/IUIScene_CreativeMenu.h"
  "Common/UI/IUIScene_DispenserMenu.cpp"
  "Common/UI/IUIScene_DispenserMenu.h"
  "Common/UI/IUIScene_EnchantingMenu.cpp"
  "Common/UI/IUIScene_EnchantingMenu.h"
  "Common/UI/IUIScene_FireworksMenu.cpp"
  "Common/UI/IUIScene_FireworksMenu.h"
  "Common/UI/IUIScene_FurnaceMenu.cpp"
  "Common/UI/IUIScene_FurnaceMenu.h"
  "Common/UI/IUIScene_HUD.cpp"
  "Common/UI/IUIScene_HUD.h"
  "Common/UI/IUIScene_HopperMenu.cpp"
  "Common/UI/IUIScene_HopperMenu.h"
  "Common/UI/IUIScene_HorseInventoryMenu.cpp"
  "Common/UI/IUIScene_HorseInventoryMenu.h"
  "Common/UI/IUIScene_InventoryMenu.cpp"
  "Common/UI/IUIScene_InventoryMenu.h"
  "Common/UI/IUIScene_PauseMenu.cpp"
  "Common/UI/IUIScene_PauseMenu.h"
  "Common/UI/IUIScene_TradingMenu.cpp"
  "Common/UI/IUIScene_TradingMenu.h"
  "Common/UI/UIEnums.h"
  "Common/UI/UIStructs.h"
)
source_group("Common/Source Files/UI/All Platforms" FILES ${_SOURCES_COMMON_SOURCE_FILES_UI_ALL_PLATFORMS})

set(_SOURCES_COMMON_SOURCE_FILES_UI_COMPONENTS
  "Common/UI/UIComponent_Chat.cpp"
  "Common/UI/UIComponent_Chat.h"
  "Common/UI/UIComponent_DebugUIConsole.cpp"
  "Common/UI/UIComponent_DebugUIConsole.h"
  "Common/UI/UIComponent_DebugUIMarketingGuide.cpp"
  "Common/UI/UIComponent_DebugUIMarketingGuide.h"
  "Common/UI/UIComponent_Logo.cpp"
  "Common/UI/UIComponent_Logo.h"
  "Common/UI/UIComponent_MenuBackground.cpp"
  "Common/UI/UIComponent_MenuBackground.h"
  "Common/UI/UIComponent_Panorama.cpp"
  "Common/UI/UIComponent_Panorama.h"
  "Common/UI/UIComponent_PressStartToPlay.cpp"
  "Common/UI/UIComponent_PressStartToPlay.h"
  "Common/UI/UIComponent_Tooltips.cpp"
  "Common/UI/UIComponent_Tooltips.h"
  "Common/UI/UIComponent_TutorialPopup.cpp"
  "Common/UI/UIComponent_TutorialPopup.h"
  "Common/UI/UIScene_HUD.cpp"
  "Common/UI/UIScene_HUD.h"
)
source_group("Common/Source Files/UI/Components" FILES ${_SOURCES_COMMON_SOURCE_FILES_UI_COMPONENTS})

set(_SOURCES_COMMON_SOURCE_FILES_UI_CONTROLS
  "Common/UI/UIControl.cpp"
  "Common/UI/UIControl.h"
  "Common/UI/UIControl_Base.cpp"
  "Common/UI/UIControl_Base.h"
  "Common/UI/UIControl_BeaconEffectButton.cpp"
  "Common/UI/UIControl_BeaconEffectButton.h"
  "Common/UI/UIControl_BitmapIcon.cpp"
  "Common/UI/UIControl_BitmapIcon.h"
  "Common/UI/UIControl_Button.cpp"
  "Common/UI/UIControl_Button.h"
  "Common/UI/UIControl_ButtonList.cpp"
  "Common/UI/UIControl_ButtonList.h"
  "Common/UI/UIControl_CheckBox.cpp"
  "Common/UI/UIControl_CheckBox.h"
  "Common/UI/UIControl_Cursor.cpp"
  "Common/UI/UIControl_Cursor.h"
  "Common/UI/UIControl_DLCList.cpp"
  "Common/UI/UIControl_DLCList.h"
  "Common/UI/UIControl_DynamicLabel.cpp"
  "Common/UI/UIControl_DynamicLabel.h"
  "Common/UI/UIControl_EnchantmentBook.cpp"
  "Common/UI/UIControl_EnchantmentBook.h"
  "Common/UI/UIControl_EnchantmentButton.cpp"
  "Common/UI/UIControl_EnchantmentButton.h"
  "Common/UI/UIControl_HTMLLabel.cpp"
  "Common/UI/UIControl_HTMLLabel.h"
  "Common/UI/UIControl_Label.cpp"
  "Common/UI/UIControl_Label.h"
  "Common/UI/UIControl_LeaderboardList.cpp"
  "Common/UI/UIControl_LeaderboardList.h"
  "Common/UI/UIControl_MinecraftHorse.cpp"
  "Common/UI/UIControl_MinecraftHorse.h"
  "Common/UI/UIControl_MinecraftPlayer.cpp"
  "Common/UI/UIControl_MinecraftPlayer.h"
  "Common/UI/UIControl_PlayerList.cpp"
  "Common/UI/UIControl_PlayerList.h"
  "Common/UI/UIControl_PlayerSkinPreview.cpp"
  "Common/UI/UIControl_PlayerSkinPreview.h"
  "Common/UI/UIControl_Progress.cpp"
  "Common/UI/UIControl_Progress.h"
  "Common/UI/UIControl_SaveList.cpp"
  "Common/UI/UIControl_SaveList.h"
  "Common/UI/UIControl_Slider.cpp"
  "Common/UI/UIControl_Slider.h"
  "Common/UI/UIControl_SlotList.cpp"
  "Common/UI/UIControl_SlotList.h"
  "Common/UI/UIControl_SpaceIndicatorBar.cpp"
  "Common/UI/UIControl_SpaceIndicatorBar.h"
  "Common/UI/UIControl_TextInput.cpp"
  "Common/UI/UIControl_TextInput.h"
  "Common/UI/UIControl_TexturePackList.cpp"
  "Common/UI/UIControl_TexturePackList.h"
  "Common/UI/UIControl_Touch.cpp"
  "Common/UI/UIControl_Touch.h"
)
source_group("Common/Source Files/UI/Controls" FILES ${_SOURCES_COMMON_SOURCE_FILES_UI_CONTROLS})

set(_SOURCES_COMMON_SOURCE_FILES_UI_SCENES
  "Common/UI/UIScene_ConnectingProgress.cpp"
  "Common/UI/UIScene_ConnectingProgress.h"
  "Common/UI/UIScene_FullscreenProgress.cpp"
  "Common/UI/UIScene_FullscreenProgress.h"
  "Common/UI/UIScene_Keyboard.cpp"
  "Common/UI/UIScene_Keyboard.h"
  "Common/UI/UIScene_MessageBox.cpp"
  "Common/UI/UIScene_MessageBox.h"
  "Common/UI/UIScene_QuadrantSignin.cpp"
  "Common/UI/UIScene_QuadrantSignin.h"
  "Common/UI/UIScene_Timer.cpp"
  "Common/UI/UIScene_Timer.h"
)
source_group("Common/Source Files/UI/Scenes" FILES ${_SOURCES_COMMON_SOURCE_FILES_UI_SCENES})

set(_SOURCES_COMMON_SOURCE_FILES_UI_SCENES_DEBUG
  "Common/UI/UIScene_DebugCreateSchematic.cpp"
  "Common/UI/UIScene_DebugCreateSchematic.h"
  "Common/UI/UIScene_DebugOptions.cpp"
  "Common/UI/UIScene_DebugOptions.h"
  "Common/UI/UIScene_DebugOverlay.cpp"
  "Common/UI/UIScene_DebugOverlay.h"
  "Common/UI/UIScene_DebugSetCamera.cpp"
  "Common/UI/UIScene_DebugSetCamera.h"
)
source_group("Common/Source Files/UI/Scenes/Debug" FILES ${_SOURCES_COMMON_SOURCE_FILES_UI_SCENES_DEBUG})

set(_SOURCES_COMMON_SOURCE_FILES_UI_SCENES_FRONTEND_MENU_SCREENS
  "Common/UI/IUIScene_StartGame.cpp"
  "Common/UI/IUIScene_StartGame.h"
  "Common/UI/UIScene_CreateWorldMenu.cpp"
  "Common/UI/UIScene_CreateWorldMenu.h"
  "Common/UI/UIScene_DLCMainMenu.cpp"
  "Common/UI/UIScene_DLCMainMenu.h"
  "Common/UI/UIScene_DLCOffersMenu.cpp"
  "Common/UI/UIScene_DLCOffersMenu.h"
  "Common/UI/UIScene_EULA.cpp"
  "Common/UI/UIScene_EULA.h"
  "Common/UI/UIScene_Intro.cpp"
  "Common/UI/UIScene_Intro.h"
  "Common/UI/UIScene_JoinMenu.cpp"
  "Common/UI/UIScene_JoinMenu.h"
  "Common/UI/UIScene_LaunchMoreOptionsMenu.cpp"
  "Common/UI/UIScene_LaunchMoreOptionsMenu.h"
  "Common/UI/UIScene_LeaderboardsMenu.cpp"
  "Common/UI/UIScene_LeaderboardsMenu.h"
  "Common/UI/UIScene_LoadMenu.cpp"
  "Common/UI/UIScene_LoadMenu.h"
  "Common/UI/UIScene_LoadOrJoinMenu.cpp"
  "Common/UI/UIScene_LoadOrJoinMenu.h"
  "Common/UI/UIScene_MainMenu.cpp"
  "Common/UI/UIScene_MainMenu.h"
  "Common/UI/UIScene_NewUpdateMessage.cpp"
  "Common/UI/UIScene_NewUpdateMessage.h"
  "Common/UI/UIScene_SaveMessage.cpp"
  "Common/UI/UIScene_SaveMessage.h"
  "Common/UI/UIScene_TrialExitUpsell.cpp"
  "Common/UI/UIScene_TrialExitUpsell.h"
)
source_group("Common/Source Files/UI/Scenes/Frontend Menu screens" FILES ${_SOURCES_COMMON_SOURCE_FILES_UI_SCENES_FRONTEND_MENU_SCREENS})

set(_SOURCES_COMMON_SOURCE_FILES_UI_SCENES_HELP__OPTIONS
  "Common/UI/UIScene_ControlsMenu.cpp"
  "Common/UI/UIScene_ControlsMenu.h"
  "Common/UI/UIScene_Credits.cpp"
  "Common/UI/UIScene_Credits.h"
  "Common/UI/UIScene_HelpAndOptionsMenu.cpp"
  "Common/UI/UIScene_HelpAndOptionsMenu.h"
  "Common/UI/UIScene_HowToPlay.cpp"
  "Common/UI/UIScene_HowToPlay.h"
  "Common/UI/UIScene_HowToPlayMenu.cpp"
  "Common/UI/UIScene_HowToPlayMenu.h"
  "Common/UI/UIScene_LanguageSelector.cpp"
  "Common/UI/UIScene_LanguageSelector.h"
  "Common/UI/UIScene_ReinstallMenu.cpp"
  "Common/UI/UIScene_ReinstallMenu.h"
  "Common/UI/UIScene_SettingsAudioMenu.cpp"
  "Common/UI/UIScene_SettingsAudioMenu.h"
  "Common/UI/UIScene_SettingsControlMenu.cpp"
  "Common/UI/UIScene_SettingsControlMenu.h"
  "Common/UI/UIScene_SettingsGraphicsMenu.cpp"
  "Common/UI/UIScene_SettingsGraphicsMenu.h"
  "Common/UI/UIScene_SettingsMenu.cpp"
  "Common/UI/UIScene_SettingsMenu.h"
  "Common/UI/UIScene_SettingsOptionsMenu.cpp"
  "Common/UI/UIScene_SettingsOptionsMenu.h"
  "Common/UI/UIScene_SettingsUIMenu.cpp"
  "Common/UI/UIScene_SettingsUIMenu.h"
  "Common/UI/UIScene_SkinSelectMenu.cpp"
  "Common/UI/UIScene_SkinSelectMenu.h"
)
source_group("Common/Source Files/UI/Scenes/Help & Options" FILES ${_SOURCES_COMMON_SOURCE_FILES_UI_SCENES_HELP__OPTIONS})

set(_SOURCES_COMMON_SOURCE_FILES_UI_SCENES_INGAME_MENU_SCREENS
  "Common/UI/UIScene_CraftingMenu.cpp"
  "Common/UI/UIScene_CraftingMenu.h"
  "Common/UI/UIScene_DeathMenu.cpp"
  "Common/UI/UIScene_DeathMenu.h"
  "Common/UI/UIScene_EndPoem.cpp"
  "Common/UI/UIScene_EndPoem.h"
  "Common/UI/UIScene_InGameHostOptionsMenu.cpp"
  "Common/UI/UIScene_InGameHostOptionsMenu.h"
  "Common/UI/UIScene_InGameInfoMenu.cpp"
  "Common/UI/UIScene_InGameInfoMenu.h"
  "Common/UI/UIScene_InGamePlayerOptionsMenu.cpp"
  "Common/UI/UIScene_InGamePlayerOptionsMenu.h"
  "Common/UI/UIScene_InGameSaveManagementMenu.cpp"
  "Common/UI/UIScene_InGameSaveManagementMenu.h"
  "Common/UI/UIScene_PauseMenu.cpp"
  "Common/UI/UIScene_PauseMenu.h"
  "Common/UI/UIScene_SignEntryMenu.cpp"
  "Common/UI/UIScene_SignEntryMenu.h"
  "Common/UI/UIScene_TeleportMenu.cpp"
  "Common/UI/UIScene_TeleportMenu.h"
)
source_group("Common/Source Files/UI/Scenes/In-Game Menu Screens" FILES ${_SOURCES_COMMON_SOURCE_FILES_UI_SCENES_INGAME_MENU_SCREENS})

set(_SOURCES_COMMON_SOURCE_FILES_UI_SCENES_INGAME_MENU_SCREENS_CONTAINERS
  "Common/UI/UIScene_AbstractContainerMenu.cpp"
  "Common/UI/UIScene_AbstractContainerMenu.h"
  "Common/UI/UIScene_AnvilMenu.cpp"
  "Common/UI/UIScene_AnvilMenu.h"
  "Common/UI/UIScene_BeaconMenu.cpp"
  "Common/UI/UIScene_BeaconMenu.h"
  "Common/UI/UIScene_BrewingStandMenu.cpp"
  "Common/UI/UIScene_BrewingStandMenu.h"
  "Common/UI/UIScene_ContainerMenu.cpp"
  "Common/UI/UIScene_ContainerMenu.h"
  "Common/UI/UIScene_CreativeMenu.cpp"
  "Common/UI/UIScene_CreativeMenu.h"
  "Common/UI/UIScene_DispenserMenu.cpp"
  "Common/UI/UIScene_DispenserMenu.h"
  "Common/UI/UIScene_EnchantingMenu.cpp"
  "Common/UI/UIScene_EnchantingMenu.h"
  "Common/UI/UIScene_FireworksMenu.cpp"
  "Common/UI/UIScene_FireworksMenu.h"
  "Common/UI/UIScene_FurnaceMenu.cpp"
  "Common/UI/UIScene_FurnaceMenu.h"
  "Common/UI/UIScene_HopperMenu.cpp"
  "Common/UI/UIScene_HopperMenu.h"
  "Common/UI/UIScene_HorseInventoryMenu.cpp"
  "Common/UI/UIScene_HorseInventoryMenu.h"
  "Common/UI/UIScene_InventoryMenu.cpp"
  "Common/UI/UIScene_InventoryMenu.h"
  "Common/UI/UIScene_TradingMenu.cpp"
  "Common/UI/UIScene_TradingMenu.h"
)
source_group("Common/Source Files/UI/Scenes/In-Game Menu Screens/Containers" FILES ${_SOURCES_COMMON_SOURCE_FILES_UI_SCENES_INGAME_MENU_SCREENS_CONTAINERS})

set(_SOURCES_COMMON_SOURCE_FILES_ZLIB
  "Common/zlib/adler32.c"
  "Common/zlib/compress.c"
  "Common/zlib/crc32.c"
  "Common/zlib/crc32.h"
  "Common/zlib/deflate.c"
  "Common/zlib/deflate.h"
  "Common/zlib/gzclose.c"
  "Common/zlib/gzguts.h"
  "Common/zlib/gzlib.c"
  "Common/zlib/gzread.c"
  "Common/zlib/gzwrite.c"
  "Common/zlib/infback.c"
  "Common/zlib/inffast.c"
  "Common/zlib/inffast.h"
  "Common/zlib/inffixed.h"
  "Common/zlib/inflate.c"
  "Common/zlib/inflate.h"
  "Common/zlib/inftrees.c"
  "Common/zlib/inftrees.h"
  "Common/zlib/trees.c"
  "Common/zlib/trees.h"
  "Common/zlib/uncompr.c"
  "Common/zlib/zconf.h"
  "Common/zlib/zlib.h"
  "Common/zlib/zutil.c"
  "Common/zlib/zutil.h"
)
source_group("Common/Source Files/zlib" FILES ${_SOURCES_COMMON_SOURCE_FILES_ZLIB})

set(_SOURCES_DURANGO
  "Durango/Durango_App.cpp"
  "Durango/Durango_App.h"
  "Durango/Durango_UIController.cpp"
  "Durango/Durango_UIController.h"
  "Durango/Resource.h"
  "Durango/SmallLogo.png"
  "Durango/SplashScreen.png"
  "Durango/StoreLogo.png"
)
source_group("Durango" FILES ${_SOURCES_DURANGO})

set(_SOURCES_DURANGO_4JLIBS_INC
  "Durango/4JLibs/inc/4J_Input.h"
  "Durango/4JLibs/inc/4J_Profile.h"
  "Durango/4JLibs/inc/4J_Render.h"
  "Durango/4JLibs/inc/4J_Storage.h"
)
source_group("Durango/4JLibs/inc" FILES ${_SOURCES_DURANGO_4JLIBS_INC})

set(_SOURCES_DURANGO_DURANGOEXTRAS
  "Durango/DurangoExtras/DurangoStubs.cpp"
  "Durango/DurangoExtras/DurangoStubs.h"
)
source_group("Durango/DurangoExtras" FILES ${_SOURCES_DURANGO_DURANGOEXTRAS})

set(_SOURCES_DURANGO_IGGY_GDRAW
  "Durango/Iggy/gdraw/gdraw_d3d10_shaders.inl"
  "Durango/Iggy/gdraw/gdraw_d3d11.cpp"
  "Durango/Iggy/gdraw/gdraw_d3d11.h"
  "Durango/Iggy/gdraw/gdraw_d3d1x_shared.inl"
  "Durango/Iggy/gdraw/gdraw_shared.inl"
)
source_group("Durango/Iggy/gdraw" FILES ${_SOURCES_DURANGO_IGGY_GDRAW})

set(_SOURCES_DURANGO_IGGY_INCLUDE
  "Durango/Iggy/include/gdraw.h"
  "Durango/Iggy/include/iggy.h"
  "Durango/Iggy/include/iggyexpruntime.h"
  "Durango/Iggy/include/iggyperfmon.h"
  "Durango/Iggy/include/rrCore.h"
)
source_group("Durango/Iggy/include" FILES ${_SOURCES_DURANGO_IGGY_INCLUDE})

set(_SOURCES_DURANGO_NETWORK
  "Durango/Network/ChatIntegrationLayer.cpp"
  "Durango/Network/ChatIntegrationLayer.h"
  "Durango/Network/DQRNetworkManager.cpp"
  "Durango/Network/DQRNetworkManager.h"
  "Durango/Network/DQRNetworkManager_FriendSessions.cpp"
  "Durango/Network/DQRNetworkManager_Log.cpp"
  "Durango/Network/DQRNetworkManager_SendReceive.cpp"
  "Durango/Network/DQRNetworkManager_XRNSEvent.cpp"
  "Durango/Network/DQRNetworkPlayer.cpp"
  "Durango/Network/DQRNetworkPlayer.h"
  "Durango/Network/NetworkPlayerDurango.cpp"
  "Durango/Network/NetworkPlayerDurango.h"
  "Durango/Network/PartyController.cpp"
  "Durango/Network/PartyController.h"
  "Durango/Network/PlatformNetworkManagerDurango.cpp"
  "Durango/Network/PlatformNetworkManagerDurango.h"
  "Durango/Network/base64.cpp"
  "Durango/Network/base64.h"
)
source_group("Durango/Network" FILES ${_SOURCES_DURANGO_NETWORK})

set(_SOURCES_DURANGO_SERVICECONFIG
  "Durango/ServiceConfig/Events-XBLA.8-149E11AEEvents.h"
)
source_group("Durango/ServiceConfig" FILES ${_SOURCES_DURANGO_SERVICECONFIG})

set(_SOURCES_DURANGO_SOURCE_FILES
  "Durango/ApplicationView.cpp"
  "Durango/ApplicationView.h"
  "Durango/Durango_Minecraft.cpp"
  "Durango/Minecraft_Macros.h"
  "Durango/PresenceIds.h"
)
source_group("Durango/Source Files" FILES ${_SOURCES_DURANGO_SOURCE_FILES})

set(_SOURCES_DURANGO_SOURCE_FILES_ACHIEVEMENTS
  "Durango/Achievements/AchievementManager.cpp"
  "Durango/Achievements/AchievementManager.h"
)
source_group("Durango/Source Files/Achievements" FILES ${_SOURCES_DURANGO_SOURCE_FILES_ACHIEVEMENTS})

set(_SOURCES_DURANGO_SOURCE_FILES_LEADERBOARDS
  "Durango/Leaderboards/DurangoLeaderboardManager.cpp"
  "Durango/Leaderboards/DurangoLeaderboardManager.h"
  "Durango/Leaderboards/DurangoStatsDebugger.cpp"
  "Durango/Leaderboards/DurangoStatsDebugger.h"
  "Durango/Leaderboards/GameProgress.cpp"
  "Durango/Leaderboards/GameProgress.h"
)
source_group("Durango/Source Files/Leaderboards" FILES ${_SOURCES_DURANGO_SOURCE_FILES_LEADERBOARDS})

set(_SOURCES_DURANGO_SOURCE_FILES_SENTIENT
  "Durango/Sentient/DurangoTelemetry.cpp"
  "Durango/Sentient/DurangoTelemetry.h"
  "Durango/Sentient/DynamicConfigurations.h"
  "Durango/Sentient/MinecraftTelemetry.h"
  "Durango/Sentient/SentientManager.h"
  "Durango/Sentient/SentientStats.h"
  "Durango/Sentient/SentientTelemetryCommon.h"
  "Durango/Sentient/TelemetryEnum.h"
)
source_group("Durango/Source Files/Sentient" FILES ${_SOURCES_DURANGO_SOURCE_FILES_SENTIENT})

set(_SOURCES_DURANGO_SOURCE_FILES_SOCIAL
  "Durango/Social/SocialManager.h"
)
source_group("Durango/Source Files/Social" FILES ${_SOURCES_DURANGO_SOURCE_FILES_SOCIAL})

set(_SOURCES_DURANGO_XML
  "Durango/XML/ATGXmlParser.cpp"
  "Durango/XML/ATGXmlParser.h"
  "Durango/XML/xmlFilesCallback.h"
)
source_group("Durango/XML" FILES ${_SOURCES_DURANGO_XML})

set(_SOURCES_HEADER_FILES
  "BufferedImage.h"
  "MemTexture.h"
  "MemTextureProcessor.h"
  "MobSkinMemTextureProcessor.h"
  "Orbis/GameConfig/Minecraft.spa.h"
  "SkinBox.h"
  "extraX64client.h"
  "stdafx.h"
  "stubs.h"
)
source_group("Header Files" FILES ${_SOURCES_HEADER_FILES})

set(_SOURCES_ORBIS
  "Orbis/Orbis_App.cpp"
  "Orbis/Orbis_App.h"
  "Orbis/Orbis_PlayerUID.cpp"
  "Orbis/Orbis_PlayerUID.h"
  "Orbis/Orbis_UIController.cpp"
  "Orbis/Orbis_UIController.h"
  "Orbis/user_malloc.cpp"
  "Orbis/user_malloc_for_tls.cpp"
  "Orbis/user_new.cpp"
)
source_group("Orbis" FILES ${_SOURCES_ORBIS})

set(_SOURCES_ORBIS_4JLIBS_INC
  "Orbis/4JLibs/inc/4J_Input.h"
  "Orbis/4JLibs/inc/4J_Profile.h"
  "Orbis/4JLibs/inc/4J_Render.h"
  "Orbis/4JLibs/inc/4J_Storage.h"
)
source_group("Orbis/4JLibs/inc" FILES ${_SOURCES_ORBIS_4JLIBS_INC})

set(_SOURCES_ORBIS_4JLIBS_LIBS
  "Orbis/4JLibs/Libs/4J_Input.a"
  "Orbis/4JLibs/Libs/4J_Input_d.a"
  "Orbis/4JLibs/Libs/4J_Input_r.a"
  "Orbis/4JLibs/Libs/4J_Render.a"
  "Orbis/4JLibs/Libs/4J_Render_d.a"
)
source_group("Orbis/4JLibs/libs" FILES ${_SOURCES_ORBIS_4JLIBS_LIBS})

set(_SOURCES_ORBIS_IGGY_GDRAW
  "Orbis/Iggy/gdraw/gdraw_orbis.cpp"
  "Orbis/Iggy/gdraw/gdraw_orbis.h"
  "Orbis/Iggy/gdraw/gdraw_orbis_shaders.inl"
  "Orbis/Iggy/gdraw/gdraw_shared.inl"
)
source_group("Orbis/Iggy/gdraw" FILES ${_SOURCES_ORBIS_IGGY_GDRAW})

set(_SOURCES_ORBIS_IGGY_INCLUDE
  "Orbis/Iggy/include/gdraw.h"
  "Orbis/Iggy/include/iggy.h"
  "Orbis/Iggy/include/iggyexpruntime.h"
  "Orbis/Iggy/include/iggyperfmon.h"
  "Orbis/Iggy/include/iggyperfmon_orbis.h"
  "Orbis/Iggy/include/rrCore.h"
)
source_group("Orbis/Iggy/include" FILES ${_SOURCES_ORBIS_IGGY_INCLUDE})

set(_SOURCES_ORBIS_MILES_SOUND_SYSTEM_LIB
  "Orbis/Miles/lib/mssorbis.a"
)
source_group("Orbis/Miles Sound System/lib" FILES ${_SOURCES_ORBIS_MILES_SOUND_SYSTEM_LIB})

set(_SOURCES_ORBIS_NETWORK
  "Orbis/Network/Orbis_NPToolkit.cpp"
  "Orbis/Network/Orbis_NPToolkit.h"
  "Orbis/Network/PsPlusUpsellWrapper_Orbis.cpp"
  "Orbis/Network/PsPlusUpsellWrapper_Orbis.h"
  "Orbis/Network/SQRNetworkManager_Orbis.cpp"
  "Orbis/Network/SQRNetworkManager_Orbis.h"
  "Orbis/Network/SonyCommerce_Orbis.cpp"
  "Orbis/Network/SonyCommerce_Orbis.h"
  "Orbis/Network/SonyHttp_Orbis.cpp"
  "Orbis/Network/SonyHttp_Orbis.h"
  "Orbis/Network/SonyRemoteStorage_Orbis.cpp"
  "Orbis/Network/SonyRemoteStorage_Orbis.h"
  "Orbis/Network/SonyVoiceChat_Orbis.cpp"
  "Orbis/Network/SonyVoiceChat_Orbis.h"
)
source_group("Orbis/Network" FILES ${_SOURCES_ORBIS_NETWORK})

set(_SOURCES_ORBIS_ORBISEXTRAS
  "Orbis/OrbisExtras/OrbisMaths.h"
  "Orbis/OrbisExtras/OrbisStubs.cpp"
  "Orbis/OrbisExtras/OrbisStubs.h"
  "Orbis/OrbisExtras/OrbisTypes.h"
  "Orbis/OrbisExtras/TLSStorage.cpp"
  "Orbis/OrbisExtras/TLSStorage.h"
  "Orbis/OrbisExtras/winerror.h"
)
source_group("Orbis/OrbisExtras" FILES ${_SOURCES_ORBIS_ORBISEXTRAS})

set(_SOURCES_ORBIS_SOURCE_FILES
  "Orbis/Minecraft_Macros.h"
  "Orbis/Orbis_Minecraft.cpp"
  "Orbis/ps4__np_conf.h"
)
source_group("Orbis/Source Files" FILES ${_SOURCES_ORBIS_SOURCE_FILES})

set(_SOURCES_ORBIS_SOURCE_FILES_LEADERBOARDS
  "Orbis/Leaderboards/OrbisLeaderboardManager.cpp"
  "Orbis/Leaderboards/OrbisLeaderboardManager.h"
)
source_group("Orbis/Source Files/Leaderboards" FILES ${_SOURCES_ORBIS_SOURCE_FILES_LEADERBOARDS})

set(_SOURCES_ORBIS_SOURCE_FILES_SENTIENT
  "Orbis/Sentient/DynamicConfigurations.h"
  "Orbis/Sentient/MinecraftTelemetry.h"
  "Orbis/Sentient/SentientManager.h"
  "Orbis/Sentient/SentientStats.h"
  "Orbis/Sentient/SentientTelemetryCommon.h"
  "Orbis/Sentient/TelemetryEnum.h"
)
source_group("Orbis/Source Files/Sentient" FILES ${_SOURCES_ORBIS_SOURCE_FILES_SENTIENT})

set(_SOURCES_ORBIS_SOURCE_FILES_SOCIAL
  "Orbis/Social/SocialManager.h"
)
source_group("Orbis/Source Files/Social" FILES ${_SOURCES_ORBIS_SOURCE_FILES_SOCIAL})

set(_SOURCES_ORBIS_XML
  "Orbis/XML/ATGXmlParser.h"
)
source_group("Orbis/XML" FILES ${_SOURCES_ORBIS_XML})

set(_SOURCES_PS3
  "PS3/PS3Extras/C4JSpursJob.cpp"
  "PS3/PS3Extras/C4JSpursJob.h"
  "PS3/PS3_App.cpp"
  "PS3/PS3_App.h"
  "PS3/PS3_UIController.cpp"
  "PS3/PS3_UIController.h"
  "PS3/Passphrase/ps3__np_conf.h"
)
source_group("PS3" FILES ${_SOURCES_PS3})

set(_SOURCES_PS3_4JLIBS
  "PS3/4JLibs/STO_TitleSmallStorage.cpp"
  "PS3/4JLibs/STO_TitleSmallStorage.h"
)
source_group("PS3/4JLibs" FILES ${_SOURCES_PS3_4JLIBS})

set(_SOURCES_PS3_4JLIBS_INC
  "PS3/4JLibs/inc/4J_Input.h"
  "PS3/4JLibs/inc/4J_Profile.h"
  "PS3/4JLibs/inc/4J_Render.h"
  "PS3/4JLibs/inc/4J_Storage.h"
)
source_group("PS3/4JLibs/inc" FILES ${_SOURCES_PS3_4JLIBS_INC})

set(_SOURCES_PS3_CHUNKREBUILD_SPU
  "PS3/SPU_Tasks/ChunkUpdate/BedTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/BookshelfTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/BrewingStandTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/Bush_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/ButtonTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/CactusTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/CakeTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/CauldronTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/ChestTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/ChunkRebuildData.cpp"
  "PS3/SPU_Tasks/ChunkUpdate/ChunkRebuildData.h"
  "PS3/SPU_Tasks/ChunkUpdate/CocoaTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/CropTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/DetectorRailTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/DiodeTile_SPU.cpp"
  "PS3/SPU_Tasks/ChunkUpdate/DiodeTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/Direction_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/DirectionalTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/DirtTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/DispenserTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/DoorTile_SPU.cpp"
  "PS3/SPU_Tasks/ChunkUpdate/DoorTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/EggTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/EnchantmentTableTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/EntityTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/Facing_SPU.cpp"
  "PS3/SPU_Tasks/ChunkUpdate/Facing_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/FarmTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/FenceGateTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/FenceTile_SPU.cpp"
  "PS3/SPU_Tasks/ChunkUpdate/FenceTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/FireTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/FurnaceTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/GlassTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/GrassTile_SPU.cpp"
  "PS3/SPU_Tasks/ChunkUpdate/GrassTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/HalfSlabTile_SPU.cpp"
  "PS3/SPU_Tasks/ChunkUpdate/HalfSlabTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/HalfTransparentTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/HugeMushroomTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/IceTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/Icon_SPU.cpp"
  "PS3/SPU_Tasks/ChunkUpdate/Icon_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/LadderTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/LeafTile_SPU.cpp"
  "PS3/SPU_Tasks/ChunkUpdate/LeafTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/LeverTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/LiquidTile_SPU.cpp"
  "PS3/SPU_Tasks/ChunkUpdate/LiquidTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/Material_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/MelonTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/Mushroom_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/MycelTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/NetherStalkTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/PistonBaseTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/PistonExtensionTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/PistonMovingPiece_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/PortalTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/PressurePlateTile_SPU.cpp"
  "PS3/SPU_Tasks/ChunkUpdate/PressurePlateTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/PumpkinTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/RailTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/RecordPlayerTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/RedStoneDustTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/RedlightTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/ReedTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/SandStoneTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/Sapling_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/SignTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/SmoothStoneBrickTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/StairTile_SPU.cpp"
  "PS3/SPU_Tasks/ChunkUpdate/StairTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/StemTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/StoneMonsterTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/TallGrass_SPU.cpp"
  "PS3/SPU_Tasks/ChunkUpdate/TallGrass_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/Tesselator_SPU.cpp"
  "PS3/SPU_Tasks/ChunkUpdate/Tesselator_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/TheEndPortalFrameTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/TheEndPortal_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/ThinFenceTile_SPU.cpp"
  "PS3/SPU_Tasks/ChunkUpdate/ThinFenceTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/TileRenderer_SPU.cpp"
  "PS3/SPU_Tasks/ChunkUpdate/TileRenderer_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/Tile_SPU.cpp"
  "PS3/SPU_Tasks/ChunkUpdate/Tile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/TntTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/TopSnowTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/TorchTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/TrapDoorTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/TreeTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/VineTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/WaterLilyTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/WebTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/WoodTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/WorkbenchTile_SPU.h"
  "PS3/SPU_Tasks/ChunkUpdate/stdafx.h"
  "PS3/SPU_Tasks/ChunkUpdate/stubs_SPU.h"
)
source_group("PS3/ChunkRebuild_SPU" FILES ${_SOURCES_PS3_CHUNKREBUILD_SPU})

set(_SOURCES_PS3_COMPRESSEDTILE_SPU
  "PS3/SPU_Tasks/CompressedTile/CompressedTileStorage_SPU.cpp"
  "PS3/SPU_Tasks/CompressedTile/CompressedTileStorage_SPU.h"
)
source_group("PS3/CompressedTile_SPU" FILES ${_SOURCES_PS3_COMPRESSEDTILE_SPU})

set(_SOURCES_PS3_IGGY_GDRAW
  "PS3/Iggy/gdraw/gdraw_ps3gcm.cpp"
  "PS3/Iggy/gdraw/gdraw_ps3gcm.h"
  "PS3/Iggy/gdraw/gdraw_ps3gcm_shaders.inl"
  "PS3/Iggy/gdraw/gdraw_shared.inl"
)
source_group("PS3/Iggy/gdraw" FILES ${_SOURCES_PS3_IGGY_GDRAW})

set(_SOURCES_PS3_IGGY_INCLUDE
  "PS3/Iggy/include/gdraw.h"
  "PS3/Iggy/include/iggy.h"
  "PS3/Iggy/include/iggyexpruntime.h"
  "PS3/Iggy/include/iggyperfmon.h"
  "PS3/Iggy/include/iggyperfmon_ps3.h"
  "PS3/Iggy/include/rrCore.h"
)
source_group("PS3/Iggy/include" FILES ${_SOURCES_PS3_IGGY_INCLUDE})

set(_SOURCES_PS3_MILES_SOUND_SYSTEM_LIB
  "PS3/Miles/lib/audps3.a"
  "PS3/Miles/lib/fltps3.a"
  "PS3/Miles/lib/mssps3.a"
)
source_group("PS3/Miles Sound System/lib" FILES ${_SOURCES_PS3_MILES_SOUND_SYSTEM_LIB})

set(_SOURCES_PS3_MILES_SOUND_SYSTEM_LIB_SPU
  "PS3/Miles/lib/spu/binkaspu.a"
  "PS3/Miles/lib/spu/mssppu_raw.a"
  "PS3/Miles/lib/spu/mssppu_spurs.a"
  "PS3/Miles/lib/spu/mssppu_sputhreads.a"
  "PS3/Miles/lib/spu/mssspu.a"
  "PS3/Miles/lib/spu/mssspu_raw.a"
  "PS3/Miles/lib/spu/mssspu_spurs.a"
  "PS3/Miles/lib/spu/mssspu_sputhreads.a"
)
source_group("PS3/Miles Sound System/lib/spu" FILES ${_SOURCES_PS3_MILES_SOUND_SYSTEM_LIB_SPU})

set(_SOURCES_PS3_PS3EXTRAS
  "PS3/PS3Extras/C4JThread_SPU.cpp"
  "PS3/PS3Extras/C4JThread_SPU.h"
  "PS3/PS3Extras/EdgeZLib.cpp"
  "PS3/PS3Extras/EdgeZLib.h"
  "PS3/PS3Extras/PS3Maths.h"
  "PS3/PS3Extras/PS3Strings.cpp"
  "PS3/PS3Extras/PS3Strings.h"
  "PS3/PS3Extras/Ps3Stubs.cpp"
  "PS3/PS3Extras/Ps3Stubs.h"
  "PS3/PS3Extras/Ps3Types.h"
  "PS3/PS3Extras/ShutdownManager.cpp"
  "PS3/PS3Extras/ShutdownManager.h"
  "PS3/PS3Extras/TLSStorage.cpp"
  "PS3/PS3Extras/TLSStorage.h"
  "PS3/PS3Extras/winerror.h"
)
source_group("PS3/PS3Extras" FILES ${_SOURCES_PS3_PS3EXTRAS})

set(_SOURCES_PS3_SOURCE_FILES
  "PS3/Minecraft_Macros.h"
  "PS3/PS3_Minecraft.cpp"
  "PS3/PS3_PlayerUID.cpp"
  "PS3/PS3_PlayerUID.h"
)
source_group("PS3/Source Files" FILES ${_SOURCES_PS3_SOURCE_FILES})

set(_SOURCES_PS3_SOURCE_FILES_AUDIO
  "PS3/Audio/PS3_SoundEngine.cpp"
)
source_group("PS3/Source Files/Audio" FILES ${_SOURCES_PS3_SOURCE_FILES_AUDIO})

set(_SOURCES_PS3_SOURCE_FILES_LEADERBOARDS
  "PS3/Leaderboards/PS3LeaderboardManager.cpp"
  "PS3/Leaderboards/PS3LeaderboardManager.h"
)
source_group("PS3/Source Files/Leaderboards" FILES ${_SOURCES_PS3_SOURCE_FILES_LEADERBOARDS})

set(_SOURCES_PS3_SOURCE_FILES_NETWORK
  "PS3/Network/SQRNetworkManager_PS3.cpp"
  "PS3/Network/SQRNetworkManager_PS3.h"
  "PS3/Network/SonyCommerce_PS3.cpp"
  "PS3/Network/SonyCommerce_PS3.h"
  "PS3/Network/SonyHttp_PS3.cpp"
  "PS3/Network/SonyHttp_PS3.h"
  "PS3/Network/SonyRemoteStorage_PS3.cpp"
  "PS3/Network/SonyRemoteStorage_PS3.h"
  "PS3/Network/SonyVoiceChat.cpp"
  "PS3/Network/SonyVoiceChat.h"
)
source_group("PS3/Source Files/Network" FILES ${_SOURCES_PS3_SOURCE_FILES_NETWORK})

set(_SOURCES_PS3_SOURCE_FILES_SENTIENT
  "PS3/Sentient/DynamicConfigurations.h"
  "PS3/Sentient/MinecraftTelemetry.h"
  "PS3/Sentient/SentientManager.h"
  "PS3/Sentient/SentientStats.h"
  "PS3/Sentient/SentientTelemetryCommon.h"
  "PS3/Sentient/TelemetryEnum.h"
)
source_group("PS3/Source Files/Sentient" FILES ${_SOURCES_PS3_SOURCE_FILES_SENTIENT})

set(_SOURCES_PS3_SOURCE_FILES_SOCIAL
  "PS3/Social/SocialManager.h"
)
source_group("PS3/Source Files/Social" FILES ${_SOURCES_PS3_SOURCE_FILES_SOCIAL})

set(_SOURCES_PSVITA
  "PSVita/PSVita_App.cpp"
  "PSVita/PSVita_App.h"
  "PSVita/PSVita_UIController.cpp"
  "PSVita/PSVita_UIController.h"
)
source_group("PSVita" FILES ${_SOURCES_PSVITA})

set(_SOURCES_PSVITA_4JLIBS_INC
  "PSVita/4JLibs/inc/4J_Input.h"
  "PSVita/4JLibs/inc/4J_Profile.h"
  "PSVita/4JLibs/inc/4J_Render.h"
  "PSVita/4JLibs/inc/4J_Storage.h"
)
source_group("PSVita/4JLibs/inc" FILES ${_SOURCES_PSVITA_4JLIBS_INC})

set(_SOURCES_PSVITA_GAMECONFIG
  "PSVita/GameConfig/Minecraft.gameconfig"
  "PSVita/GameConfig/Minecraft.spa"
  "PSVita/GameConfig/Minecraft.spa.h"
)
source_group("PSVita/GameConfig" FILES ${_SOURCES_PSVITA_GAMECONFIG})

set(_SOURCES_PSVITA_IGGY_GDRAW
  "PSVita/Iggy/gdraw/gdraw_psp2.cpp"
  "PSVita/Iggy/gdraw/gdraw_psp2.h"
  "PSVita/Iggy/gdraw/gdraw_psp2_shaders.inl"
  "PSVita/Iggy/gdraw/gdraw_shared.inl"
)
source_group("PSVita/Iggy/gdraw" FILES ${_SOURCES_PSVITA_IGGY_GDRAW})

set(_SOURCES_PSVITA_IGGY_INCLUDE
  "PSVita/Iggy/include/gdraw.h"
  "PSVita/Iggy/include/iggy.h"
  "PSVita/Iggy/include/iggyexpruntime.h"
  "PSVita/Iggy/include/iggyperfmon.h"
  "PSVita/Iggy/include/iggyperfmon_psp2.h"
  "PSVita/Iggy/include/rrCore.h"
)
source_group("PSVita/Iggy/include" FILES ${_SOURCES_PSVITA_IGGY_INCLUDE})

set(_SOURCES_PSVITA_PSVITAEXTRAS
  "PSVita/PSVitaExtras/Conf.h"
  "PSVita/PSVitaExtras/CustomMap.cpp"
  "PSVita/PSVitaExtras/CustomMap.h"
  "PSVita/PSVitaExtras/CustomSet.cpp"
  "PSVita/PSVitaExtras/CustomSet.h"
  "PSVita/PSVitaExtras/PSVitaMaths.h"
  "PSVita/PSVitaExtras/PSVitaStrings.cpp"
  "PSVita/PSVitaExtras/PSVitaStrings.h"
  "PSVita/PSVitaExtras/PSVitaStubs.h"
  "PSVita/PSVitaExtras/PSVitaTLSStorage.cpp"
  "PSVita/PSVitaExtras/PSVitaTLSStorage.h"
  "PSVita/PSVitaExtras/PSVitaTypes.h"
  "PSVita/PSVitaExtras/PsVitaStubs.cpp"
  "PSVita/PSVitaExtras/ShutdownManager.cpp"
  "PSVita/PSVitaExtras/ShutdownManager.h"
  "PSVita/PSVitaExtras/libdivide.h"
  "PSVita/PSVitaExtras/user_malloc.c"
  "PSVita/PSVitaExtras/user_malloc_for_tls.c"
  "PSVita/PSVitaExtras/user_new.cpp"
  "PSVita/PSVitaExtras/zconf.h"
  "PSVita/PSVitaExtras/zlib.h"
)
source_group("PSVita/PSVitaExtras" FILES ${_SOURCES_PSVITA_PSVITAEXTRAS})

set(_SOURCES_PSVITA_SOURCE_FILES
  "PSVita/PSVita_Minecraft.cpp"
)
source_group("PSVita/Source Files" FILES ${_SOURCES_PSVITA_SOURCE_FILES})

set(_SOURCES_PSVITA_SOURCE_FILES_LEADERBOARDS
  "PSVita/Leaderboards/PSVitaLeaderboardManager.cpp"
  "PSVita/Leaderboards/PSVitaLeaderboardManager.h"
)
source_group("PSVita/Source Files/Leaderboards" FILES ${_SOURCES_PSVITA_SOURCE_FILES_LEADERBOARDS})

set(_SOURCES_PSVITA_SOURCE_FILES_NETWORK
  "PSVita/Network/PSVita_NPToolkit.cpp"
  "PSVita/Network/PSVita_NPToolkit.h"
  "PSVita/Network/SQRNetworkManager_AdHoc_Vita.cpp"
  "PSVita/Network/SQRNetworkManager_AdHoc_Vita.h"
  "PSVita/Network/SQRNetworkManager_Vita.cpp"
  "PSVita/Network/SQRNetworkManager_Vita.h"
  "PSVita/Network/SonyCommerce_Vita.cpp"
  "PSVita/Network/SonyCommerce_Vita.h"
  "PSVita/Network/SonyHttp_Vita.cpp"
  "PSVita/Network/SonyHttp_Vita.h"
  "PSVita/Network/SonyRemoteStorage_Vita.cpp"
  "PSVita/Network/SonyRemoteStorage_Vita.h"
  "PSVita/Network/SonyVoiceChat_Vita.cpp"
  "PSVita/Network/SonyVoiceChat_Vita.h"
)
source_group("PSVita/Source Files/Network" FILES ${_SOURCES_PSVITA_SOURCE_FILES_NETWORK})

set(_SOURCES_PSVITA_SOURCE_FILES_SENTIENT
  "PSVita/Sentient/DynamicConfigurations.h"
  "PSVita/Sentient/MinecraftTelemetry.h"
  "PSVita/Sentient/SentientManager.h"
  "PSVita/Sentient/SentientStats.h"
  "PSVita/Sentient/SentientTelemetryCommon.h"
  "PSVita/Sentient/TelemetryEnum.h"
)
source_group("PSVita/Source Files/Sentient" FILES ${_SOURCES_PSVITA_SOURCE_FILES_SENTIENT})

set(_SOURCES_PSVITA_SOURCE_FILES_SOCIAL
  "PSVita/Social/SocialManager.h"
)
source_group("PSVita/Source Files/Social" FILES ${_SOURCES_PSVITA_SOURCE_FILES_SOCIAL})

set(_SOURCES_PSVITA_XML
  "PSVita/XML/ATGXmlParser.h"
)
source_group("PSVita/XML" FILES ${_SOURCES_PSVITA_XML})

set(_SOURCES_SOURCE_FILES
  "BufferedImage.cpp"
  "Extrax64Stubs.cpp"
  "compat_shims.cpp"
  "glWrapper.cpp"
  "iob_shim.asm"
  "stdafx.cpp"
  "stubs.cpp"
)
source_group("Source Files" FILES ${_SOURCES_SOURCE_FILES})

set(_SOURCES_WINDOWS
  "Xbox/MinecraftWindows.rc"
  "Xbox/Resource.h"
  "Xbox/targetver.h"
)
source_group("Windows" FILES ${_SOURCES_WINDOWS})

set(_SOURCES_WINDOWS64
  "Windows64/Resource.h"
  "Windows64/Windows64_App.cpp"
  "Windows64/Windows64_App.h"
  "Windows64/Windows64_UIController.cpp"
  "Windows64/Windows64_UIController.h"
)
source_group("Windows64" FILES ${_SOURCES_WINDOWS64})

set(_SOURCES_WINDOWS64_4JLIBS_INC
  "Windows64/4JLibs/inc/4J_Input.h"
  "Windows64/4JLibs/inc/4J_Profile.h"
  "Windows64/4JLibs/inc/4J_Render.h"
  "Windows64/4JLibs/inc/4J_Storage.h"
)
source_group("Windows64/4JLibs/inc" FILES ${_SOURCES_WINDOWS64_4JLIBS_INC})

set(_SOURCES_WINDOWS64_GAMECONFIG
  "Windows64/GameConfig/Minecraft.gameconfig"
  "Windows64/GameConfig/Minecraft.spa"
  "Windows64/GameConfig/Minecraft.spa.h"
)
source_group("Windows64/GameConfig" FILES ${_SOURCES_WINDOWS64_GAMECONFIG})

set(_SOURCES_WINDOWS64_IGGY_GDRAW
  "Windows64/Iggy/gdraw/gdraw_d3d10_shaders.inl"
  "Windows64/Iggy/gdraw/gdraw_d3d11.cpp"
  "Windows64/Iggy/gdraw/gdraw_d3d11.h"
  "Windows64/Iggy/gdraw/gdraw_d3d1x_shared.inl"
  "Windows64/Iggy/gdraw/gdraw_gl_shaders.inl"
  "Windows64/Iggy/gdraw/gdraw_gl_shared.inl"
  "Windows64/Iggy/gdraw/gdraw_shared.inl"
)
source_group("Windows64/Iggy/gdraw" FILES ${_SOURCES_WINDOWS64_IGGY_GDRAW})

set(_SOURCES_WINDOWS64_IGGY_INCLUDE
  "Windows64/Iggy/include/gdraw.h"
  "Windows64/Iggy/include/iggy.h"
  "Windows64/Iggy/include/iggyexpruntime.h"
  "Windows64/Iggy/include/iggyperfmon.h"
  "Windows64/Iggy/include/rrCore.h"
)
source_group("Windows64/Iggy/include" FILES ${_SOURCES_WINDOWS64_IGGY_INCLUDE})

set(_SOURCES_WINDOWS64_SOURCE_FILES
  "Windows64/KeyboardMouseInput.cpp"
  "Windows64/KeyboardMouseInput.h"
  "Windows64/Minecraft_Macros.h"
  "Windows64/PostProcesser.cpp"
  "Windows64/Windows64_Minecraft.cpp"
)
source_group("Windows64/Source Files" FILES ${_SOURCES_WINDOWS64_SOURCE_FILES})

set(_SOURCES_WINDOWS64_SOURCE_FILES_LEADERBOARDS
  "Windows64/Leaderboards/WindowsLeaderboardManager.cpp"
  "Windows64/Leaderboards/WindowsLeaderboardManager.h"
)
source_group("Windows64/Source Files/Leaderboards" FILES ${_SOURCES_WINDOWS64_SOURCE_FILES_LEADERBOARDS})

set(_SOURCES_WINDOWS64_SOURCE_FILES_NETWORK
  "Windows64/Network/WinsockNetLayer.cpp"
  "Windows64/Network/WinsockNetLayer.h"
)
source_group("Windows64/Source Files/Network" FILES ${_SOURCES_WINDOWS64_SOURCE_FILES_NETWORK})

set(_SOURCES_WINDOWS64_SOURCE_FILES_SENTIENT
  "Windows64/Sentient/DynamicConfigurations.h"
  "Windows64/Sentient/MinecraftTelemetry.h"
  "Windows64/Sentient/SentientManager.h"
  "Windows64/Sentient/SentientStats.h"
  "Windows64/Sentient/SentientTelemetryCommon.h"
  "Windows64/Sentient/TelemetryEnum.h"
)
source_group("Windows64/Source Files/Sentient" FILES ${_SOURCES_WINDOWS64_SOURCE_FILES_SENTIENT})

set(_SOURCES_WINDOWS64_SOURCE_FILES_SOCIAL
  "Windows64/Social/SocialManager.h"
)
source_group("Windows64/Source Files/Social" FILES ${_SOURCES_WINDOWS64_SOURCE_FILES_SOCIAL})

set(_SOURCES_WINDOWS64_XML
  "Windows64/XML/ATGXmlParser.h"
)
source_group("Windows64/XML" FILES ${_SOURCES_WINDOWS64_XML})

set(_SOURCES_XBOX_4JLIBS_MEDIA
  "Xbox/4JLibs/Media/4J_strings.resx"
)
source_group("Xbox/4JLibs/Media" FILES ${_SOURCES_XBOX_4JLIBS_MEDIA})

set(_SOURCES_XBOX_4JLIBS_INC
  "Xbox/4JLibs/inc/4J_Input.h"
  "Xbox/4JLibs/inc/4J_Profile.h"
  "Xbox/4JLibs/inc/4J_Render.h"
  "Xbox/4JLibs/inc/4J_Storage.h"
  "Xbox/4JLibs/inc/4J_xtms.h"
)
source_group("Xbox/4JLibs/inc" FILES ${_SOURCES_XBOX_4JLIBS_INC})

set(_SOURCES_XBOX_GAMECONFIG
  "Xbox/GameConfig/Minecraft.gameconfig"
  "Xbox/GameConfig/Minecraft.spa"
  "Xbox/GameConfig/Minecraft.spa.h"
)
source_group("Xbox/GameConfig" FILES ${_SOURCES_XBOX_GAMECONFIG})

set(_SOURCES_XBOX_SENTIENTLIBS_INC
  "Xbox/Sentient/Include/SenClientAvatar.h"
  "Xbox/Sentient/Include/SenClientBoxArt.h"
  "Xbox/Sentient/Include/SenClientConfig.h"
  "Xbox/Sentient/Include/SenClientCore.h"
  "Xbox/Sentient/Include/SenClientCulture.h"
  "Xbox/Sentient/Include/SenClientCultureBackCompat_SenBoxArt.h"
  "Xbox/Sentient/Include/SenClientCultureBackCompat_SenClientUGC.h"
  "Xbox/Sentient/Include/SenClientCultureBackCompat_SenCore.h"
  "Xbox/Sentient/Include/SenClientCultureBackCompat_SenNews.h"
  "Xbox/Sentient/Include/SenClientCultureBackCompat_SenSuperstars.h"
  "Xbox/Sentient/Include/SenClientDynamicConfig.h"
  "Xbox/Sentient/Include/SenClientFame.h"
  "Xbox/Sentient/Include/SenClientFile.h"
  "Xbox/Sentient/Include/SenClientHelp.h"
  "Xbox/Sentient/Include/SenClientMain.h"
  "Xbox/Sentient/Include/SenClientMarkers.h"
  "Xbox/Sentient/Include/SenClientNews.h"
  "Xbox/Sentient/Include/SenClientPackage.h"
  "Xbox/Sentient/Include/SenClientRawData.h"
  "Xbox/Sentient/Include/SenClientResource.h"
  "Xbox/Sentient/Include/SenClientStats.h"
  "Xbox/Sentient/Include/SenClientSuperstars.h"
  "Xbox/Sentient/Include/SenClientSys.h"
  "Xbox/Sentient/Include/SenClientTypes.h"
  "Xbox/Sentient/Include/SenClientUGC.h"
  "Xbox/Sentient/Include/SenClientUGCLeaderboards.h"
  "Xbox/Sentient/Include/SenClientUGCTypes.h"
  "Xbox/Sentient/Include/SenClientUser.h"
  "Xbox/Sentient/Include/SenClientUtil.h"
  "Xbox/Sentient/Include/SenClientXML.h"
)
source_group("Xbox/SentientLibs/inc" FILES ${_SOURCES_XBOX_SENTIENTLIBS_INC})

set(_SOURCES_XBOX_SOURCE_FILES
  "Xbox/Xbox_App.cpp"
  "Xbox/Xbox_App.h"
  "Xbox/Xbox_Minecraft.cpp"
  "Xbox/Xbox_UIController.cpp"
  "Xbox/Xbox_UIController.h"
)
source_group("Xbox/Source Files" FILES ${_SOURCES_XBOX_SOURCE_FILES})

set(_SOURCES_XBOX_SOURCE_FILES_AUDIO
  "Xbox/Audio/SoundEngine.cpp"
  "Xbox/Audio/SoundEngine.h"
)
source_group("Xbox/Source Files/Audio" FILES ${_SOURCES_XBOX_SOURCE_FILES_AUDIO})

set(_SOURCES_XBOX_SOURCE_FILES_FONT
  "Xbox/Font/XUI_Font.cpp"
  "Xbox/Font/XUI_Font.h"
  "Xbox/Font/XUI_FontData.cpp"
  "Xbox/Font/XUI_FontData.h"
  "Xbox/Font/XUI_FontRenderer.cpp"
  "Xbox/Font/XUI_FontRenderer.h"
)
source_group("Xbox/Source Files/Font" FILES ${_SOURCES_XBOX_SOURCE_FILES_FONT})

set(_SOURCES_XBOX_SOURCE_FILES_LEADERBOARDS
  "Xbox/Leaderboards/XboxLeaderboardManager.cpp"
  "Xbox/Leaderboards/XboxLeaderboardManager.h"
)
source_group("Xbox/Source Files/Leaderboards" FILES ${_SOURCES_XBOX_SOURCE_FILES_LEADERBOARDS})

set(_SOURCES_XBOX_SOURCE_FILES_NETWORK
  "Xbox/Network/NetworkPlayerXbox.cpp"
  "Xbox/Network/NetworkPlayerXbox.h"
  "Xbox/Network/PlatformNetworkManagerXbox.cpp"
  "Xbox/Network/PlatformNetworkManagerXbox.h"
  "Xbox/Network/extra.h"
)
source_group("Xbox/Source Files/Network" FILES ${_SOURCES_XBOX_SOURCE_FILES_NETWORK})

set(_SOURCES_XBOX_SOURCE_FILES_SENTIENT
  "Xbox/Sentient/SentientManager.cpp"
  "Xbox/Sentient/SentientManager.h"
)
source_group("Xbox/Source Files/Sentient" FILES ${_SOURCES_XBOX_SOURCE_FILES_SENTIENT})

set(_SOURCES_XBOX_SOURCE_FILES_SENTIENT_DYNAMICCONF
  "Xbox/Sentient/DynamicConfigurations.cpp"
  "Xbox/Sentient/DynamicConfigurations.h"
  "Xbox/Sentient/trialConfigv1.bin"
)
source_group("Xbox/Source Files/Sentient/DynamicConf" FILES ${_SOURCES_XBOX_SOURCE_FILES_SENTIENT_DYNAMICCONF})

set(_SOURCES_XBOX_SOURCE_FILES_SENTIENT_TELEMETRY
  "Xbox/Sentient/MinecraftTelemetry.h"
  "Xbox/Sentient/MinecraftTelemetry.xml"
  "Xbox/Sentient/SentientStats.cpp"
  "Xbox/Sentient/SentientStats.h"
  "Xbox/Sentient/SentientTelemetryCommon.h"
  "Xbox/Sentient/TelemetryEnum.h"
)
source_group("Xbox/Source Files/Sentient/Telemetry" FILES ${_SOURCES_XBOX_SOURCE_FILES_SENTIENT_TELEMETRY})

set(_SOURCES_XBOX_SOURCE_FILES_SOCIAL
  "Xbox/Social/SocialManager.cpp"
  "Xbox/Social/SocialManager.h"
)
source_group("Xbox/Source Files/Social" FILES ${_SOURCES_XBOX_SOURCE_FILES_SOCIAL})

set(_SOURCES_XBOX_SOURCE_FILES_XUI
  "Common/XUI/XUI_CustomMessages.h"
  "Common/XUI/XUI_Reinstall.cpp"
  "Common/XUI/XUI_Reinstall.h"
  "Common/XUI/XUI_TextEntry.cpp"
  "Common/XUI/XUI_TextEntry.h"
  "Common/XUI/XUI_XZP_Icons.h"
)
source_group("Xbox/Source Files/XUI" FILES ${_SOURCES_XBOX_SOURCE_FILES_XUI})

set(_SOURCES_XBOX_SOURCE_FILES_XUI_BASE_SCENE
  "Common/XUI/XUI_BasePlayer.cpp"
  "Common/XUI/XUI_BasePlayer.h"
  "Common/XUI/XUI_Chat.cpp"
  "Common/XUI/XUI_Chat.h"
  "Common/XUI/XUI_HUD.cpp"
  "Common/XUI/XUI_HUD.h"
  "Common/XUI/XUI_Scene_Base.cpp"
  "Common/XUI/XUI_Scene_Base.h"
)
source_group("Xbox/Source Files/XUI/Base Scene" FILES ${_SOURCES_XBOX_SOURCE_FILES_XUI_BASE_SCENE})

set(_SOURCES_XBOX_SOURCE_FILES_XUI_CONTAINERS
  "Common/XUI/XUI_Scene_AbstractContainer.cpp"
  "Common/XUI/XUI_Scene_AbstractContainer.h"
  "Common/XUI/XUI_Scene_Anvil.cpp"
  "Common/XUI/XUI_Scene_Anvil.h"
  # "Common/XUI/XUI_Scene_Beacon.cpp"
  # "Common/XUI/XUI_Scene_Beacon.h"
  "Common/XUI/XUI_Scene_BrewingStand.cpp"
  "Common/XUI/XUI_Scene_BrewingStand.h"
  "Common/XUI/XUI_Scene_Container.cpp"
  "Common/XUI/XUI_Scene_Container.h"
  "Common/XUI/XUI_Scene_CraftingPanel.cpp"
  "Common/XUI/XUI_Scene_CraftingPanel.h"
  "Common/XUI/XUI_Scene_Enchant.cpp"
  "Common/XUI/XUI_Scene_Enchant.h"
  # "Common/XUI/XUI_Scene_Fireworks.cpp"
  # "Common/XUI/XUI_Scene_Fireworks.h"
  "Common/XUI/XUI_Scene_Furnace.cpp"
  "Common/XUI/XUI_Scene_Furnace.h"
  # "Common/XUI/XUI_Scene_Hopper.cpp"
  # "Common/XUI/XUI_Scene_Hopper.h"
  # "Common/XUI/XUI_Scene_HorseInventory.cpp"
  # "Common/XUI/XUI_Scene_HorseInventory.h"
  "Common/XUI/XUI_Scene_Inventory.cpp"
  "Common/XUI/XUI_Scene_Inventory.h"
  "Common/XUI/XUI_Scene_Inventory_Creative.cpp"
  "Common/XUI/XUI_Scene_Inventory_Creative.h"
  "Common/XUI/XUI_Scene_Trading.cpp"
  "Common/XUI/XUI_Scene_Trading.h"
  "Common/XUI/XUI_Scene_Trap.cpp"
  "Common/XUI/XUI_Scene_Trap.h"
)
source_group("Xbox/Source Files/XUI/Containers" FILES ${_SOURCES_XBOX_SOURCE_FILES_XUI_CONTAINERS})

set(_SOURCES_XBOX_SOURCE_FILES_XUI_CONTROLS
  "Common/XUI/XUI_Controls.h"
  "Common/XUI/XUI_Ctrl_4JEdit.cpp"
  "Common/XUI/XUI_Ctrl_4JEdit.h"
  "Common/XUI/XUI_Ctrl_4JIcon.cpp"
  "Common/XUI/XUI_Ctrl_4JIcon.h"
  "Common/XUI/XUI_Ctrl_4JList.cpp"
  "Common/XUI/XUI_Ctrl_4JList.h"
  # "Common/XUI/XUI_Ctrl_BeaconButton.cpp"
  # "Common/XUI/XUI_Ctrl_BeaconButton.h"
  "Common/XUI/XUI_Ctrl_BrewProgress.cpp"
  "Common/XUI/XUI_Ctrl_BrewProgress.h"
  "Common/XUI/XUI_Ctrl_BubblesProgress.cpp"
  "Common/XUI/XUI_Ctrl_BubblesProgress.h"
  "Common/XUI/XUI_Ctrl_BurnProgress.cpp"
  "Common/XUI/XUI_Ctrl_BurnProgress.h"
  "Common/XUI/XUI_Ctrl_CraftIngredientSlot.cpp"
  "Common/XUI/XUI_Ctrl_CraftIngredientSlot.h"
  "Common/XUI/XUI_Ctrl_EnchantButton.cpp"
  "Common/XUI/XUI_Ctrl_EnchantButton.h"
  "Common/XUI/XUI_Ctrl_EnchantmentBook.cpp"
  "Common/XUI/XUI_Ctrl_EnchantmentBook.h"
  "Common/XUI/XUI_Ctrl_EnchantmentButtonText.cpp"
  "Common/XUI/XUI_Ctrl_EnchantmentButtonText.h"
  "Common/XUI/XUI_Ctrl_FireProgress.cpp"
  "Common/XUI/XUI_Ctrl_FireProgress.h"
  "Common/XUI/XUI_Ctrl_LoadingProgress.cpp"
  "Common/XUI/XUI_Ctrl_LoadingProgress.h"
  # "Common/XUI/XUI_Ctrl_MinecraftHorse.cpp"
  # "Common/XUI/XUI_Ctrl_MinecraftHorse.h"
  "Common/XUI/XUI_Ctrl_MinecraftPlayer.cpp"
  "Common/XUI/XUI_Ctrl_MinecraftPlayer.h"
  "Common/XUI/XUI_Ctrl_MinecraftSkinPreview.cpp"
  "Common/XUI/XUI_Ctrl_MinecraftSkinPreview.h"
  "Common/XUI/XUI_Ctrl_MinecraftSlot.cpp"
  "Common/XUI/XUI_Ctrl_MinecraftSlot.h"
  "Common/XUI/XUI_Ctrl_MobEffect.cpp"
  "Common/XUI/XUI_Ctrl_MobEffect.h"
  "Common/XUI/XUI_Ctrl_PassThroughList.cpp"
  "Common/XUI/XUI_Ctrl_PassthroughList.h"
  "Common/XUI/XUI_Ctrl_ProgressCtrlBase.cpp"
  "Common/XUI/XUI_Ctrl_ProgressCtrlBase.h"
  "Common/XUI/XUI_Ctrl_SliderWrapper.cpp"
  "Common/XUI/XUI_Ctrl_SliderWrapper.h"
  "Common/XUI/XUI_Ctrl_SlotItem.h"
  "Common/XUI/XUI_Ctrl_SlotItemCtrlBase.cpp"
  "Common/XUI/XUI_Ctrl_SlotItemCtrlBase.h"
  "Common/XUI/XUI_Ctrl_SlotItemListItem.h"
  "Common/XUI/XUI_Ctrl_SlotList.cpp"
  "Common/XUI/XUI_Ctrl_SlotList.h"
  "Common/XUI/XUI_Ctrl_SplashPulser.cpp"
  "Common/XUI/XUI_Ctrl_SplashPulser.h"
)
source_group("Xbox/Source Files/XUI/Controls" FILES ${_SOURCES_XBOX_SOURCE_FILES_XUI_CONTROLS})

set(_SOURCES_XBOX_SOURCE_FILES_XUI_MENU_SCREENS
  "Common/XUI/XUI_ConnectingProgress.cpp"
  "Common/XUI/XUI_ConnectingProgress.h"
  "Common/XUI/XUI_DLCOffers.cpp"
  "Common/XUI/XUI_DLCOffers.h"
  "Common/XUI/XUI_Death.cpp"
  "Common/XUI/XUI_Death.h"
  "Common/XUI/XUI_FullscreenProgress.cpp"
  "Common/XUI/XUI_FullscreenProgress.h"
  "Common/XUI/XUI_Helper.h"
  "Common/XUI/XUI_InGameHostOptions.cpp"
  "Common/XUI/XUI_InGameHostOptions.h"
  "Common/XUI/XUI_InGameInfo.cpp"
  "Common/XUI/XUI_InGameInfo.h"
  "Common/XUI/XUI_InGamePlayerOptions.cpp"
  "Common/XUI/XUI_InGamePlayerOptions.h"
  "Common/XUI/XUI_Intro.cpp"
  "Common/XUI/XUI_Intro.h"
  "Common/XUI/XUI_LoadSettings.cpp"
  "Common/XUI/XUI_LoadSettings.h"
  "Common/XUI/XUI_MainMenu.cpp"
  "Common/XUI/XUI_MainMenu.h"
  "Common/XUI/XUI_MultiGameCreate.cpp"
  "Common/XUI/XUI_MultiGameCreate.h"
  "Common/XUI/XUI_MultiGameInfo.cpp"
  "Common/XUI/XUI_MultiGameInfo.h"
  "Common/XUI/XUI_MultiGameJoinLoad.cpp"
  "Common/XUI/XUI_MultiGameJoinLoad.h"
  "Common/XUI/XUI_MultiGameLaunchMoreOptions.cpp"
  "Common/XUI/XUI_MultiGameLaunchMoreOptions.h"
  "Common/XUI/XUI_NewUpdateMessage.cpp"
  "Common/XUI/XUI_NewUpdateMessage.h"
  "Common/XUI/XUI_PartnernetPassword.cpp"
  "Common/XUI/XUI_PartnernetPassword.h"
  "Common/XUI/XUI_SaveMessage.cpp"
  "Common/XUI/XUI_SaveMessage.h"
  "Common/XUI/XUI_Scene_Win.cpp"
  "Common/XUI/XUI_Scene_Win.h"
  "Common/XUI/XUI_SignEntry.cpp"
  "Common/XUI/XUI_SignEntry.h"
  "Common/XUI/XUI_Teleport.cpp"
  "Common/XUI/XUI_Teleport.h"
  "Common/XUI/XUI_TransferToXboxOne.cpp"
  "Common/XUI/XUI_TransferToXboxOne.h"
  "Common/XUI/XUI_TrialExitUpsell.cpp"
  "Common/XUI/XUI_TrialExitUpsell.h"
)
source_group("Xbox/Source Files/XUI/Menu screens" FILES ${_SOURCES_XBOX_SOURCE_FILES_XUI_MENU_SCREENS})

set(_SOURCES_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_DEBUG
  "Common/XUI/XUI_Debug.h"
  "Common/XUI/XUI_DebugItemEditor.cpp"
  "Common/XUI/XUI_DebugItemEditor.h"
  "Common/XUI/XUI_DebugOverlay.cpp"
  "Common/XUI/XUI_DebugOverlay.h"
  "Common/XUI/XUI_DebugSchematicCreator.cpp"
  "Common/XUI/XUI_DebugSchematicCreator.h"
  "Common/XUI/XUI_DebugSetCamera.cpp"
  "Common/XUI/XUI_DebugSetCamera.h"
  "Common/XUI/XUI_DebugTips.cpp"
  "Common/XUI/XUI_DebugTips.h"
  "Common/XUI/XUI_debug.cpp"
)
source_group("Xbox/Source Files/XUI/Menu screens/Debug" FILES ${_SOURCES_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_DEBUG})

set(_SOURCES_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_HELP__OPTIONS
  "Common/XUI/XUI_HelpAndOptions.cpp"
  "Common/XUI/XUI_HelpAndOptions.h"
)
source_group("Xbox/Source Files/XUI/Menu screens/Help & Options" FILES ${_SOURCES_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_HELP__OPTIONS})

set(_SOURCES_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_HELP__OPTIONS_CONTROLS
  "Common/XUI/XUI_HelpControls.cpp"
  "Common/XUI/XUI_HelpControls.h"
)
source_group("Xbox/Source Files/XUI/Menu screens/Help & Options/Controls" FILES ${_SOURCES_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_HELP__OPTIONS_CONTROLS})

set(_SOURCES_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_HELP__OPTIONS_CREDITS
  "Common/XUI/XUI_HelpCredits.cpp"
  "Common/XUI/XUI_HelpCredits.h"
)
source_group("Xbox/Source Files/XUI/Menu screens/Help & Options/Credits" FILES ${_SOURCES_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_HELP__OPTIONS_CREDITS})

set(_SOURCES_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_HELP__OPTIONS_HOW_TO_PLAY
  "Common/XUI/XUI_HelpHowToPlay.cpp"
  "Common/XUI/XUI_HelpHowToPlay.h"
  "Common/XUI/XUI_HowToPlayMenu.cpp"
  "Common/XUI/XUI_HowToPlayMenu.h"
)
source_group("Xbox/Source Files/XUI/Menu screens/Help & Options/How To Play" FILES ${_SOURCES_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_HELP__OPTIONS_HOW_TO_PLAY})

set(_SOURCES_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_HELP__OPTIONS_SETTINGS
  "Common/XUI/XUI_SettingsAll.cpp"
  "Common/XUI/XUI_SettingsAll.h"
  "Common/XUI/XUI_SettingsAudio.cpp"
  "Common/XUI/XUI_SettingsAudio.h"
  "Common/XUI/XUI_SettingsControl.cpp"
  "Common/XUI/XUI_SettingsControl.h"
  "Common/XUI/XUI_SettingsGraphics.cpp"
  "Common/XUI/XUI_SettingsGraphics.h"
  "Common/XUI/XUI_SettingsOptions.cpp"
  "Common/XUI/XUI_SettingsOptions.h"
  "Common/XUI/XUI_SettingsUI.cpp"
  "Common/XUI/XUI_SettingsUI.h"
  "Common/XUI/XUI_SkinSelect.cpp"
  "Common/XUI/XUI_SkinSelect.h"
)
source_group("Xbox/Source Files/XUI/Menu screens/Help & Options/Settings" FILES ${_SOURCES_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_HELP__OPTIONS_SETTINGS})

set(_SOURCES_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_LEADERBOARDS
  "Common/XUI/XUI_Leaderboards.cpp"
  "Common/XUI/XUI_Leaderboards.h"
)
source_group("Xbox/Source Files/XUI/Menu screens/Leaderboards" FILES ${_SOURCES_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_LEADERBOARDS})

set(_SOURCES_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_PAUSE
  "Common/XUI/XUI_PauseMenu.cpp"
  "Common/XUI/XUI_PauseMenu.h"
)
source_group("Xbox/Source Files/XUI/Menu screens/Pause" FILES ${_SOURCES_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_PAUSE})

set(_SOURCES_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_SOCIAL
  "Common/XUI/XUI_SocialPost.cpp"
  "Common/XUI/XUI_SocialPost.h"
)
source_group("Xbox/Source Files/XUI/Menu screens/Social" FILES ${_SOURCES_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_SOCIAL})

set(_SOURCES_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_TUTORIAL
  "Common/XUI/XUI_TutorialPopup.cpp"
  "Common/XUI/XUI_TutorialPopup.h"
)
source_group("Xbox/Source Files/XUI/Menu screens/Tutorial" FILES ${_SOURCES_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_TUTORIAL})

set(_SOURCES_XBOX_XML
  "Xbox/XML/ATGXmlParser.cpp"
  "Xbox/XML/ATGXmlParser.h"
  "Xbox/XML/xmlFilesCallback.h"
)
source_group("Xbox/XML" FILES ${_SOURCES_XBOX_XML})

set(_SOURCES_XBOX_RES
  # "Xbox/res/terrain_MipmapLevel2.bmp"
  # "Xbox/res/terrain_MipmapLevel3.bmp"
)
source_group("Xbox/res" FILES ${_SOURCES_XBOX_RES})

set(_SOURCES_XBOX_RES_AUDIO
  # "Xbox/res/audio/minecraft.xsb"
  # "Xbox/res/audio/resident.xwb"
  # "Xbox/res/audio/streamed.xwb"
)
source_group("Xbox/res/audio" FILES ${_SOURCES_XBOX_RES_AUDIO})

set(_SOURCES_XBOX_XEXXML
  "Xbox/xex-dev.xml"
  "Xbox/xex.xml"
)
source_group("Xbox/xexxml" FILES ${_SOURCES_XBOX_XEXXML})

set(_SOURCES_NET_MINECRAFT_CLIENT
  "Camera.cpp"
  "Camera.h"
  "ClientConstants.cpp"
  "ClientConstants.h"
  "DemoUser.cpp"
  "DemoUser.h"
  "GuiMessage.cpp"
  "GuiMessage.h"
  "KeyMapping.cpp"
  "KeyMapping.h"
  "Lighting.cpp"
  "Lighting.h"
  "MemoryTracker.cpp"
  "MemoryTracker.h"
  "Minecraft.cpp"
  "Minecraft.h"
  "Options.cpp"
  "Options.h"
  "ProgressRenderer.cpp"
  "ProgressRenderer.h"
  "Timer.cpp"
  "Timer.h"
  "User.cpp"
  "User.h"
)
source_group("net/minecraft/client" FILES ${_SOURCES_NET_MINECRAFT_CLIENT})

set(_SOURCES_NET_GUI
  "Button.cpp"
  "Button.h"
  "ChatScreen.cpp"
  "ChatScreen.h"
  "ConfirmScreen.cpp"
  "ConfirmScreen.h"
  "ControlsScreen.cpp"
  "ControlsScreen.h"
  "CreateWorldScreen.cpp"
  "CreateWorldScreen.h"
  "DeathScreen.cpp"
  "DeathScreen.h"
  "EditBox.cpp"
  "EditBox.h"
  "ErrorScreen.cpp"
  "ErrorScreen.h"
  "Font.cpp"
  "Font.h"
  "Gui.cpp"
  "Gui.h"
  "GuiComponent.cpp"
  "GuiComponent.h"
  "InBedChatScreen.cpp"
  "InBedChatScreen.h"
  "JoinMultiplayerScreen.cpp"
  "JoinMultiplayerScreen.h"
  "Minimap.cpp"
  "Minimap.h"
  "NameEntryScreen.cpp"
  "NameEntryScreen.h"
  "OptionsScreen.cpp"
  "OptionsScreen.h"
  "PauseScreen.cpp"
  "PauseScreen.h"
  "RenameWorldScreen.cpp"
  "RenameWorldScreen.h"
  "Screen.cpp"
  "Screen.h"
  "ScreenSizeCalculator.cpp"
  "ScreenSizeCalculator.h"
  "ScrolledSelectionList.cpp"
  "ScrolledSelectionList.h"
  "SelectWorldScreen.cpp"
  "SelectWorldScreen.h"
  "SlideButton.cpp"
  "SlideButton.h"
  "SmallButton.cpp"
  "SmallButton.h"
  "VideoSettingsScreen.cpp"
  "VideoSettingsScreen.h"
)
source_group("net/minecraft/client/gui" FILES ${_SOURCES_NET_GUI})

set(_SOURCES_NET_GUI_ACHIEVEMENT
  "AchievementPopup.cpp"
  "AchievementPopup.h"
  "AchievementScreen.cpp"
  "AchievementScreen.h"
  "StatsScreen.cpp"
  "StatsScreen.h"
)
source_group("net/minecraft/client/gui/achievement" FILES ${_SOURCES_NET_GUI_ACHIEVEMENT})

set(_SOURCES_NET_GUI_INVENTORY
  "AbstractContainerScreen.cpp"
  "AbstractContainerScreen.h"
  "ContainerScreen.cpp"
  "ContainerScreen.h"
  "CraftingScreen.cpp"
  "CraftingScreen.h"
  "FurnaceScreen.cpp"
  "FurnaceScreen.h"
  "InventoryScreen.cpp"
  "InventoryScreen.h"
  "TextEditScreen.cpp"
  "TextEditScreen.h"
  "TrapScreen.cpp"
  "TrapScreen.h"
)
source_group("net/minecraft/client/gui/inventory" FILES ${_SOURCES_NET_GUI_INVENTORY})

set(_SOURCES_NET_GUI_PARTICLE
  "GuiParticle.cpp"
  "GuiParticle.h"
  "GuiParticles.cpp"
  "GuiParticles.h"
)
source_group("net/minecraft/client/gui/particle" FILES ${_SOURCES_NET_GUI_PARTICLE})

set(_SOURCES_NET_LEVEL
  "DemoLevel.cpp"
  "DemoLevel.h"
)
source_group("net/minecraft/client/level" FILES ${_SOURCES_NET_LEVEL})

set(_SOURCES_NET_MODEL
  "BatModel.cpp"
  "BatModel.h"
  "BlazeModel.cpp"
  "BlazeModel.h"
  "BoatModel.cpp"
  "BoatModel.h"
  "BookModel.cpp"
  "BookModel.h"
  "ChestModel.cpp"
  "ChestModel.h"
  "ChickenModel.cpp"
  "ChickenModel.h"
  "CowModel.cpp"
  "CowModel.h"
  "CreeperModel.cpp"
  "CreeperModel.h"
  "EndermanModel.cpp"
  "EndermanModel.h"
  "GhastModel.cpp"
  "GhastModel.h"
  "HumanoidModel.cpp"
  "HumanoidModel.h"
  "LargeChestModel.cpp"
  "LargeChestModel.h"
  "LavaSlimeModel.cpp"
  "LavaSlimeModel.h"
  "LeashKnotModel.cpp"
  "LeashKnotModel.h"
  "MinecartModel.cpp"
  "MinecartModel.h"
  "ModelHorse.cpp"
  "ModelHorse.h"
  "OcelotModel.cpp"
  "OcelotModel.h"
  "PigModel.cpp"
  "PigModel.h"
  "Polygon.cpp"
  "Polygon.h"
  "QuadrupedModel.cpp"
  "QuadrupedModel.h"
  "SheepFurModel.cpp"
  "SheepFurModel.h"
  "SheepModel.cpp"
  "SheepModel.h"
  "SignModel.cpp"
  "SignModel.h"
  "SilverfishModel.cpp"
  "SilverfishModel.h"
  "SkeletonHeadModel.cpp"
  "SkeletonHeadModel.h"
  "SkeletonModel.cpp"
  "SkeletonModel.h"
  "SkiModel.cpp"
  "SkiModel.h"
  "SlimeModel.cpp"
  "SlimeModel.h"
  "SnowManModel.cpp"
  "SnowManModel.h"
  "SpiderModel.cpp"
  "SpiderModel.h"
  "SquidModel.cpp"
  "SquidModel.h"
  "Vertex.cpp"
  "Vertex.h"
  "VillagerGolemModel.cpp"
  "VillagerGolemModel.h"
  "VillagerModel.cpp"
  "VillagerModel.h"
  "VillagerZombieModel.cpp"
  "VillagerZombieModel.h"
  "WitchModel.cpp"
  "WitchModel.h"
  "WitherBossModel.cpp"
  "WitherBossModel.h"
  "WolfModel.cpp"
  "WolfModel.h"
  "ZombieModel.cpp"
  "ZombieModel.h"
)
source_group("net/minecraft/client/model" FILES ${_SOURCES_NET_MODEL})

set(_SOURCES_NET_MODEL_DRAGON
  "DragonModel.cpp"
  "DragonModel.h"
  "EnderCrystalModel.cpp"
  "EnderCrystalModel.h"
)
source_group("net/minecraft/client/model/dragon" FILES ${_SOURCES_NET_MODEL_DRAGON})

set(_SOURCES_NET_MODEL_GEOM
  "Cube.cpp"
  "Cube.h"
  "Model.cpp"
  "Model.h"
  "ModelPart.cpp"
  "ModelPart.h"
  "TexOffs.cpp"
  "TexOffs.h"
)
source_group("net/minecraft/client/model/geom" FILES ${_SOURCES_NET_MODEL_GEOM})

set(_SOURCES_NET_MULTIPLAYER
  "ClientConnection.cpp"
  "ClientConnection.h"
  "ConnectScreen.cpp"
  "ConnectScreen.h"
  "DisconnectedScreen.cpp"
  "DisconnectedScreen.h"
  "MultiPlayerChunkCache.cpp"
  "MultiPlayerChunkCache.h"
  "MultiPlayerGameMode.cpp"
  "MultiPlayerGameMode.h"
  "MultiPlayerLevel.cpp"
  "MultiPlayerLevel.h"
  "MultiPlayerLocalPlayer.cpp"
  "MultiPlayerLocalPlayer.h"
  "PlayerInfo.h"
  "ReceivingLevelScreen.cpp"
  "ReceivingLevelScreen.h"
)
source_group("net/minecraft/client/multiplayer" FILES ${_SOURCES_NET_MULTIPLAYER})

set(_SOURCES_NET_PARTICLE
  "BreakingItemParticle.cpp"
  "BreakingItemParticle.h"
  "BubbleParticle.cpp"
  "BubbleParticle.h"
  "CritParticle.cpp"
  "CritParticle.h"
  "CritParticle2.cpp"
  "CritParticle2.h"
  "DragonBreathParticle.cpp"
  "DragonBreathParticle.h"
  "DripParticle.cpp"
  "DripParticle.h"
  "EchantmentTableParticle.cpp"
  "EchantmentTableParticle.h"
  "EnderParticle.cpp"
  "EnderParticle.h"
  "ExplodeParticle.cpp"
  "ExplodeParticle.h"
  "FireworksParticles.cpp"
  "FireworksParticles.h"
  "FlameParticle.cpp"
  "FlameParticle.h"
  "FootstepParticle.cpp"
  "FootstepParticle.h"
  "HeartParticle.cpp"
  "HeartParticle.h"
  "HugeExplosionParticle.cpp"
  "HugeExplosionParticle.h"
  "HugeExplosionSeedParticle.cpp"
  "HugeExplosionSeedParticle.h"
  "LavaParticle.cpp"
  "LavaParticle.h"
  "NetherPortalParticle.cpp"
  "NetherPortalParticle.h"
  "NoteParticle.cpp"
  "NoteParticle.h"
  "Particle.cpp"
  "Particle.h"
  "ParticleEngine.cpp"
  "ParticleEngine.h"
  "PlayerCloudParticle.cpp"
  "PlayerCloudParticle.h"
  "RedDustParticle.cpp"
  "RedDustParticle.h"
  "SmokeParticle.cpp"
  "SmokeParticle.h"
  "SnowShovelParticle.cpp"
  "SnowShovelParticle.h"
  "SpellParticle.cpp"
  "SpellParticle.h"
  "SplashParticle.cpp"
  "SplashParticle.h"
  "SuspendedParticle.cpp"
  "SuspendedParticle.h"
  "SuspendedTownParticle.cpp"
  "SuspendedTownParticle.h"
  "TakeAnimationParticle.cpp"
  "TakeAnimationParticle.h"
  "TerrainParticle.cpp"
  "TerrainParticle.h"
  "WaterDropParticle.cpp"
  "WaterDropParticle.h"
)
source_group("net/minecraft/client/particle" FILES ${_SOURCES_NET_PARTICLE})

set(_SOURCES_NET_PLAYER
  "Input.cpp"
  "Input.h"
  "LocalPlayer.cpp"
  "LocalPlayer.h"
  "RemotePlayer.cpp"
  "RemotePlayer.h"
)
source_group("net/minecraft/client/player" FILES ${_SOURCES_NET_PLAYER})

set(_SOURCES_NET_RENDERER
  "BossMobGuiInfo.cpp"
  "BossMobGuiInfo.h"
  "Chunk.cpp"
  "Chunk.h"
  "DirtyChunkSorter.cpp"
  "DirtyChunkSorter.h"
  "DistanceChunkSorter.cpp"
  "DistanceChunkSorter.h"
  "EntityTileRenderer.cpp"
  "EntityTileRenderer.h"
  "GameRenderer.cpp"
  "GameRenderer.h"
  "HttpTexture.cpp"
  "HttpTexture.h"
  "HttpTextureProcessor.h"
  "ItemInHandRenderer.cpp"
  "ItemInHandRenderer.h"
  "LevelRenderer.cpp"
  "LevelRenderer.h"
  "MemTexture.cpp"
  "MobSkinMemTextureProcessor.cpp"
  "MobSkinTextureProcessor.cpp"
  "MobSkinTextureProcessor.h"
  "OffsettedRenderList.cpp"
  "OffsettedRenderList.h"
  "Rect2i.cpp"
  "Rect2i.h"
  "Tesselator.cpp"
  "Tesselator.h"
  "Textures.cpp"
  "Textures.h"
  "TileRenderer.cpp"
  "TileRenderer.h"
)
source_group("net/minecraft/client/renderer" FILES ${_SOURCES_NET_RENDERER})

set(_SOURCES_NET_RENDERER_CULLING
  "AllowAllCuller.cpp"
  "AllowAllCuller.h"
  "Culler.h"
  "Frustum.cpp"
  "Frustum.h"
  "FrustumCuller.cpp"
  "FrustumCuller.h"
  "FrustumData.cpp"
  "FrustumData.h"
  "ViewportCuller.cpp"
  "ViewportCuller.h"
)
source_group("net/minecraft/client/renderer/culling" FILES ${_SOURCES_NET_RENDERER_CULLING})

set(_SOURCES_NET_RENDERER_ENTITY
  "ArrowRenderer.cpp"
  "ArrowRenderer.h"
  "BatRenderer.cpp"
  "BatRenderer.h"
  "BlazeRenderer.cpp"
  "BlazeRenderer.h"
  "BoatRenderer.cpp"
  "BoatRenderer.h"
  "CaveSpiderRenderer.cpp"
  "CaveSpiderRenderer.h"
  "ChickenRenderer.cpp"
  "ChickenRenderer.h"
  "CowRenderer.cpp"
  "CowRenderer.h"
  "CreeperRenderer.cpp"
  "CreeperRenderer.h"
  "DefaultRenderer.cpp"
  "DefaultRenderer.h"
  "EnderCrystalRenderer.cpp"
  "EnderCrystalRenderer.h"
  "EnderDragonRenderer.cpp"
  "EnderDragonRenderer.h"
  "EndermanRenderer.cpp"
  "EndermanRenderer.h"
  "EntityRenderDispatcher.cpp"
  "EntityRenderDispatcher.h"
  "EntityRenderer.cpp"
  "EntityRenderer.h"
  "ExperienceOrbRenderer.cpp"
  "ExperienceOrbRenderer.h"
  "FallingTileRenderer.cpp"
  "FallingTileRenderer.h"
  "FireballRenderer.cpp"
  "FireballRenderer.h"
  "FishingHookRenderer.cpp"
  "FishingHookRenderer.h"
  "GhastRenderer.cpp"
  "GhastRenderer.h"
  "GiantMobRenderer.cpp"
  "GiantMobRenderer.h"
  "HorseRenderer.cpp"
  "HorseRenderer.h"
  "HumanoidMobRenderer.cpp"
  "HumanoidMobRenderer.h"
  "ItemFrameRenderer.cpp"
  "ItemFrameRenderer.h"
  "ItemRenderer.cpp"
  "ItemRenderer.h"
  "ItemSpriteRenderer.cpp"
  "ItemSpriteRenderer.h"
  "LavaSlimeRenderer.cpp"
  "LavaSlimeRenderer.h"
  "LeashKnotRenderer.cpp"
  "LeashKnotRenderer.h"
  "LightningBoltRenderer.cpp"
  "LightningBoltRenderer.h"
  "LivingEntityRenderer.cpp"
  "LivingEntityRenderer.h"
  "MinecartRenderer.cpp"
  "MinecartRenderer.h"
  "MinecartSpawnerRenderer.cpp"
  "MinecartSpawnerRenderer.h"
  "MobRenderer.cpp"
  "MobRenderer.h"
  "MushroomCowRenderer.cpp"
  "MushroomCowRenderer.h"
  "OcelotRenderer.cpp"
  "OcelotRenderer.h"
  "PaintingRenderer.cpp"
  "PaintingRenderer.h"
  "PigRenderer.cpp"
  "PigRenderer.h"
  "PlayerRenderer.cpp"
  "PlayerRenderer.h"
  "SheepRenderer.cpp"
  "SheepRenderer.h"
  "SilverfishRenderer.cpp"
  "SilverfishRenderer.h"
  "SkeletonRenderer.cpp"
  "SkeletonRenderer.h"
  "SlimeRenderer.cpp"
  "SlimeRenderer.h"
  "SnowManRenderer.cpp"
  "SnowManRenderer.h"
  "SpiderRenderer.cpp"
  "SpiderRenderer.h"
  "SquidRenderer.cpp"
  "SquidRenderer.h"
  "TntMinecartRenderer.cpp"
  "TntMinecartRenderer.h"
  "TntRenderer.cpp"
  "TntRenderer.h"
  "VillagerGolemRenderer.cpp"
  "VillagerGolemRenderer.h"
  "VillagerRenderer.cpp"
  "VillagerRenderer.h"
  "WitchRenderer.cpp"
  "WitchRenderer.h"
  "WitherBossRenderer.cpp"
  "WitherBossRenderer.h"
  "WitherSkullRenderer.cpp"
  "WitherSkullRenderer.h"
  "WolfRenderer.cpp"
  "WolfRenderer.h"
  "ZombieRenderer.cpp"
  "ZombieRenderer.h"
)
source_group("net/minecraft/client/renderer/entity" FILES ${_SOURCES_NET_RENDERER_ENTITY})

set(_SOURCES_NET_RENDERER_TEXTURE
  "PreStitchedTextureMap.cpp"
  "PreStitchedTextureMap.h"
  "SimpleIcon.cpp"
  "SimpleIcon.h"
  "StitchSlot.cpp"
  "StitchSlot.h"
  "StitchedTexture.cpp"
  "StitchedTexture.h"
  "Stitcher.cpp"
  "Stitcher.h"
  "Texture.cpp"
  "Texture.h"
  "TextureAtlas.cpp"
  "TextureAtlas.h"
  "TextureHolder.cpp"
  "TextureHolder.h"
  "TextureManager.cpp"
  "TextureManager.h"
  "TextureMap.cpp"
  "TextureMap.h"
)
source_group("net/minecraft/client/renderer/texture" FILES ${_SOURCES_NET_RENDERER_TEXTURE})

set(_SOURCES_NET_RENDERER_TEXTURE_CUSTOM
  "ClockTexture.cpp"
  "ClockTexture.h"
  "CompassTexture.cpp"
  "CompassTexture.h"
)
source_group("net/minecraft/client/renderer/texture/custom" FILES ${_SOURCES_NET_RENDERER_TEXTURE_CUSTOM})

set(_SOURCES_NET_RENDERER_TILEENTITY
  "BeaconRenderer.cpp"
  "BeaconRenderer.h"
  "ChestRenderer.cpp"
  "ChestRenderer.h"
  "EnchantTableRenderer.cpp"
  "EnchantTableRenderer.h"
  "EnderChestRenderer.cpp"
  "EnderChestRenderer.h"
  "MobSpawnerRenderer.cpp"
  "MobSpawnerRenderer.h"
  "PistonPieceRenderer.cpp"
  "PistonPieceRenderer.h"
  "SignRenderer.cpp"
  "SignRenderer.h"
  "SkullTileRenderer.cpp"
  "SkullTileRenderer.h"
  "TheEndPortalRenderer.cpp"
  "TheEndPortalRenderer.h"
  "TileEntityRenderDispatcher.cpp"
  "TileEntityRenderDispatcher.h"
  "TileEntityRenderer.cpp"
  "TileEntityRenderer.h"
)
source_group("net/minecraft/client/renderer/tileentity" FILES ${_SOURCES_NET_RENDERER_TILEENTITY})

set(_SOURCES_NET_RESOURCES
  "ResourceLocation.h"
)
source_group("net/minecraft/client/resources" FILES ${_SOURCES_NET_RESOURCES})

set(_SOURCES_NET_SKINS
  "AbstractTexturePack.cpp"
  "AbstractTexturePack.h"
  "DLCTexturePack.cpp"
  "DLCTexturePack.h"
  "DefaultTexturePack.cpp"
  "DefaultTexturePack.h"
  "FileTexturePack.cpp"
  "FileTexturePack.h"
  "FolderTexturePack.cpp"
  "FolderTexturePack.h"
  "TexturePack.cpp"
  "TexturePack.h"
  "TexturePackRepository.cpp"
  "TexturePackRepository.h"
)
source_group("net/minecraft/client/skins" FILES ${_SOURCES_NET_SKINS})

set(_SOURCES_NET_TITLE
  "TitleScreen.cpp"
  "TitleScreen.h"
)
source_group("net/minecraft/client/title" FILES ${_SOURCES_NET_TITLE})

set(_SOURCES_NET_MINECRAFT_SERVER
  "ConsoleInput.cpp"
  "ConsoleInput.h"
  "ConsoleInputSource.h"
  "DispenserBootstrap.cpp"
  "DispenserBootstrap.h"
  "MinecraftServer.cpp"
  "MinecraftServer.h"
  "PlayerList.cpp"
  "PlayerList.h"
  "ServerInterface.h"
  "ServerScoreboard.cpp"
  "ServerScoreboard.h"
  "Settings.cpp"
  "Settings.h"
)
source_group("net/minecraft/server" FILES ${_SOURCES_NET_MINECRAFT_SERVER})

set(_SOURCES_NET_MINECRAFT_SERVER_COMMANDS
  "ServerCommandDispatcher.cpp"
  "ServerCommandDispatcher.h"
  "TeleportCommand.cpp"
  "TeleportCommand.h"
)
source_group("net/minecraft/server/commands" FILES ${_SOURCES_NET_MINECRAFT_SERVER_COMMANDS})

set(_SOURCES_NET_MINECRAFT_SERVER_LEVEL
  "DerivedServerLevel.cpp"
  "DerivedServerLevel.h"
  "EntityTracker.cpp"
  "EntityTracker.h"
  "PlayerChunkMap.cpp"
  "PlayerChunkMap.h"
  "ServerChunkCache.cpp"
  "ServerChunkCache.h"
  "ServerLevel.cpp"
  "ServerLevel.h"
  "ServerLevelListener.cpp"
  "ServerLevelListener.h"
  "ServerPlayer.cpp"
  "ServerPlayer.h"
  "ServerPlayerGameMode.cpp"
  "ServerPlayerGameMode.h"
  "TrackedEntity.cpp"
  "TrackedEntity.h"
)
source_group("net/minecraft/server/level" FILES ${_SOURCES_NET_MINECRAFT_SERVER_LEVEL})

set(_SOURCES_NET_MINECRAFT_SERVER_NETWORK
  "PendingConnection.cpp"
  "PendingConnection.h"
  "PlayerConnection.cpp"
  "PlayerConnection.h"
  "ServerConnection.cpp"
  "ServerConnection.h"
)
source_group("net/minecraft/server/network" FILES ${_SOURCES_NET_MINECRAFT_SERVER_NETWORK})

set(_SOURCES_NET_MINECRAFT_STATS
  "StatsCounter.cpp"
  "StatsCounter.h"
  "StatsSyncher.cpp"
  "StatsSyncher.h"
)
source_group("net/minecraft/stats" FILES ${_SOURCES_NET_MINECRAFT_STATS})

set(MINECRAFT_CLIENT_SOURCES
  ${_SOURCES_ROOT}
  ${_SOURCES_COMMON}
  ${_SOURCES_COMMON_SOURCE_FILES}
  ${_SOURCES_COMMON_SOURCE_FILES_AUDIO}
  ${_SOURCES_COMMON_SOURCE_FILES_BUILDVER}
  ${_SOURCES_COMMON_SOURCE_FILES_COLOURS}
  ${_SOURCES_COMMON_SOURCE_FILES_DLC}
  ${_SOURCES_COMMON_SOURCE_FILES_FILESYSTEM}
  ${_SOURCES_COMMON_SOURCE_FILES_GAMERULES}
  ${_SOURCES_COMMON_SOURCE_FILES_GAMERULES_LEVELGENERATION}
  ${_SOURCES_COMMON_SOURCE_FILES_GAMERULES_LEVELGENERATION_STRUCTUREACTIONS}
  ${_SOURCES_COMMON_SOURCE_FILES_GAMERULES_LEVELRULES}
  ${_SOURCES_COMMON_SOURCE_FILES_GAMERULES_LEVELRULES_RULEDEFINITIONS}
  ${_SOURCES_COMMON_SOURCE_FILES_GAMERULES_LEVELRULES_RULES}
  ${_SOURCES_COMMON_SOURCE_FILES_LEADERBOARDS}
  ${_SOURCES_COMMON_SOURCE_FILES_LOCALISATION}
  ${_SOURCES_COMMON_SOURCE_FILES_NETWORK}
  ${_SOURCES_COMMON_SOURCE_FILES_NETWORK_SONY}
  ${_SOURCES_COMMON_SOURCE_FILES_TELEMETRY}
  ${_SOURCES_COMMON_SOURCE_FILES_TRIAL}
  ${_SOURCES_COMMON_SOURCE_FILES_TUTORIAL}
  ${_SOURCES_COMMON_SOURCE_FILES_TUTORIAL_CONSTRAINTS}
  ${_SOURCES_COMMON_SOURCE_FILES_TUTORIAL_HINTS}
  ${_SOURCES_COMMON_SOURCE_FILES_TUTORIAL_TASKS}
  ${_SOURCES_COMMON_SOURCE_FILES_UI}
  ${_SOURCES_COMMON_SOURCE_FILES_UI_ALL_PLATFORMS}
  ${_SOURCES_COMMON_SOURCE_FILES_UI_COMPONENTS}
  ${_SOURCES_COMMON_SOURCE_FILES_UI_CONTROLS}
  ${_SOURCES_COMMON_SOURCE_FILES_UI_SCENES}
  ${_SOURCES_COMMON_SOURCE_FILES_UI_SCENES_DEBUG}
  ${_SOURCES_COMMON_SOURCE_FILES_UI_SCENES_FRONTEND_MENU_SCREENS}
  ${_SOURCES_COMMON_SOURCE_FILES_UI_SCENES_HELP__OPTIONS}
  ${_SOURCES_COMMON_SOURCE_FILES_UI_SCENES_INGAME_MENU_SCREENS}
  ${_SOURCES_COMMON_SOURCE_FILES_UI_SCENES_INGAME_MENU_SCREENS_CONTAINERS}
  ${_SOURCES_COMMON_SOURCE_FILES_ZLIB}
  ${_SOURCES_DURANGO}
  ${_SOURCES_DURANGO_4JLIBS_INC}
  ${_SOURCES_DURANGO_DURANGOEXTRAS}
  ${_SOURCES_DURANGO_IGGY_GDRAW}
  ${_SOURCES_DURANGO_IGGY_INCLUDE}
  ${_SOURCES_DURANGO_NETWORK}
  ${_SOURCES_DURANGO_SERVICECONFIG}
  ${_SOURCES_DURANGO_SOURCE_FILES}
  ${_SOURCES_DURANGO_SOURCE_FILES_ACHIEVEMENTS}
  ${_SOURCES_DURANGO_SOURCE_FILES_LEADERBOARDS}
  ${_SOURCES_DURANGO_SOURCE_FILES_SENTIENT}
  ${_SOURCES_DURANGO_SOURCE_FILES_SOCIAL}
  ${_SOURCES_DURANGO_XML}
  ${_SOURCES_HEADER_FILES}
  ${_SOURCES_ORBIS}
  ${_SOURCES_ORBIS_4JLIBS_INC}
  ${_SOURCES_ORBIS_4JLIBS_LIBS}
  ${_SOURCES_ORBIS_IGGY_GDRAW}
  ${_SOURCES_ORBIS_IGGY_INCLUDE}
  ${_SOURCES_ORBIS_MILES_SOUND_SYSTEM_LIB}
  ${_SOURCES_ORBIS_NETWORK}
  ${_SOURCES_ORBIS_ORBISEXTRAS}
  ${_SOURCES_ORBIS_SOURCE_FILES}
  ${_SOURCES_ORBIS_SOURCE_FILES_LEADERBOARDS}
  ${_SOURCES_ORBIS_SOURCE_FILES_SENTIENT}
  ${_SOURCES_ORBIS_SOURCE_FILES_SOCIAL}
  ${_SOURCES_ORBIS_XML}
  ${_SOURCES_PS3}
  ${_SOURCES_PS3_4JLIBS}
  ${_SOURCES_PS3_4JLIBS_INC}
  ${_SOURCES_PS3_CHUNKREBUILD_SPU}
  ${_SOURCES_PS3_COMPRESSEDTILE_SPU}
  ${_SOURCES_PS3_IGGY_GDRAW}
  ${_SOURCES_PS3_IGGY_INCLUDE}
  ${_SOURCES_PS3_MILES_SOUND_SYSTEM_LIB}
  ${_SOURCES_PS3_MILES_SOUND_SYSTEM_LIB_SPU}
  ${_SOURCES_PS3_PS3EXTRAS}
  ${_SOURCES_PS3_SOURCE_FILES}
  ${_SOURCES_PS3_SOURCE_FILES_AUDIO}
  ${_SOURCES_PS3_SOURCE_FILES_LEADERBOARDS}
  ${_SOURCES_PS3_SOURCE_FILES_NETWORK}
  ${_SOURCES_PS3_SOURCE_FILES_SENTIENT}
  ${_SOURCES_PS3_SOURCE_FILES_SOCIAL}
  ${_SOURCES_PSVITA}
  ${_SOURCES_PSVITA_4JLIBS_INC}
  ${_SOURCES_PSVITA_GAMECONFIG}
  ${_SOURCES_PSVITA_IGGY_GDRAW}
  ${_SOURCES_PSVITA_IGGY_INCLUDE}
  ${_SOURCES_PSVITA_PSVITAEXTRAS}
  ${_SOURCES_PSVITA_SOURCE_FILES}
  ${_SOURCES_PSVITA_SOURCE_FILES_LEADERBOARDS}
  ${_SOURCES_PSVITA_SOURCE_FILES_NETWORK}
  ${_SOURCES_PSVITA_SOURCE_FILES_SENTIENT}
  ${_SOURCES_PSVITA_SOURCE_FILES_SOCIAL}
  ${_SOURCES_PSVITA_XML}
  ${_SOURCES_SOURCE_FILES}
  ${_SOURCES_WINDOWS}
  ${_SOURCES_WINDOWS64}
  ${_SOURCES_WINDOWS64_4JLIBS_INC}
  ${_SOURCES_WINDOWS64_GAMECONFIG}
  ${_SOURCES_WINDOWS64_IGGY_GDRAW}
  ${_SOURCES_WINDOWS64_IGGY_INCLUDE}
  ${_SOURCES_WINDOWS64_SOURCE_FILES}
  ${_SOURCES_WINDOWS64_SOURCE_FILES_LEADERBOARDS}
  ${_SOURCES_WINDOWS64_SOURCE_FILES_NETWORK}
  ${_SOURCES_WINDOWS64_SOURCE_FILES_SENTIENT}
  ${_SOURCES_WINDOWS64_SOURCE_FILES_SOCIAL}
  ${_SOURCES_WINDOWS64_XML}
  ${_SOURCES_XBOX_4JLIBS_MEDIA}
  ${_SOURCES_XBOX_4JLIBS_INC}
  ${_SOURCES_XBOX_GAMECONFIG}
  ${_SOURCES_XBOX_SENTIENTLIBS_INC}
  ${_SOURCES_XBOX_SOURCE_FILES}
  ${_SOURCES_XBOX_SOURCE_FILES_AUDIO}
  ${_SOURCES_XBOX_SOURCE_FILES_FONT}
  ${_SOURCES_XBOX_SOURCE_FILES_LEADERBOARDS}
  ${_SOURCES_XBOX_SOURCE_FILES_NETWORK}
  ${_SOURCES_XBOX_SOURCE_FILES_SENTIENT}
  ${_SOURCES_XBOX_SOURCE_FILES_SENTIENT_DYNAMICCONF}
  ${_SOURCES_XBOX_SOURCE_FILES_SENTIENT_TELEMETRY}
  ${_SOURCES_XBOX_SOURCE_FILES_SOCIAL}
  ${_SOURCES_XBOX_SOURCE_FILES_XUI}
  ${_SOURCES_XBOX_SOURCE_FILES_XUI_BASE_SCENE}
  ${_SOURCES_XBOX_SOURCE_FILES_XUI_CONTAINERS}
  ${_SOURCES_XBOX_SOURCE_FILES_XUI_CONTROLS}
  ${_SOURCES_XBOX_SOURCE_FILES_XUI_MENU_SCREENS}
  ${_SOURCES_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_DEBUG}
  ${_SOURCES_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_HELP__OPTIONS}
  ${_SOURCES_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_HELP__OPTIONS_CONTROLS}
  ${_SOURCES_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_HELP__OPTIONS_CREDITS}
  ${_SOURCES_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_HELP__OPTIONS_HOW_TO_PLAY}
  ${_SOURCES_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_HELP__OPTIONS_SETTINGS}
  ${_SOURCES_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_LEADERBOARDS}
  ${_SOURCES_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_PAUSE}
  ${_SOURCES_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_SOCIAL}
  ${_SOURCES_XBOX_SOURCE_FILES_XUI_MENU_SCREENS_TUTORIAL}
  ${_SOURCES_XBOX_XML}
  ${_SOURCES_XBOX_RES}
  ${_SOURCES_XBOX_RES_AUDIO}
  ${_SOURCES_XBOX_XEXXML}
  ${_SOURCES_NET_MINECRAFT_CLIENT}
  ${_SOURCES_NET_GUI}
  ${_SOURCES_NET_GUI_ACHIEVEMENT}
  ${_SOURCES_NET_GUI_INVENTORY}
  ${_SOURCES_NET_GUI_PARTICLE}
  ${_SOURCES_NET_LEVEL}
  ${_SOURCES_NET_MODEL}
  ${_SOURCES_NET_MODEL_DRAGON}
  ${_SOURCES_NET_MODEL_GEOM}
  ${_SOURCES_NET_MULTIPLAYER}
  ${_SOURCES_NET_PARTICLE}
  ${_SOURCES_NET_PLAYER}
  ${_SOURCES_NET_RENDERER}
  ${_SOURCES_NET_RENDERER_CULLING}
  ${_SOURCES_NET_RENDERER_ENTITY}
  ${_SOURCES_NET_RENDERER_TEXTURE}
  ${_SOURCES_NET_RENDERER_TEXTURE_CUSTOM}
  ${_SOURCES_NET_RENDERER_TILEENTITY}
  ${_SOURCES_NET_RESOURCES}
  ${_SOURCES_NET_SKINS}
  ${_SOURCES_NET_TITLE}
  ${_SOURCES_NET_MINECRAFT_SERVER}
  ${_SOURCES_NET_MINECRAFT_SERVER_COMMANDS}
  ${_SOURCES_NET_MINECRAFT_SERVER_LEVEL}
  ${_SOURCES_NET_MINECRAFT_SERVER_NETWORK}
  ${_SOURCES_NET_MINECRAFT_STATS}
  PARENT_SCOPE
)
