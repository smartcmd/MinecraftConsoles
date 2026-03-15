set(BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Common/")

set(_MINECRAFT_CLIENT_COMMON_ROOT
  "${CMAKE_CURRENT_SOURCE_DIR}/ClassDiagram.cd"
  "${CMAKE_CURRENT_SOURCE_DIR}/ReadMe.txt"
)
source_group("" FILES ${_MINECRAFT_CLIENT_COMMON_ROOT})

set(_MINECRAFT_CLIENT_COMMON_COMMON
  "${BASE_DIR}/App_Defines.h"
  "${BASE_DIR}/App_enums.h"
  "${BASE_DIR}/App_structs.h"
  "${BASE_DIR}/Consoles_App.cpp"
  "${BASE_DIR}/Consoles_App.h"
  "${BASE_DIR}/PostProcesser.h"
  "${BASE_DIR}/Potion_Macros.h"
)
source_group("Common" FILES ${_MINECRAFT_CLIENT_COMMON_COMMON})

set(_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES
  "${BASE_DIR}/ConsoleGameMode.cpp"
  "${BASE_DIR}/ConsoleGameMode.h"
  "${BASE_DIR}/Console_Awards_enum.h"
  "${BASE_DIR}/Console_Debug_enum.h"
  "${BASE_DIR}/Console_Utils.cpp"
)
source_group("Common/Source Files" FILES ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES})

set(_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_AUDIO
  "${BASE_DIR}/Audio/Consoles_SoundEngine.cpp"
  "${BASE_DIR}/Audio/Consoles_SoundEngine.h"
  "${BASE_DIR}/Audio/SoundEngine.h"
  "${BASE_DIR}/Audio/SoundNames.cpp"
  "${BASE_DIR}/Audio/miniaudio.h"
  "${BASE_DIR}/Audio/stb_vorbis.h"
)
source_group("Common/Source Files/Audio" FILES ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_AUDIO})

set(_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_COLOURS
  "${BASE_DIR}/Colours/ColourTable.cpp"
  "${BASE_DIR}/Colours/ColourTable.h"
)
source_group("Common/Source Files/Colours" FILES ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_COLOURS})

set(_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_DLC
  "${BASE_DIR}/DLC/DLCAudioFile.cpp"
  "${BASE_DIR}/DLC/DLCAudioFile.h"
  "${BASE_DIR}/DLC/DLCCapeFile.cpp"
  "${BASE_DIR}/DLC/DLCCapeFile.h"
  "${BASE_DIR}/DLC/DLCColourTableFile.cpp"
  "${BASE_DIR}/DLC/DLCColourTableFile.h"
  "${BASE_DIR}/DLC/DLCFile.cpp"
  "${BASE_DIR}/DLC/DLCFile.h"
  "${BASE_DIR}/DLC/DLCGameRules.h"
  "${BASE_DIR}/DLC/DLCGameRulesFile.cpp"
  "${BASE_DIR}/DLC/DLCGameRulesFile.h"
  "${BASE_DIR}/DLC/DLCGameRulesHeader.cpp"
  "${BASE_DIR}/DLC/DLCGameRulesHeader.h"
  "${BASE_DIR}/DLC/DLCLocalisationFile.cpp"
  "${BASE_DIR}/DLC/DLCLocalisationFile.h"
  "${BASE_DIR}/DLC/DLCManager.cpp"
  "${BASE_DIR}/DLC/DLCManager.h"
  "${BASE_DIR}/DLC/DLCPack.cpp"
  "${BASE_DIR}/DLC/DLCPack.h"
  "${BASE_DIR}/DLC/DLCSkinFile.cpp"
  "${BASE_DIR}/DLC/DLCSkinFile.h"
  "${BASE_DIR}/DLC/DLCTextureFile.cpp"
  "${BASE_DIR}/DLC/DLCTextureFile.h"
  "${BASE_DIR}/DLC/DLCUIDataFile.cpp"
  "${BASE_DIR}/DLC/DLCUIDataFile.h"
)
source_group("Common/Source Files/DLC" FILES ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_DLC})

set(_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_GAMERULES
  "${BASE_DIR}/GameRules/ConsoleGameRules.h"
  "${BASE_DIR}/GameRules/ConsoleGameRulesConstants.h"
  "${BASE_DIR}/GameRules/GameRuleManager.cpp"
  "${BASE_DIR}/GameRules/GameRuleManager.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/WstringLookup.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/WstringLookup.h"
)
source_group("Common/Source Files/GameRules" FILES ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_GAMERULES})

set(_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_GAMERULES_LEVELGENERATION
  "${BASE_DIR}/GameRules/ApplySchematicRuleDefinition.cpp"
  "${BASE_DIR}/GameRules/ApplySchematicRuleDefinition.h"
  "${BASE_DIR}/GameRules/BiomeOverride.cpp"
  "${BASE_DIR}/GameRules/BiomeOverride.h"
  "${BASE_DIR}/GameRules/ConsoleGenerateStructure.cpp"
  "${BASE_DIR}/GameRules/ConsoleGenerateStructure.h"
  "${BASE_DIR}/GameRules/ConsoleGenerateStructureAction.h"
  "${BASE_DIR}/GameRules/ConsoleSchematicFile.cpp"
  "${BASE_DIR}/GameRules/ConsoleSchematicFile.h"
  "${BASE_DIR}/GameRules/LevelGenerationOptions.cpp"
  "${BASE_DIR}/GameRules/LevelGenerationOptions.h"
  "${BASE_DIR}/GameRules/LevelGenerators.cpp"
  "${BASE_DIR}/GameRules/LevelGenerators.h"
  "${BASE_DIR}/GameRules/StartFeature.cpp"
  "${BASE_DIR}/GameRules/StartFeature.h"
)
source_group("Common/Source Files/GameRules/LevelGeneration" FILES ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_GAMERULES_LEVELGENERATION})

set(_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_GAMERULES_LEVELGENERATION_STRUCTUREACTIONS
  "${BASE_DIR}/GameRules/XboxStructureActionGenerateBox.cpp"
  "${BASE_DIR}/GameRules/XboxStructureActionGenerateBox.h"
  "${BASE_DIR}/GameRules/XboxStructureActionPlaceBlock.cpp"
  "${BASE_DIR}/GameRules/XboxStructureActionPlaceBlock.h"
  "${BASE_DIR}/GameRules/XboxStructureActionPlaceContainer.cpp"
  "${BASE_DIR}/GameRules/XboxStructureActionPlaceContainer.h"
  "${BASE_DIR}/GameRules/XboxStructureActionPlaceSpawner.cpp"
  "${BASE_DIR}/GameRules/XboxStructureActionPlaceSpawner.h"
)
source_group("Common/Source Files/GameRules/LevelGeneration/StructureActions" FILES ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_GAMERULES_LEVELGENERATION_STRUCTUREACTIONS})

set(_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_GAMERULES_LEVELRULES
  "${BASE_DIR}/GameRules/LevelRules.cpp"
  "${BASE_DIR}/GameRules/LevelRules.h"
)
source_group("Common/Source Files/GameRules/LevelRules" FILES ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_GAMERULES_LEVELRULES})

set(_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_GAMERULES_LEVELRULES_RULEDEFINITIONS
  "${BASE_DIR}/GameRules/AddEnchantmentRuleDefinition.cpp"
  "${BASE_DIR}/GameRules/AddEnchantmentRuleDefinition.h"
  "${BASE_DIR}/GameRules/AddItemRuleDefinition.cpp"
  "${BASE_DIR}/GameRules/AddItemRuleDefinition.h"
  "${BASE_DIR}/GameRules/CollectItemRuleDefinition.cpp"
  "${BASE_DIR}/GameRules/CollectItemRuleDefinition.h"
  "${BASE_DIR}/GameRules/CompleteAllRuleDefinition.cpp"
  "${BASE_DIR}/GameRules/CompleteAllRuleDefinition.h"
  "${BASE_DIR}/GameRules/CompoundGameRuleDefinition.cpp"
  "${BASE_DIR}/GameRules/CompoundGameRuleDefinition.h"
  "${BASE_DIR}/GameRules/GameRuleDefinition.cpp"
  "${BASE_DIR}/GameRules/GameRuleDefinition.h"
  "${BASE_DIR}/GameRules/LevelRuleset.cpp"
  "${BASE_DIR}/GameRules/LevelRuleset.h"
  "${BASE_DIR}/GameRules/NamedAreaRuleDefinition.cpp"
  "${BASE_DIR}/GameRules/NamedAreaRuleDefinition.h"
  "${BASE_DIR}/GameRules/UpdatePlayerRuleDefinition.cpp"
  "${BASE_DIR}/GameRules/UpdatePlayerRuleDefinition.h"
  "${BASE_DIR}/GameRules/UseTileRuleDefinition.cpp"
  "${BASE_DIR}/GameRules/UseTileRuleDefinition.h"
)
source_group("Common/Source Files/GameRules/LevelRules/RuleDefinitions" FILES ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_GAMERULES_LEVELRULES_RULEDEFINITIONS})

set(_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_GAMERULES_LEVELRULES_RULES
  "${BASE_DIR}/GameRules/GameRule.cpp"
  "${BASE_DIR}/GameRules/GameRule.h"
  "${BASE_DIR}/GameRules/GameRulesInstance.h"
)
source_group("Common/Source Files/GameRules/LevelRules/Rules" FILES ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_GAMERULES_LEVELRULES_RULES})

set(_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_LEADERBOARDS
  "${BASE_DIR}/Leaderboards/LeaderboardInterface.cpp"
  "${BASE_DIR}/Leaderboards/LeaderboardInterface.h"
  "${BASE_DIR}/Leaderboards/LeaderboardManager.cpp"
  "${BASE_DIR}/Leaderboards/LeaderboardManager.h"
)
source_group("Common/Source Files/Leaderboards" FILES ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_LEADERBOARDS})

set(_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_LOCALISATION
  "${CMAKE_CURRENT_SOURCE_DIR}/StringTable.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/StringTable.h"
)
source_group("Common/Source Files/Localisation" FILES ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_LOCALISATION})

set(_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_NETWORK
  "${BASE_DIR}/Network/GameNetworkManager.cpp"
  "${BASE_DIR}/Network/GameNetworkManager.h"
  "${BASE_DIR}/Network/NetworkPlayerInterface.h"
  "${BASE_DIR}/Network/PlatformNetworkManagerInterface.h"
  "${BASE_DIR}/Network/SessionInfo.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Network Implementation Notes.txt"
)
source_group("Common/Source Files/Network" FILES ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_NETWORK})

set(_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_TELEMETRY
  "${BASE_DIR}/Telemetry/TelemetryManager.cpp"
  "${BASE_DIR}/Telemetry/TelemetryManager.h"
)
source_group("Common/Source Files/Telemetry" FILES ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_TELEMETRY})

set(_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_TRIAL
  "${BASE_DIR}/Trial/TrialMode.cpp"
  "${BASE_DIR}/Trial/TrialMode.h"
)
source_group("Common/Source Files/Trial" FILES ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_TRIAL})

set(_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_TUTORIAL
  "${BASE_DIR}/Tutorial/FullTutorial.cpp"
  "${BASE_DIR}/Tutorial/FullTutorial.h"
  "${BASE_DIR}/Tutorial/FullTutorialMode.cpp"
  "${BASE_DIR}/Tutorial/FullTutorialMode.h"
  "${BASE_DIR}/Tutorial/Tutorial.cpp"
  "${BASE_DIR}/Tutorial/Tutorial.h"
  "${BASE_DIR}/Tutorial/TutorialEnum.h"
  "${BASE_DIR}/Tutorial/TutorialMessage.cpp"
  "${BASE_DIR}/Tutorial/TutorialMessage.h"
  "${BASE_DIR}/Tutorial/TutorialMode.cpp"
  "${BASE_DIR}/Tutorial/TutorialMode.h"
)
source_group("Common/Source Files/Tutorial" FILES ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_TUTORIAL})

set(_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_TUTORIAL_CONSTRAINTS
  "${BASE_DIR}/Tutorial/AreaConstraint.cpp"
  "${BASE_DIR}/Tutorial/AreaConstraint.h"
  "${BASE_DIR}/Tutorial/ChangeStateConstraint.cpp"
  "${BASE_DIR}/Tutorial/ChangeStateConstraint.h"
  "${BASE_DIR}/Tutorial/InputConstraint.cpp"
  "${BASE_DIR}/Tutorial/InputConstraint.h"
  "${BASE_DIR}/Tutorial/TutorialConstraint.h"
  "${BASE_DIR}/Tutorial/TutorialConstraints.h"
)
source_group("Common/Source Files/Tutorial/Constraints" FILES ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_TUTORIAL_CONSTRAINTS})

set(_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_TUTORIAL_HINTS
  "${BASE_DIR}/Tutorial/AreaHint.cpp"
  "${BASE_DIR}/Tutorial/AreaHint.h"
  "${BASE_DIR}/Tutorial/DiggerItemHint.cpp"
  "${BASE_DIR}/Tutorial/DiggerItemHint.h"
  "${BASE_DIR}/Tutorial/LookAtEntityHint.cpp"
  "${BASE_DIR}/Tutorial/LookAtEntityHint.h"
  "${BASE_DIR}/Tutorial/LookAtTileHint.cpp"
  "${BASE_DIR}/Tutorial/LookAtTileHint.h"
  "${BASE_DIR}/Tutorial/TakeItemHint.cpp"
  "${BASE_DIR}/Tutorial/TakeItemHint.h"
  "${BASE_DIR}/Tutorial/TutorialHint.cpp"
  "${BASE_DIR}/Tutorial/TutorialHint.h"
  "${BASE_DIR}/Tutorial/TutorialHints.h"
)
source_group("Common/Source Files/Tutorial/Hints" FILES ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_TUTORIAL_HINTS})

set(_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_TUTORIAL_TASKS
  "${BASE_DIR}/Tutorial/AreaTask.cpp"
  "${BASE_DIR}/Tutorial/AreaTask.h"
  "${BASE_DIR}/Tutorial/ChoiceTask.cpp"
  "${BASE_DIR}/Tutorial/ChoiceTask.h"
  "${BASE_DIR}/Tutorial/CompleteUsingItemTask.cpp"
  "${BASE_DIR}/Tutorial/CompleteUsingItemTask.h"
  "${BASE_DIR}/Tutorial/ControllerTask.cpp"
  "${BASE_DIR}/Tutorial/ControllerTask.h"
  "${BASE_DIR}/Tutorial/CraftTask.cpp"
  "${BASE_DIR}/Tutorial/CraftTask.h"
  "${BASE_DIR}/Tutorial/EffectChangedTask.cpp"
  "${BASE_DIR}/Tutorial/EffectChangedTask.h"
  "${BASE_DIR}/Tutorial/FullTutorialActiveTask.cpp"
  "${BASE_DIR}/Tutorial/FullTutorialActiveTask.h"
  "${BASE_DIR}/Tutorial/HorseChoiceTask.cpp"
  "${BASE_DIR}/Tutorial/HorseChoiceTask.h"
  "${BASE_DIR}/Tutorial/InfoTask.cpp"
  "${BASE_DIR}/Tutorial/InfoTask.h"
  "${BASE_DIR}/Tutorial/PickupTask.cpp"
  "${BASE_DIR}/Tutorial/PickupTask.h"
  "${BASE_DIR}/Tutorial/ProcedureCompoundTask.cpp"
  "${BASE_DIR}/Tutorial/ProcedureCompoundTask.h"
  "${BASE_DIR}/Tutorial/ProgressFlagTask.cpp"
  "${BASE_DIR}/Tutorial/ProgressFlagTask.h"
  "${BASE_DIR}/Tutorial/RideEntityTask.cpp"
  "${BASE_DIR}/Tutorial/RideEntityTask.h"
  "${BASE_DIR}/Tutorial/StatTask.cpp"
  "${BASE_DIR}/Tutorial/StatTask.h"
  "${BASE_DIR}/Tutorial/StateChangeTask.h"
  "${BASE_DIR}/Tutorial/TutorialTask.cpp"
  "${BASE_DIR}/Tutorial/TutorialTask.h"
  "${BASE_DIR}/Tutorial/TutorialTasks.h"
  "${BASE_DIR}/Tutorial/UseItemTask.cpp"
  "${BASE_DIR}/Tutorial/UseItemTask.h"
  "${BASE_DIR}/Tutorial/UseTileTask.cpp"
  "${BASE_DIR}/Tutorial/UseTileTask.h"
  "${BASE_DIR}/Tutorial/XuiCraftingTask.cpp"
  "${BASE_DIR}/Tutorial/XuiCraftingTask.h"
)
source_group("Common/Source Files/Tutorial/Tasks" FILES ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_TUTORIAL_TASKS})

set(_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_UI
  "${BASE_DIR}/UI/UIFontData.cpp"
  "${BASE_DIR}/UI/UIFontData.h"
  "${BASE_DIR}/UI/UIString.cpp"
  "${BASE_DIR}/UI/UIString.h"
)
source_group("Common/Source Files/UI" FILES ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_UI})

set(_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_UI_ALL_PLATFORMS
  "${CMAKE_CURRENT_SOURCE_DIR}/ArchiveFile.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ArchiveFile.h"
  "${BASE_DIR}/UI/IUIController.h"
  "${BASE_DIR}/UI/IUIScene_AbstractContainerMenu.cpp"
  "${BASE_DIR}/UI/IUIScene_AbstractContainerMenu.h"
  "${BASE_DIR}/UI/IUIScene_AnvilMenu.cpp"
  "${BASE_DIR}/UI/IUIScene_AnvilMenu.h"
  "${BASE_DIR}/UI/IUIScene_BeaconMenu.cpp"
  "${BASE_DIR}/UI/IUIScene_BeaconMenu.h"
  "${BASE_DIR}/UI/IUIScene_BrewingMenu.cpp"
  "${BASE_DIR}/UI/IUIScene_BrewingMenu.h"
  "${BASE_DIR}/UI/IUIScene_CommandBlockMenu.cpp"
  "${BASE_DIR}/UI/IUIScene_CommandBlockMenu.h"
  "${BASE_DIR}/UI/IUIScene_ContainerMenu.cpp"
  "${BASE_DIR}/UI/IUIScene_ContainerMenu.h"
  "${BASE_DIR}/UI/IUIScene_CraftingMenu.cpp"
  "${BASE_DIR}/UI/IUIScene_CraftingMenu.h"
  "${BASE_DIR}/UI/IUIScene_CreativeMenu.cpp"
  "${BASE_DIR}/UI/IUIScene_CreativeMenu.h"
  "${BASE_DIR}/UI/IUIScene_DispenserMenu.cpp"
  "${BASE_DIR}/UI/IUIScene_DispenserMenu.h"
  "${BASE_DIR}/UI/IUIScene_EnchantingMenu.cpp"
  "${BASE_DIR}/UI/IUIScene_EnchantingMenu.h"
  "${BASE_DIR}/UI/IUIScene_FireworksMenu.cpp"
  "${BASE_DIR}/UI/IUIScene_FireworksMenu.h"
  "${BASE_DIR}/UI/IUIScene_FurnaceMenu.cpp"
  "${BASE_DIR}/UI/IUIScene_FurnaceMenu.h"
  "${BASE_DIR}/UI/IUIScene_HUD.cpp"
  "${BASE_DIR}/UI/IUIScene_HUD.h"
  "${BASE_DIR}/UI/IUIScene_HopperMenu.cpp"
  "${BASE_DIR}/UI/IUIScene_HopperMenu.h"
  "${BASE_DIR}/UI/IUIScene_HorseInventoryMenu.cpp"
  "${BASE_DIR}/UI/IUIScene_HorseInventoryMenu.h"
  "${BASE_DIR}/UI/IUIScene_InventoryMenu.cpp"
  "${BASE_DIR}/UI/IUIScene_InventoryMenu.h"
  "${BASE_DIR}/UI/IUIScene_PauseMenu.cpp"
  "${BASE_DIR}/UI/IUIScene_PauseMenu.h"
  "${BASE_DIR}/UI/IUIScene_TradingMenu.cpp"
  "${BASE_DIR}/UI/IUIScene_TradingMenu.h"
  "${BASE_DIR}/UI/UIEnums.h"
  "${BASE_DIR}/UI/UIStructs.h"
)
source_group("Common/Source Files/UI/All Platforms" FILES ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_UI_ALL_PLATFORMS})

set(_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_UI_SCENES_FRONTEND_MENU_SCREENS
  "${BASE_DIR}/UI/IUIScene_StartGame.h"
)
source_group("Common/Source Files/UI/Scenes/Frontend Menu screens" FILES ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_UI_SCENES_FRONTEND_MENU_SCREENS})

set(_MINECRAFT_CLIENT_COMMON_DURANGO_SERVICECONFIG
  "${CMAKE_CURRENT_SOURCE_DIR}/Durango/ServiceConfig/Events-XBLA.8-149E11AEEvents.h"
)
source_group("Durango/ServiceConfig" FILES ${_MINECRAFT_CLIENT_COMMON_DURANGO_SERVICECONFIG})

set(_MINECRAFT_CLIENT_COMMON_DURANGO_SOURCE_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/Durango/PresenceIds.h"
)
source_group("Durango/Source Files" FILES ${_MINECRAFT_CLIENT_COMMON_DURANGO_SOURCE_FILES})

set(_MINECRAFT_CLIENT_COMMON_DURANGO_SOURCE_FILES_ACHIEVEMENTS
  "${CMAKE_CURRENT_SOURCE_DIR}/Durango/Achievements/AchievementManager.h"
)
source_group("Durango/Source Files/Achievements" FILES ${_MINECRAFT_CLIENT_COMMON_DURANGO_SOURCE_FILES_ACHIEVEMENTS})

set(_MINECRAFT_CLIENT_COMMON_HEADER_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/BufferedImage.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/MemTexture.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/MemTextureProcessor.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/MobSkinMemTextureProcessor.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/SkinBox.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/extraX64client.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/stdafx.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/stubs.h"
)
source_group("Header Files" FILES ${_MINECRAFT_CLIENT_COMMON_HEADER_FILES})

set(_MINECRAFT_CLIENT_COMMON_ORBIS_4JLIBS_INC
  "${CMAKE_CURRENT_SOURCE_DIR}/Orbis/4JLibs/inc/4J_Input.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Orbis/4JLibs/inc/4J_Profile.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Orbis/4JLibs/inc/4J_Render.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Orbis/4JLibs/inc/4J_Storage.h"
)
source_group("Orbis/4JLibs/inc" FILES ${_MINECRAFT_CLIENT_COMMON_ORBIS_4JLIBS_INC})

set(_MINECRAFT_CLIENT_COMMON_PSVITA
  "${CMAKE_CURRENT_SOURCE_DIR}/PSVita/PSVita_App.h"
)
source_group("PSVita" FILES ${_MINECRAFT_CLIENT_COMMON_PSVITA})

set(_MINECRAFT_CLIENT_COMMON_PSVITA_GAMECONFIG
  "${CMAKE_CURRENT_SOURCE_DIR}/PSVita/GameConfig/Minecraft.gameconfig"
  "${CMAKE_CURRENT_SOURCE_DIR}/PSVita/GameConfig/Minecraft.spa"
  "${CMAKE_CURRENT_SOURCE_DIR}/PSVita/GameConfig/Minecraft.spa.h"
)
source_group("PSVita/GameConfig" FILES ${_MINECRAFT_CLIENT_COMMON_PSVITA_GAMECONFIG})

set(_MINECRAFT_CLIENT_COMMON_PSVITA_MILES_SOUND_SYSTEM_INCLUDE
  # "${CMAKE_CURRENT_SOURCE_DIR}/PSVita/Miles/include/mss.h"
  # "${CMAKE_CURRENT_SOURCE_DIR}/PSVita/Miles/include/rrCore.h"
)
source_group("PSVita/Miles Sound System/Include" FILES ${_MINECRAFT_CLIENT_COMMON_PSVITA_MILES_SOUND_SYSTEM_INCLUDE})

set(_MINECRAFT_CLIENT_COMMON_PSVITA_SOURCE_FILES_SOCIAL
  "${CMAKE_CURRENT_SOURCE_DIR}/PSVita/Social/SocialManager.h"
)
source_group("PSVita/Source Files/Social" FILES ${_MINECRAFT_CLIENT_COMMON_PSVITA_SOURCE_FILES_SOCIAL})

set(_MINECRAFT_CLIENT_COMMON_PSVITA_XML
  "${CMAKE_CURRENT_SOURCE_DIR}/PSVita/XML/ATGXmlParser.h"
)
source_group("PSVita/XML" FILES ${_MINECRAFT_CLIENT_COMMON_PSVITA_XML})

set(_MINECRAFT_CLIENT_COMMON_SOURCE_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/BufferedImage.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/compat_shims.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/glWrapper.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/iob_shim.asm"
  "${CMAKE_CURRENT_SOURCE_DIR}/stdafx.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/stubs.cpp"
)
source_group("Source Files" FILES ${_MINECRAFT_CLIENT_COMMON_SOURCE_FILES})

set(_MINECRAFT_CLIENT_COMMON_WINDOWS64_IGGY_GDRAW
  "${CMAKE_CURRENT_SOURCE_DIR}/Windows64/Iggy/gdraw/gdraw_d3d10_shaders.inl"
  "${CMAKE_CURRENT_SOURCE_DIR}/Windows64/Iggy/gdraw/gdraw_d3d1x_shared.inl"
  "${CMAKE_CURRENT_SOURCE_DIR}/Windows64/Iggy/gdraw/gdraw_gl_shaders.inl"
  "${CMAKE_CURRENT_SOURCE_DIR}/Windows64/Iggy/gdraw/gdraw_gl_shared.inl"
  "${CMAKE_CURRENT_SOURCE_DIR}/Windows64/Iggy/gdraw/gdraw_shared.inl"
)
source_group("Windows64/Iggy/gdraw" FILES ${_MINECRAFT_CLIENT_COMMON_WINDOWS64_IGGY_GDRAW})

set(_MINECRAFT_CLIENT_COMMON_WINDOWS64_SOURCE_FILES_NETWORK
  "${CMAKE_CURRENT_SOURCE_DIR}/Windows64/Network/WinsockNetLayer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Windows64/Network/WinsockNetLayer.h"
)
source_group("Windows64/Source Files/Network" FILES ${_MINECRAFT_CLIENT_COMMON_WINDOWS64_SOURCE_FILES_NETWORK})

set(_MINECRAFT_CLIENT_COMMON_XBOX_4JLIBS_INC
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/4JLibs/inc/4J_xtms.h"
)
source_group("Xbox/4JLibs/inc" FILES ${_MINECRAFT_CLIENT_COMMON_XBOX_4JLIBS_INC})

set(_MINECRAFT_CLIENT_COMMON_XBOX_SOURCE_FILES_AUDIO
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/Audio/SoundEngine.h"
)
source_group("Xbox/Source Files/Audio" FILES ${_MINECRAFT_CLIENT_COMMON_XBOX_SOURCE_FILES_AUDIO})

set(_MINECRAFT_CLIENT_COMMON_XBOX_SOURCE_FILES_NETWORK
  "${CMAKE_CURRENT_SOURCE_DIR}/Xbox/Network/extra.h"
)
source_group("Xbox/Source Files/Network" FILES ${_MINECRAFT_CLIENT_COMMON_XBOX_SOURCE_FILES_NETWORK})

set(_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT
  "${CMAKE_CURRENT_SOURCE_DIR}/Camera.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Camera.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ClientConstants.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ClientConstants.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/DemoUser.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/DemoUser.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/GuiMessage.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/GuiMessage.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/KeyMapping.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/KeyMapping.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Lighting.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Lighting.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/MemoryTracker.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/MemoryTracker.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Options.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Options.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ProgressRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ProgressRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Timer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Timer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/User.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/User.h"
)
source_group("net/minecraft/client" FILES ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT})

set(_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_GUI
  "${CMAKE_CURRENT_SOURCE_DIR}/Button.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Button.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ChatScreen.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ChatScreen.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ConfirmScreen.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ConfirmScreen.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ControlsScreen.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ControlsScreen.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/CreateWorldScreen.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CreateWorldScreen.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/DeathScreen.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/DeathScreen.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/EditBox.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/EditBox.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ErrorScreen.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ErrorScreen.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Font.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Font.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Gui.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Gui.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/GuiComponent.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/GuiComponent.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/InBedChatScreen.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/InBedChatScreen.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/JoinMultiplayerScreen.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/JoinMultiplayerScreen.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minimap.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minimap.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/NameEntryScreen.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/NameEntryScreen.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/OptionsScreen.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/OptionsScreen.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/PauseScreen.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/PauseScreen.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/RenameWorldScreen.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/RenameWorldScreen.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Screen.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Screen.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ScreenSizeCalculator.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ScreenSizeCalculator.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ScrolledSelectionList.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ScrolledSelectionList.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/SelectWorldScreen.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/SelectWorldScreen.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/SlideButton.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/SlideButton.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/SmallButton.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/SmallButton.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/VideoSettingsScreen.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/VideoSettingsScreen.h"
)
source_group("net/minecraft/client/gui" FILES ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_GUI})

set(_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_GUI_ACHIEVEMENT
  "${CMAKE_CURRENT_SOURCE_DIR}/AchievementPopup.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/AchievementPopup.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/AchievementScreen.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/AchievementScreen.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/StatsScreen.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/StatsScreen.h"
)
source_group("net/minecraft/client/gui/achievement" FILES ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_GUI_ACHIEVEMENT})

set(_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_GUI_PARTICLE
  "${CMAKE_CURRENT_SOURCE_DIR}/GuiParticle.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/GuiParticle.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/GuiParticles.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/GuiParticles.h"
)
source_group("net/minecraft/client/gui/particle" FILES ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_GUI_PARTICLE})

set(_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_LEVEL
  "${CMAKE_CURRENT_SOURCE_DIR}/DemoLevel.h"
)
source_group("net/minecraft/client/level" FILES ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_LEVEL})

set(_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_MODEL
  "${CMAKE_CURRENT_SOURCE_DIR}/BatModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/BatModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/BlazeModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/BlazeModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/BoatModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/BoatModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/BookModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/BookModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ChestModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ChestModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ChickenModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ChickenModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/CowModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CowModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/CreeperModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CreeperModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/EndermanModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/EndermanModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/GhastModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/GhastModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/HumanoidModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/HumanoidModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/LargeChestModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/LargeChestModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/LavaSlimeModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/LavaSlimeModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/LeashKnotModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/LeashKnotModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/MinecartModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/MinecartModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ModelHorse.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ModelHorse.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/OcelotModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/OcelotModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/PigModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/PigModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Polygon.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Polygon.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/QuadrupedModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/QuadrupedModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/SheepFurModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/SheepFurModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/SheepModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/SheepModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/SignModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/SignModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/SilverfishModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/SilverfishModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/SkeletonHeadModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/SkeletonHeadModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/SkeletonModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/SkeletonModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/SkiModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/SkiModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/SlimeModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/SlimeModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/SnowManModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/SnowManModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/SpiderModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/SpiderModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/SquidModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/SquidModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Vertex.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Vertex.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/VillagerGolemModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/VillagerGolemModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/VillagerModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/VillagerModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/VillagerZombieModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/VillagerZombieModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/WitchModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/WitchModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/WitherBossModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/WitherBossModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/WolfModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/WolfModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ZombieModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ZombieModel.h"
)
source_group("net/minecraft/client/model" FILES ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_MODEL})

set(_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_MODEL_DRAGON
  "${CMAKE_CURRENT_SOURCE_DIR}/DragonModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/DragonModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/EnderCrystalModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/EnderCrystalModel.h"
)
source_group("net/minecraft/client/model/dragon" FILES ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_MODEL_DRAGON})

set(_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_MODEL_GEOM
  "${CMAKE_CURRENT_SOURCE_DIR}/Cube.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Cube.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Model.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Model.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ModelPart.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ModelPart.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/TexOffs.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/TexOffs.h"
)
source_group("net/minecraft/client/model/geom" FILES ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_MODEL_GEOM})

set(_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_MULTIPLAYER
  "${CMAKE_CURRENT_SOURCE_DIR}/ClientConnection.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ClientConnection.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/MultiPlayerChunkCache.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/MultiPlayerChunkCache.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/MultiPlayerGameMode.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/MultiPlayerGameMode.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/MultiPlayerLevel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/MultiPlayerLevel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/MultiPlayerLocalPlayer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/MultiPlayerLocalPlayer.h"
)
source_group("net/minecraft/client/multiplayer" FILES ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_MULTIPLAYER})

set(_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_PARTICLE
  "${CMAKE_CURRENT_SOURCE_DIR}/BreakingItemParticle.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/BreakingItemParticle.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/BubbleParticle.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/BubbleParticle.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/CritParticle.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CritParticle.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/CritParticle2.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CritParticle2.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/DragonBreathParticle.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/DragonBreathParticle.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/DripParticle.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/DripParticle.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/EchantmentTableParticle.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/EchantmentTableParticle.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/EnderParticle.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/EnderParticle.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ExplodeParticle.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ExplodeParticle.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/FireworksParticles.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/FireworksParticles.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/FlameParticle.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/FlameParticle.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/FootstepParticle.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/FootstepParticle.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/HeartParticle.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/HeartParticle.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/HugeExplosionParticle.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/HugeExplosionParticle.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/HugeExplosionSeedParticle.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/HugeExplosionSeedParticle.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/LavaParticle.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/LavaParticle.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/NetherPortalParticle.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/NetherPortalParticle.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/NoteParticle.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/NoteParticle.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Particle.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Particle.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ParticleEngine.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ParticleEngine.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/PlayerCloudParticle.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/PlayerCloudParticle.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/RedDustParticle.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/RedDustParticle.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/SmokeParticle.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/SmokeParticle.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/SnowShovelParticle.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/SnowShovelParticle.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/SpellParticle.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/SpellParticle.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/SplashParticle.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/SplashParticle.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/SuspendedParticle.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/SuspendedParticle.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/SuspendedTownParticle.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/SuspendedTownParticle.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/TakeAnimationParticle.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/TakeAnimationParticle.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/TerrainParticle.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/TerrainParticle.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/WaterDropParticle.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/WaterDropParticle.h"
)
source_group("net/minecraft/client/particle" FILES ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_PARTICLE})

set(_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_PLAYER
  "${CMAKE_CURRENT_SOURCE_DIR}/Input.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Input.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/LocalPlayer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/LocalPlayer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/RemotePlayer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/RemotePlayer.h"
)
source_group("net/minecraft/client/player" FILES ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_PLAYER})

set(_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_RENDERER
  "${CMAKE_CURRENT_SOURCE_DIR}/BossMobGuiInfo.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/BossMobGuiInfo.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Chunk.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Chunk.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/DirtyChunkSorter.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/DirtyChunkSorter.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/DistanceChunkSorter.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/DistanceChunkSorter.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/EntityTileRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/EntityTileRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/GameRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/GameRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/HttpTexture.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/HttpTexture.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/HttpTextureProcessor.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ItemInHandRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ItemInHandRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/LevelRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/LevelRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/MemTexture.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/MobSkinMemTextureProcessor.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/MobSkinTextureProcessor.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/MobSkinTextureProcessor.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/OffsettedRenderList.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/OffsettedRenderList.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Rect2i.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Rect2i.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Tesselator.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Tesselator.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Textures.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Textures.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/TileRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/TileRenderer.h"
)
source_group("net/minecraft/client/renderer" FILES ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_RENDERER})

set(_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_RENDERER_CULLING
  "${CMAKE_CURRENT_SOURCE_DIR}/AllowAllCuller.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/AllowAllCuller.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Culler.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Frustum.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Frustum.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/FrustumCuller.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/FrustumCuller.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/FrustumData.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/FrustumData.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ViewportCuller.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ViewportCuller.h"
)
source_group("net/minecraft/client/renderer/culling" FILES ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_RENDERER_CULLING})

set(_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_RENDERER_ENTITY
  "${CMAKE_CURRENT_SOURCE_DIR}/ArrowRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ArrowRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/BatRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/BatRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/BlazeRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/BlazeRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/BoatRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/BoatRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/CaveSpiderRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CaveSpiderRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ChickenRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ChickenRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/CowRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CowRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/CreeperRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CreeperRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/DefaultRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/DefaultRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/EnderCrystalRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/EnderCrystalRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/EnderDragonRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/EnderDragonRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/EndermanRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/EndermanRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/EntityRenderDispatcher.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/EntityRenderDispatcher.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/EntityRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/EntityRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ExperienceOrbRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ExperienceOrbRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/FallingTileRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/FallingTileRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/FireballRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/FireballRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/FishingHookRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/FishingHookRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/GhastRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/GhastRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/GiantMobRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/GiantMobRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/HorseRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/HorseRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/HumanoidMobRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/HumanoidMobRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ItemFrameRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ItemFrameRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ItemRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ItemRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ItemSpriteRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ItemSpriteRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/LavaSlimeRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/LavaSlimeRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/LeashKnotRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/LeashKnotRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/LightningBoltRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/LightningBoltRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/LivingEntityRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/LivingEntityRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/MinecartRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/MinecartRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/MinecartSpawnerRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/MinecartSpawnerRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/MobRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/MobRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/MushroomCowRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/MushroomCowRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/OcelotRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/OcelotRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/PaintingRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/PaintingRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/PigRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/PigRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/PlayerRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/PlayerRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/SheepRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/SheepRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/SilverfishRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/SilverfishRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/SkeletonRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/SkeletonRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/SlimeRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/SlimeRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/SnowManRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/SnowManRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/SpiderRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/SpiderRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/SquidRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/SquidRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/TntMinecartRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/TntMinecartRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/TntRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/TntRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/VillagerGolemRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/VillagerGolemRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/VillagerRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/VillagerRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/WitchRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/WitchRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/WitherBossRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/WitherBossRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/WitherSkullRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/WitherSkullRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/WolfRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/WolfRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ZombieRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ZombieRenderer.h"
)
source_group("net/minecraft/client/renderer/entity" FILES ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_RENDERER_ENTITY})

set(_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_RENDERER_TEXTURE
  "${CMAKE_CURRENT_SOURCE_DIR}/PreStitchedTextureMap.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/PreStitchedTextureMap.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/SimpleIcon.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/SimpleIcon.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/StitchSlot.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/StitchSlot.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/StitchedTexture.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/StitchedTexture.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Stitcher.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Stitcher.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Texture.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Texture.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/TextureAtlas.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/TextureAtlas.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/TextureHolder.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/TextureHolder.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/TextureManager.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/TextureManager.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/TextureMap.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/TextureMap.h"
)
source_group("net/minecraft/client/renderer/texture" FILES ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_RENDERER_TEXTURE})

set(_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_RENDERER_TEXTURE_CUSTOM
  "${CMAKE_CURRENT_SOURCE_DIR}/ClockTexture.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ClockTexture.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/CompassTexture.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CompassTexture.h"
)
source_group("net/minecraft/client/renderer/texture/custom" FILES ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_RENDERER_TEXTURE_CUSTOM})

set(_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_RENDERER_TILEENTITY
  "${CMAKE_CURRENT_SOURCE_DIR}/BeaconRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/BeaconRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ChestRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ChestRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/EnchantTableRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/EnchantTableRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/EnderChestRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/EnderChestRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/MobSpawnerRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/MobSpawnerRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/PistonPieceRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/PistonPieceRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/SignRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/SignRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/SkullTileRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/SkullTileRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/TheEndPortalRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/TheEndPortalRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/TileEntityRenderDispatcher.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/TileEntityRenderDispatcher.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/TileEntityRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/TileEntityRenderer.h"
)
source_group("net/minecraft/client/renderer/tileentity" FILES ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_RENDERER_TILEENTITY})

set(_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_RESOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/ResourceLocation.h"
)
source_group("net/minecraft/client/resources" FILES ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_RESOURCES})

set(_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_SKINS
  "${CMAKE_CURRENT_SOURCE_DIR}/AbstractTexturePack.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/AbstractTexturePack.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/DLCTexturePack.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/DLCTexturePack.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/DefaultTexturePack.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/DefaultTexturePack.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/FileTexturePack.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/FileTexturePack.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/FolderTexturePack.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/FolderTexturePack.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/TexturePack.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/TexturePack.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/TexturePackRepository.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/TexturePackRepository.h"
)
source_group("net/minecraft/client/skins" FILES ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_SKINS})

set(_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_TITLE
  "${CMAKE_CURRENT_SOURCE_DIR}/TitleScreen.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/TitleScreen.h"
)
source_group("net/minecraft/client/title" FILES ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_TITLE})

set(_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_SERVER
  "${CMAKE_CURRENT_SOURCE_DIR}/ConsoleInput.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ConsoleInput.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ConsoleInputSource.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/DispenserBootstrap.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/DispenserBootstrap.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/MinecraftServer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/MinecraftServer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/PlayerList.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/PlayerList.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ServerInterface.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ServerScoreboard.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ServerScoreboard.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Settings.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Settings.h"
)
source_group("net/minecraft/server" FILES ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_SERVER})

set(_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_SERVER_COMMANDS
  "${CMAKE_CURRENT_SOURCE_DIR}/ServerCommandDispatcher.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ServerCommandDispatcher.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/TeleportCommand.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/TeleportCommand.h"
)
source_group("net/minecraft/server/commands" FILES ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_SERVER_COMMANDS})

set(_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_SERVER_LEVEL
  "${CMAKE_CURRENT_SOURCE_DIR}/DerivedServerLevel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/DerivedServerLevel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/EntityTracker.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/EntityTracker.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/PlayerChunkMap.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/PlayerChunkMap.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ServerChunkCache.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ServerChunkCache.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ServerLevel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ServerLevel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ServerLevelListener.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ServerLevelListener.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ServerPlayer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ServerPlayer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ServerPlayerGameMode.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ServerPlayerGameMode.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/TrackedEntity.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/TrackedEntity.h"
)
source_group("net/minecraft/server/level" FILES ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_SERVER_LEVEL})

set(_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_SERVER_NETWORK
  "${CMAKE_CURRENT_SOURCE_DIR}/PendingConnection.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/PendingConnection.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/PlayerConnection.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/PlayerConnection.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ServerConnection.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ServerConnection.h"
)
source_group("net/minecraft/server/network" FILES ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_SERVER_NETWORK})

set(_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_STATS
  "${CMAKE_CURRENT_SOURCE_DIR}/StatsCounter.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/StatsSyncher.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/StatsSyncher.h"
)
source_group("net/minecraft/stats" FILES ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_STATS})

set(MINECRAFT_CLIENT_COMMON
  ${_MINECRAFT_CLIENT_COMMON_ROOT}
  ${_MINECRAFT_CLIENT_COMMON_COMMON}
  ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES}
  ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_AUDIO}
  ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_COLOURS}
  ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_DLC}
  ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_GAMERULES}
  ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_GAMERULES_LEVELGENERATION}
  ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_GAMERULES_LEVELGENERATION_STRUCTUREACTIONS}
  ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_GAMERULES_LEVELRULES}
  ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_GAMERULES_LEVELRULES_RULEDEFINITIONS}
  ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_GAMERULES_LEVELRULES_RULES}
  ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_LEADERBOARDS}
  ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_LOCALISATION}
  ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_NETWORK}
  ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_TELEMETRY}
  ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_TRIAL}
  ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_TUTORIAL}
  ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_TUTORIAL_CONSTRAINTS}
  ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_TUTORIAL_HINTS}
  ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_TUTORIAL_TASKS}
  ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_UI}
  ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_UI_ALL_PLATFORMS}
  ${_MINECRAFT_CLIENT_COMMON_COMMON_SOURCE_FILES_UI_SCENES_FRONTEND_MENU_SCREENS}
  ${_MINECRAFT_CLIENT_COMMON_DURANGO_SERVICECONFIG}
  ${_MINECRAFT_CLIENT_COMMON_DURANGO_SOURCE_FILES}
  ${_MINECRAFT_CLIENT_COMMON_DURANGO_SOURCE_FILES_ACHIEVEMENTS}
  ${_MINECRAFT_CLIENT_COMMON_HEADER_FILES}
  ${_MINECRAFT_CLIENT_COMMON_ORBIS_4JLIBS_INC}
  ${_MINECRAFT_CLIENT_COMMON_PSVITA}
  ${_MINECRAFT_CLIENT_COMMON_PSVITA_GAMECONFIG}
  ${_MINECRAFT_CLIENT_COMMON_PSVITA_MILES_SOUND_SYSTEM_INCLUDE}
  ${_MINECRAFT_CLIENT_COMMON_PSVITA_SOURCE_FILES_SOCIAL}
  ${_MINECRAFT_CLIENT_COMMON_PSVITA_XML}
  ${_MINECRAFT_CLIENT_COMMON_SOURCE_FILES}
  ${_MINECRAFT_CLIENT_COMMON_WINDOWS64_IGGY_GDRAW}
  ${_MINECRAFT_CLIENT_COMMON_WINDOWS64_SOURCE_FILES_NETWORK}
  ${_MINECRAFT_CLIENT_COMMON_XBOX_4JLIBS_INC}
  ${_MINECRAFT_CLIENT_COMMON_XBOX_SOURCE_FILES_AUDIO}
  ${_MINECRAFT_CLIENT_COMMON_XBOX_SOURCE_FILES_NETWORK}
  ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT}
  ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_GUI}
  ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_GUI_ACHIEVEMENT}
  ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_GUI_PARTICLE}
  ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_LEVEL}
  ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_MODEL}
  ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_MODEL_DRAGON}
  ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_MODEL_GEOM}
  ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_MULTIPLAYER}
  ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_PARTICLE}
  ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_PLAYER}
  ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_RENDERER}
  ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_RENDERER_CULLING}
  ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_RENDERER_ENTITY}
  ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_RENDERER_TEXTURE}
  ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_RENDERER_TEXTURE_CUSTOM}
  ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_RENDERER_TILEENTITY}
  ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_RESOURCES}
  ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_SKINS}
  ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_CLIENT_TITLE}
  ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_SERVER}
  ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_SERVER_COMMANDS}
  ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_SERVER_LEVEL}
  ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_SERVER_NETWORK}
  ${_MINECRAFT_CLIENT_COMMON_NET_MINECRAFT_STATS}
)
