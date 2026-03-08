#include "stdafx.h"
#include "../../../Minecraft.World/net.minecraft.world.item.h"
#include "../../../Minecraft.World/net.minecraft.world.level.h"
#include "../../../Minecraft.World/net.minecraft.world.level.tile.h"
#include "../../../Minecraft.World/net.minecraft.world.phys.h"
#include "../../../Minecraft.World/net.minecraft.world.effect.h"
#include "../GameRules/ConsoleGameRules.h"
#include "DiggerItemHint.h"
#include "TutorialTasks.h"
#include "AreaHint.h"
#include "FullTutorial.h"
#include "TutorialConstraints.h"

FullTutorial::FullTutorial(int iPad, bool isTrial /*= false*/)
	: Tutorial(iPad, true)
{
	m_isTrial = isTrial;
	m_freezeTime = true;
	m_progressFlags = 0;

	for(unsigned int i = 0; i < e_Tutorial_State_Max; ++i)
	{
		m_completedStates[i] = false;
	}

	addMessage(IDS_TUTORIAL_COMPLETED);

	/*
	*
	*
	* GAMEPLAY
	*
	*/	
	// START OF BASIC TUTORIAL
	if( m_isTrial )
	{
		addTask(e_Tutorial_State_Gameplay, new ChoiceTask(this, IDS_TUTORIAL_TASK_OVERVIEW, IDS_TUTORIAL_PROMPT_START_TUTORIAL, true, ACTION_MENU_A, ACTION_MENU_B, e_Tutorial_Completion_Jump_To_Last_Task, eTelemetryTutorial_TrialStart) );
	}
	else
	{
#ifdef _XBOX
		if(getCompleted(eTutorial_Telemetry_Halfway) && !isStateCompleted(e_Tutorial_State_Redstone_And_Piston) )
		{
			addTask(e_Tutorial_State_Gameplay, new ChoiceTask(this, IDS_TUTORIAL_NEW_FEATURES_CHOICE, IDS_TUTORIAL_PROMPT_NEW_FEATURES_CHOICE, true, ACTION_MENU_A, ACTION_MENU_B, e_Tutorial_Completion_Jump_To_Last_Task, eTelemetryTutorial_TrialStart) );	
		}

		addTask(e_Tutorial_State_Gameplay,  new InfoTask(this, IDS_TUTORIAL_TASK_OVERVIEW, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
#else
		if(getCompleted(eTutorial_Telemetry_Halfway))
		{
			addTask(e_Tutorial_State_Gameplay, new ChoiceTask(this, IDS_TUTORIAL_TASK_OVERVIEW, IDS_TUTORIAL_PROMPT_START_TUTORIAL, true, ACTION_MENU_A, ACTION_MENU_B, e_Tutorial_Completion_Jump_To_Last_Task, eTelemetryTutorial_TrialStart) );
		}
		else
		{
			addTask(e_Tutorial_State_Gameplay,  new InfoTask(this, IDS_TUTORIAL_TASK_OVERVIEW, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		}
#endif
	}

	int lookMappings[] = {MINECRAFT_ACTION_LOOK_UP, MINECRAFT_ACTION_LOOK_DOWN, MINECRAFT_ACTION_LOOK_LEFT, MINECRAFT_ACTION_LOOK_RIGHT};
	int moveMappings[] = {MINECRAFT_ACTION_FORWARD, MINECRAFT_ACTION_BACKWARD, MINECRAFT_ACTION_LEFT, MINECRAFT_ACTION_RIGHT};
	int iLookCompletionMaskA[]= {	10, // 1010 
								9,  // 1001
								6,  // 0110
								5   // 0101
	};
	addTask(e_Tutorial_State_Gameplay,  new ControllerTask( this, IDS_TUTORIAL_TASK_LOOK, false, false, lookMappings, 4, iLookCompletionMaskA, 4, moveMappings, 4) );

	addTask(e_Tutorial_State_Gameplay,  new ControllerTask( this, IDS_TUTORIAL_TASK_MOVE, false, false, moveMappings, 4, iLookCompletionMaskA, 4, lookMappings, 4) );

	addTask(e_Tutorial_State_Gameplay,  new InfoTask(this, IDS_TUTORIAL_TASK_SPRINT, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );

	int jumpMappings[] = {MINECRAFT_ACTION_JUMP};
	addTask(e_Tutorial_State_Gameplay,  new ControllerTask( this, IDS_TUTORIAL_TASK_JUMP, false, true, jumpMappings, 1) );

	int mineMappings[] = {MINECRAFT_ACTION_ACTION};
	addTask(e_Tutorial_State_Gameplay,  new ControllerTask( this, IDS_TUTORIAL_TASK_MINE, false, true,  mineMappings, 1) );
	addTask(e_Tutorial_State_Gameplay,  new PickupTask( Tile::treeTrunk_Id, 4, -1, this, IDS_TUTORIAL_TASK_CHOP_WOOD ) );

	int scrollMappings[] = {MINECRAFT_ACTION_LEFT_SCROLL,MINECRAFT_ACTION_RIGHT_SCROLL};
	//int scrollMappings[] = {ACTION_MENU_LEFT_SCROLL,ACTION_MENU_RIGHT_SCROLL};
	int iScrollCompletionMaskA[]= {	2, // 10 
									1};// 01
	addTask(e_Tutorial_State_Gameplay,  new ControllerTask( this, IDS_TUTORIAL_TASK_SCROLL, false, false, scrollMappings, 2,iScrollCompletionMaskA,2) );

	int invMappings[] = {MINECRAFT_ACTION_INVENTORY};
	addTask(e_Tutorial_State_Gameplay,  new ControllerTask( this, IDS_TUTORIAL_TASK_INVENTORY, false, false, invMappings, 1) );
	addTask(e_Tutorial_State_Gameplay, new StateChangeTask( e_Tutorial_State_Inventory_Menu, this) );

	addTask(e_Tutorial_State_Gameplay,  new InfoTask(this, IDS_TUTORIAL_TASK_FOOD_BAR_DEPLETE, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
	addTask(e_Tutorial_State_Gameplay,  new InfoTask(this, IDS_TUTORIAL_TASK_FOOD_BAR_HEAL, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
	addTask(e_Tutorial_State_Gameplay,  new InfoTask(this, IDS_TUTORIAL_TASK_FOOD_BAR_FEED, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );

	// While they should only eat the item we give them, includ the ability to complete this task with different items
	int foodItems[] = {Item::mushroomStew_Id, Item::apple_Id, Item::bread_Id, Item::porkChop_raw_Id, Item::porkChop_cooked_Id,
						Item::apple_gold_Id, Item::fish_raw_Id, Item::fish_cooked_Id, Item::cookie_Id, Item::beef_cooked_Id,
						Item::beef_raw_Id, Item::chicken_cooked_Id, Item::chicken_raw_Id, Item::melon_Id, Item::rotten_flesh_Id};
	addTask(e_Tutorial_State_Gameplay,  new CompleteUsingItemTask(this, IDS_TUTORIAL_TASK_FOOD_BAR_EAT_STEAK, foodItems, 15, true) );

	int crftMappings[] = {MINECRAFT_ACTION_CRAFTING};
	addTask(e_Tutorial_State_Gameplay,  new ControllerTask( this, IDS_TUTORIAL_TASK_CRAFTING, false, false, crftMappings, 1) );

	addTask(e_Tutorial_State_Gameplay, new ProgressFlagTask( &m_progressFlags, FULL_TUTORIAL_PROGRESS_2_X_2_Crafting, ProgressFlagTask::e_Progress_Set_Flag, this ) );
	addTask(e_Tutorial_State_Gameplay, new StateChangeTask( e_Tutorial_State_2x2Crafting_Menu, this) );

	addTask(e_Tutorial_State_Gameplay,  new CraftTask( Tile::wood_Id, -1, 1, this, IDS_TUTORIAL_TASK_CREATE_PLANKS) );	
	addTask(e_Tutorial_State_Gameplay,  new CraftTask( Tile::workBench_Id, -1, 1, this, IDS_TUTORIAL_TASK_CREATE_CRAFTING_TABLE) );

	//int useMappings[] = {MINECRAFT_ACTION_USE};
	//addTask(e_Tutorial_State_Gameplay, new ControllerTask( this, IDS_TUTORIAL_TASK_USE, false, false, useMappings, 1) );
	addTask(e_Tutorial_State_Gameplay, new InfoTask(this, IDS_TUTORIAL_TASK_USE, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
	addTask(e_Tutorial_State_Gameplay, new UseItemTask( Tile::workBench_Id, this, IDS_TUTORIAL_TASK_PLACE_WORKBENCH, true ) );

	addTask(e_Tutorial_State_Gameplay, new InfoTask(this, IDS_TUTORIAL_TASK_NIGHT_DANGER, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
	addTask(e_Tutorial_State_Gameplay, new InfoTask(this, IDS_TUTORIAL_TASK_NEARBY_SHELTER, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
	addTask(e_Tutorial_State_Gameplay, new InfoTask(this, IDS_TUTORIAL_TASK_COLLECT_RESOURCES, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );

	// END OF BASIC TUTORIAL
	
	addTask(e_Tutorial_State_Gameplay, new ChoiceTask(this, IDS_TUTORIAL_TASK_BASIC_COMPLETE, IDS_TUTORIAL_PROMPT_BASIC_COMPLETE, true, ACTION_MENU_A, ACTION_MENU_B, e_Tutorial_Completion_Jump_To_Last_Task, eTelemetryTutorial_Halfway) );

	// START OF FULL TUTORIAL
	
	addTask(e_Tutorial_State_Gameplay, new UseTileTask( Tile::workBench_Id, this, IDS_TUTORIAL_TASK_OPEN_WORKBENCH, false ) );

	addTask(e_Tutorial_State_Gameplay, new ProgressFlagTask( &m_progressFlags, FULL_TUTORIAL_PROGRESS_3_X_3_Crafting, ProgressFlagTask::e_Progress_Set_Flag, this ) );
	addTask(e_Tutorial_State_Gameplay, new StateChangeTask( e_Tutorial_State_3x3Crafting_Menu, this) );

	addTask(e_Tutorial_State_Gameplay, new CraftTask( Item::stick->id, -1, 1, this, IDS_TUTORIAL_TASK_CREATE_STICKS) );

	int shovelItems[] = {Item::shovel_wood->id, Item::shovel_stone->id, Item::shovel_iron->id, Item::shovel_gold->id, Item::shovel_diamond->id};
	int shovelAuxVals[] = {-1,-1,-1,-1,-1};
	addTask(e_Tutorial_State_Gameplay, new CraftTask( shovelItems, shovelAuxVals, 5, 1, this, IDS_TUTORIAL_TASK_CREATE_WOODEN_SHOVEL) );

	int hatchetItems[] = {Item::hatchet_wood->id, Item::hatchet_stone->id, Item::hatchet_iron->id, Item::hatchet_gold->id, Item::hatchet_diamond->id};
	int hatchetAuxVals[] = {-1,-1,-1,-1,-1};
	addTask(e_Tutorial_State_Gameplay, new CraftTask( hatchetItems, hatchetAuxVals, 5, 1, this, IDS_TUTORIAL_TASK_CREATE_WOODEN_HATCHET) );

	int pickaxeItems[] = {Item::pickAxe_wood->id, Item::pickAxe_stone->id, Item::pickAxe_iron->id, Item::pickAxe_gold->id, Item::pickAxe_diamond->id};
	int pickaxeAuxVals[] = {-1,-1,-1,-1,-1};
	addTask(e_Tutorial_State_Gameplay, new CraftTask( pickaxeItems, pickaxeAuxVals, 5, 1, this, IDS_TUTORIAL_TASK_CREATE_WOODEN_PICKAXE) );

	addTask(e_Tutorial_State_Gameplay, new PickupTask( Tile::cobblestone_Id, 8, -1, this, IDS_TUTORIAL_TASK_MINE_STONE ) );	

	addTask(e_Tutorial_State_Gameplay, new ProgressFlagTask( &m_progressFlags, FULL_TUTORIAL_PROGRESS_CRAFT_FURNACE, ProgressFlagTask::e_Progress_Set_Flag, this ) );
	addTask(e_Tutorial_State_Gameplay, new CraftTask( Tile::furnace_Id, -1, 1, this, IDS_TUTORIAL_TASK_CREATE_FURNACE ) );
	addTask(e_Tutorial_State_Gameplay, new UseTileTask(Tile::furnace_Id, this, IDS_TUTORIAL_TASK_PLACE_AND_OPEN_FURNACE) );

	addTask(e_Tutorial_State_Gameplay, new ProgressFlagTask( &m_progressFlags, FULL_TUTORIAL_PROGRESS_USE_FURNACE, ProgressFlagTask::e_Progress_Set_Flag, this ) );
	addTask(e_Tutorial_State_Gameplay, new StateChangeTask( e_Tutorial_State_Furnace_Menu, this) );
	addTask(e_Tutorial_State_Gameplay, new CraftTask( Item::coal->id, -1, 1, this, IDS_TUTORIAL_TASK_CREATE_CHARCOAL) );
	addTask(e_Tutorial_State_Gameplay, new CraftTask( Tile::glass_Id, -1, 1, this, IDS_TUTORIAL_TASK_CREATE_GLASS) );
	addTask(e_Tutorial_State_Gameplay, new CraftTask( Item::door_wood->id, -1, 1, this, IDS_TUTORIAL_TASK_CREATE_WOODEN_DOOR) );
	addTask(e_Tutorial_State_Gameplay, new UseItemTask(Item::door_wood->id, this, IDS_TUTORIAL_TASK_PLACE_DOOR) );
	addTask(e_Tutorial_State_Gameplay, new CraftTask( Tile::torch_Id, -1, 1, this, IDS_TUTORIAL_TASK_CREATE_TORCH) );

	if(app.getGameRuleDefinitions() != NULL)
	{
		AABB *area = app.getGameRuleDefinitions()->getNamedArea(L"tutorialArea");
		if(area != NULL)
		{
			vector<TutorialConstraint *> *areaConstraints = new vector<TutorialConstraint *>();
			areaConstraints->push_back( new AreaConstraint( IDS_TUTORIAL_CONSTRAINT_TUTORIAL_AREA, area->x0,area->y0,area->z0,area->x1,area->y1,area->z1) );
			addTask(e_Tutorial_State_Gameplay, new AreaTask(e_Tutorial_State_Gameplay,this, areaConstraints) );
		}
	}

	// This MUST be the last task in the e_Tutorial_State_Gameplay state. Some of the earlier tasks will skip to the last
	// task when complete, and this is the one that we want the player to see.
	ProcedureCompoundTask *finalTask = new ProcedureCompoundTask( this );
	finalTask->AddTask( new InfoTask(this, IDS_TUTORIAL_COMPLETED, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A, eTelemetryTutorial_Complete) );
	// 4J Stu - Remove this string as it refers to things that don't exist in the current tutorial world!
	//finalTask->AddTask( new InfoTask(this, IDS_TUTORIAL_FEATURES_IN_THIS_AREA, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
	finalTask->AddTask( new InfoTask(this, IDS_TUTORIAL_FEATURES_OUTSIDE_THIS_AREA, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
	finalTask->AddTask( new InfoTask(this, IDS_TUTORIAL_COMPLETED_EXPLORE, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
	addTask(e_Tutorial_State_Gameplay, finalTask);
	// END OF FULL TUTORIAL
	

	/*
	*
	*
	* INVENTORY
	*
	*/
	// Some tasks already added in the super class ctor	
	addTask(e_Tutorial_State_Inventory_Menu, new FullTutorialActiveTask( this, e_Tutorial_Completion_Complete_State) );
	addTask(e_Tutorial_State_Inventory_Menu, new InfoTask(this, IDS_TUTORIAL_TASK_INV_EXIT, -1, false, ACTION_MENU_B) );

	/*
	*
	*
	* CRAFTING
	*
	*/
	// Some tasks already added in the super class ctor	
	
	addTask(e_Tutorial_State_2x2Crafting_Menu, new FullTutorialActiveTask( this, e_Tutorial_Completion_Complete_State) );
	// To block progress
	addTask(e_Tutorial_State_2x2Crafting_Menu, new ProgressFlagTask( &m_progressFlags, FULL_TUTORIAL_PROGRESS_2_X_2_Crafting, ProgressFlagTask::e_Progress_Flag_On, this ) );

	addTask(e_Tutorial_State_2x2Crafting_Menu, new FullTutorialActiveTask( this, e_Tutorial_Completion_Complete_State) );

	addTask(e_Tutorial_State_2x2Crafting_Menu, new CraftTask( Tile::wood_Id, -1, 1, this, IDS_TUTORIAL_TASK_CRAFT_CREATE_PLANKS) );

	ProcedureCompoundTask *workbenchCompound = new ProcedureCompoundTask( this );
	workbenchCompound->AddTask( new XuiCraftingTask( this, IDS_TUTORIAL_TASK_CRAFT_SELECT_STRUCTURES, Recipy::eGroupType_Structure) );
	workbenchCompound->AddTask( new XuiCraftingTask( this, IDS_TUTORIAL_TASK_CRAFT_SELECT_CRAFTING_TABLE, Tile::workBench_Id) );
	workbenchCompound->AddTask( new CraftTask( Tile::workBench_Id, -1, 1, this, IDS_TUTORIAL_TASK_CREATE_CRAFTING_TABLE) );
	addTask(e_Tutorial_State_2x2Crafting_Menu, workbenchCompound );
	addTask(e_Tutorial_State_2x2Crafting_Menu, new InfoTask(this, IDS_TUTORIAL_TASK_CRAFT_EXIT_AND_PLACE_TABLE, -1, false, ACTION_MENU_B) );
 
	// 3x3 Crafting	
	addTask(e_Tutorial_State_3x3Crafting_Menu, new FullTutorialActiveTask( this, e_Tutorial_Completion_Complete_State) );
	
	addTask(e_Tutorial_State_3x3Crafting_Menu, new ProgressFlagTask( &m_progressFlags, FULL_TUTORIAL_PROGRESS_3_X_3_Crafting, ProgressFlagTask::e_Progress_Flag_On, this ) );

	addTask(e_Tutorial_State_3x3Crafting_Menu, new CraftTask( Item::stick->id, -1, 1, this, IDS_TUTORIAL_TASK_CREATE_STICKS) );
	
	ProcedureCompoundTask *shovelCompound = new ProcedureCompoundTask( this );
	shovelCompound->AddTask( new XuiCraftingTask( this, IDS_TUTORIAL_TASK_CRAFT_SELECT_TOOLS, Recipy::eGroupType_Tool) );
	shovelCompound->AddTask( new XuiCraftingTask( this, IDS_TUTORIAL_TASK_CRAFT_SELECT_WOODEN_SHOVEL, Item::shovel_wood->id) );
	shovelCompound->AddTask( new CraftTask( shovelItems, shovelAuxVals, 5, 1, this, IDS_TUTORIAL_TASK_CREATE_WOODEN_SHOVEL) );
	addTask(e_Tutorial_State_3x3Crafting_Menu, shovelCompound );
	addTask(e_Tutorial_State_3x3Crafting_Menu, new CraftTask( hatchetItems, hatchetAuxVals, 5, 1, this, IDS_TUTORIAL_TASK_CREATE_WOODEN_HATCHET) );
	addTask(e_Tutorial_State_3x3Crafting_Menu, new CraftTask( pickaxeItems, pickaxeAuxVals, 5, 1, this, IDS_TUTORIAL_TASK_CREATE_WOODEN_PICKAXE) );

	addTask(e_Tutorial_State_3x3Crafting_Menu, new InfoTask(this, IDS_TUTORIAL_TASK_CRAFT_TOOLS_BUILT, -1, false, ACTION_MENU_B) );
	
	// To block progress
	addTask(e_Tutorial_State_3x3Crafting_Menu, new ProgressFlagTask( &m_progressFlags, FULL_TUTORIAL_PROGRESS_CRAFT_FURNACE, ProgressFlagTask::e_Progress_Flag_On, this ) );

	addTask(e_Tutorial_State_3x3Crafting_Menu, new CraftTask( Tile::furnace_Id, -1, 1, this, IDS_TUTORIAL_TASK_CRAFT_CREATE_FURNACE) );
	addTask(e_Tutorial_State_3x3Crafting_Menu, new InfoTask(this, IDS_TUTORIAL_TASK_CRAFT_EXIT_AND_PLACE_FURNACE, -1, false, ACTION_MENU_B) );

	// No need to block here, as it's fine if the player wants to do this out of order
	addTask(e_Tutorial_State_3x3Crafting_Menu, new CraftTask( Item::door_wood->id, -1, 1, this, IDS_TUTORIAL_TASK_CREATE_WOODEN_DOOR) );
	addTask(e_Tutorial_State_3x3Crafting_Menu, new CraftTask( Tile::torch_Id, -1, 1, this, IDS_TUTORIAL_TASK_CREATE_TORCH) );

	/*
	*
	*
	* FURNACE
	*
	*/
	// Some tasks already added in the super class ctor

	addTask(e_Tutorial_State_Furnace_Menu, new FullTutorialActiveTask( this, e_Tutorial_Completion_Complete_State) );

	// Blocking
	addTask(e_Tutorial_State_Furnace_Menu, new ProgressFlagTask( &m_progressFlags, FULL_TUTORIAL_PROGRESS_USE_FURNACE, ProgressFlagTask::e_Progress_Flag_On, this ) );

	addTask(e_Tutorial_State_Furnace_Menu, new CraftTask( Item::coal->id, -1, 1, this, IDS_TUTORIAL_TASK_FURNACE_CREATE_CHARCOAL) );
	addTask(e_Tutorial_State_Furnace_Menu, new InfoTask(this, IDS_TUTORIAL_TASK_FURNACE_CHARCOAL_USES, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
	addTask(e_Tutorial_State_Furnace_Menu, new CraftTask( Tile::glass_Id, -1, 1, this, IDS_TUTORIAL_TASK_FURNACE_CREATE_GLASS) );

	/*
	*
	*
	* BREWING
	*
	*/
	
	// To block progress
	addTask(e_Tutorial_State_Brewing_Menu, new ProgressFlagTask( &m_progressFlags, EXTENDED_TUTORIAL_PROGRESS_USE_BREWING_STAND, ProgressFlagTask::e_Progress_Flag_On, this ) );

	int potionItems[] = {Item::potion_Id,Item::potion_Id,Item::potion_Id,Item::potion_Id,Item::potion_Id,Item::potion_Id,Item::potion_Id,Item::potion_Id};
	int potionAuxVals[] = {	MACRO_MAKEPOTION_AUXVAL(0, 0, MASK_FIRE_RESISTANCE),
							MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, 0, MASK_FIRE_RESISTANCE),
							MACRO_MAKEPOTION_AUXVAL(0, 0, MASK_FIRE_RESISTANCE),
							MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, 0, MASK_FIRE_RESISTANCE),
							MACRO_MAKEPOTION_AUXVAL(0, MASK_EXTENDED, MASK_FIRE_RESISTANCE),
							MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, MASK_EXTENDED, MASK_FIRE_RESISTANCE),
							MACRO_MAKEPOTION_AUXVAL(0, MASK_EXTENDED, MASK_FIRE_RESISTANCE),
							MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, MASK_EXTENDED, MASK_FIRE_RESISTANCE)
	};
	addTask(e_Tutorial_State_Brewing_Menu, new CraftTask( potionItems, potionAuxVals, 8, 1, this, IDS_TUTORIAL_TASK_BREWING_MENU_CREATE_FIRE_POTION) );
	addTask(e_Tutorial_State_Brewing_Menu, new InfoTask(this, IDS_TUTORIAL_TASK_BREWING_MENU_EXIT, -1, false, ACTION_MENU_B) );
	
	/*
	*
	*
	* MINECART
	*
	*/
	if(app.getGameRuleDefinitions() != NULL)
	{
		AABB *area = app.getGameRuleDefinitions()->getNamedArea(L"minecartArea");
		if(area != NULL)
		{
			addHint(e_Tutorial_State_Gameplay, new AreaHint(e_Tutorial_Hint_Always_On, this, e_Tutorial_State_Gameplay, e_Tutorial_State_Riding_Minecart, IDS_TUTORIAL_HINT_MINECART, area->x0,area->y0,area->z0,area->x1,area->y1,area->z1 ) );
		}
	}

	/*
	*
	*
	* BOAT
	*
	*/
	if(app.getGameRuleDefinitions() != NULL)
	{
		AABB *area = app.getGameRuleDefinitions()->getNamedArea(L"boatArea");
		if(area != NULL)
		{
			addHint(e_Tutorial_State_Gameplay, new AreaHint(e_Tutorial_Hint_Always_On, this, e_Tutorial_State_Gameplay, e_Tutorial_State_Riding_Boat, IDS_TUTORIAL_HINT_BOAT, area->x0,area->y0,area->z0,area->x1,area->y1,area->z1 ) );
		}
	}

	/*
	*
	*
	* FISHING
	*
	*/
	if(app.getGameRuleDefinitions() != NULL)
	{
		AABB *area = app.getGameRuleDefinitions()->getNamedArea(L"fishingArea");
		if(area != NULL)
		{
			addHint(e_Tutorial_State_Gameplay, new AreaHint(e_Tutorial_Hint_Always_On, this, e_Tutorial_State_Gameplay, e_Tutorial_State_Fishing, IDS_TUTORIAL_HINT_FISHING, area->x0,area->y0,area->z0,area->x1,area->y1,area->z1 ) );
		}
	}
	
	/*
	*
	*
	* PISTON - SELF-REPAIRING BRIDGE
	*
	*/
	if(app.getGameRuleDefinitions() != NULL)
	{
		AABB *area = app.getGameRuleDefinitions()->getNamedArea(L"pistonBridgeArea");
		if(area != NULL)
		{
			addHint(e_Tutorial_State_Gameplay, new AreaHint(e_Tutorial_Hint_Always_On, this, e_Tutorial_State_Gameplay, e_Tutorial_State_None, IDS_TUTORIAL_HINT_PISTON_SELF_REPAIRING_BRIDGE, area->x0,area->y0,area->z0,area->x1,area->y1,area->z1, true ) );
		}
	}

	/*
	*
	*
	* PISTON - PISTON AND REDSTONE CIRCUITS
	*
	*/
	if(app.getGameRuleDefinitions() != NULL)
	{
		AABB *area = app.getGameRuleDefinitions()->getNamedArea(L"pistonArea");
		if(area != NULL)
		{
			eTutorial_State redstoneAndPistonStates[] = {e_Tutorial_State_Gameplay};
			AddGlobalConstraint( new ChangeStateConstraint(this, e_Tutorial_State_Redstone_And_Piston, redstoneAndPistonStates, 1, area->x0,area->y0,area->z0,area->x1,area->y1,area->z1) );

			addTask(e_Tutorial_State_Redstone_And_Piston, new ChoiceTask(this, IDS_TUTORIAL_REDSTONE_OVERVIEW, IDS_TUTORIAL_PROMPT_REDSTONE_OVERVIEW, true, ACTION_MENU_A, ACTION_MENU_B, e_Tutorial_Completion_Complete_State_Gameplay_Constraints, eTelemetryTutorial_Redstone_And_Pistons) );
			addTask(e_Tutorial_State_Redstone_And_Piston, new InfoTask(this, IDS_TUTORIAL_TASK_REDSTONE_POWER_SOURCES, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Redstone_And_Piston, new InfoTask(this, IDS_TUTORIAL_TASK_REDSTONE_TRIPWIRE, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Redstone_And_Piston, new InfoTask(this, IDS_TUTORIAL_TASK_REDSTONE_POWER_SOURCES_POSITION, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Redstone_And_Piston, new InfoTask(this, IDS_TUTORIAL_TASK_REDSTONE_DUST, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Redstone_And_Piston, new InfoTask(this, IDS_TUTORIAL_TASK_REDSTONE_REPEATER, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Redstone_And_Piston, new InfoTask(this, IDS_TUTORIAL_TASK_PISTONS, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Redstone_And_Piston, new InfoTask(this, IDS_TUTORIAL_TASK_TRY_IT, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		}
	}
	
	/*
	*
	*
	* PORTAL
	*
	*/
	if(app.getGameRuleDefinitions() != NULL)
	{
		AABB *area = app.getGameRuleDefinitions()->getNamedArea(L"portalArea");
		if(area != NULL)
		{
			eTutorial_State portalStates[] = {e_Tutorial_State_Gameplay};
			AddGlobalConstraint( new ChangeStateConstraint(this, e_Tutorial_State_Portal, portalStates, 1, area->x0,area->y0,area->z0,area->x1,area->y1,area->z1) );

			addTask(e_Tutorial_State_Portal, new ChoiceTask(this, IDS_TUTORIAL_PORTAL_OVERVIEW, IDS_TUTORIAL_PROMPT_PORTAL_OVERVIEW, true, ACTION_MENU_A, ACTION_MENU_B, e_Tutorial_Completion_Complete_State_Gameplay_Constraints, eTelemetryTutorial_Portal) );
			addTask(e_Tutorial_State_Portal, new InfoTask(this, IDS_TUTORIAL_TASK_BUILD_PORTAL, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Portal, new InfoTask(this, IDS_TUTORIAL_TASK_ACTIVATE_PORTAL, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Portal, new InfoTask(this, IDS_TUTORIAL_TASK_USE_PORTAL, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Portal, new InfoTask(this, IDS_TUTORIAL_TASK_NETHER, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Portal, new InfoTask(this, IDS_TUTORIAL_TASK_NETHER_FAST_TRAVEL, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		}
	}
	
	/*
	*
	*
	* CREATIVE
	*
	*/
	if(app.getGameRuleDefinitions() != NULL)
	{
		AABB *area = app.getGameRuleDefinitions()->getNamedArea(L"creativeArea");
		if(area != NULL)
		{
			eTutorial_State creativeStates[] = {e_Tutorial_State_Gameplay};
			AddGlobalConstraint( new ChangeStateConstraint(this, e_Tutorial_State_CreativeMode, creativeStates, 1, area->x0,area->y0,area->z0,area->x1,area->y1,area->z1,true,true,GameType::CREATIVE) );

			addTask(e_Tutorial_State_CreativeMode, new ChoiceTask(this, IDS_TUTORIAL_CREATIVE_OVERVIEW, IDS_TUTORIAL_PROMPT_CREATIVE_OVERVIEW, true, ACTION_MENU_A, ACTION_MENU_B, e_Tutorial_Completion_Jump_To_Last_Task, eTelemetryTutorial_CreativeMode) );
			addTask(e_Tutorial_State_CreativeMode, new InfoTask(this, IDS_TUTORIAL_TASK_CREATIVE_MODE, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_CreativeMode, new InfoTask(this, IDS_TUTORIAL_TASK_FLY, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );

			int crftMappings[] = {MINECRAFT_ACTION_CRAFTING};
			addTask(e_Tutorial_State_CreativeMode,  new ControllerTask( this, IDS_TUTORIAL_TASK_OPEN_CREATIVE_INVENTORY, false, false, crftMappings, 1) );
			addTask(e_Tutorial_State_CreativeMode, new StateChangeTask( e_Tutorial_State_Creative_Inventory_Menu, this) );

			// This last task ensures that the player is still in creative mode until they exit the area (but could skip the previous instructional stuff)
			ProcedureCompoundTask *creativeFinalTask = new ProcedureCompoundTask( this );

			AABB *exitArea = app.getGameRuleDefinitions()->getNamedArea(L"creativeExitArea");
			if(exitArea != NULL)
			{
				vector<TutorialConstraint *> *creativeExitAreaConstraints = new vector<TutorialConstraint *>();
				creativeExitAreaConstraints->push_back( new AreaConstraint( -1, exitArea->x0,exitArea->y0,exitArea->z0,exitArea->x1,exitArea->y1,exitArea->z1,true,false) );
				creativeFinalTask->AddTask( new AreaTask(e_Tutorial_State_CreativeMode, this, creativeExitAreaConstraints,IDS_TUTORIAL_TASK_CREATIVE_EXIT,AreaTask::eAreaTaskCompletion_CompleteOnConstraintsSatisfied) );
			}

			vector<TutorialConstraint *> *creativeAreaConstraints = new vector<TutorialConstraint *>();
			creativeAreaConstraints->push_back( new AreaConstraint( IDS_TUTORIAL_CONSTRAINT_TUTORIAL_AREA, area->x0,area->y0,area->z0,area->x1,area->y1,area->z1) );
			creativeFinalTask->AddTask( new AreaTask(e_Tutorial_State_CreativeMode, this, creativeAreaConstraints) );

			creativeFinalTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_CREATIVE_COMPLETE, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );

			addTask(e_Tutorial_State_CreativeMode,creativeFinalTask);
		}
	}
	
	/*
	*
	*
	* BREWING
	*
	*/
	if(app.getGameRuleDefinitions() != NULL)
	{
		AABB *area = app.getGameRuleDefinitions()->getNamedArea(L"brewingArea");
		if(area != NULL)
		{
			eTutorial_State brewingStates[] = {e_Tutorial_State_Gameplay};
			AddGlobalConstraint( new ChangeStateConstraint(this, e_Tutorial_State_Brewing, brewingStates, 1, area->x0,area->y0,area->z0,area->x1,area->y1,area->z1) );

			addTask(e_Tutorial_State_Brewing, new ChoiceTask(this, IDS_TUTORIAL_TASK_BREWING_OVERVIEW, IDS_TUTORIAL_PROMPT_BREWING_OVERVIEW, true, ACTION_MENU_A, ACTION_MENU_B, e_Tutorial_Completion_Complete_State_Gameplay_Constraints, eTelemetryTutorial_Brewing) );

			ProcedureCompoundTask *fillWaterBottleTask = new ProcedureCompoundTask( this );			
			fillWaterBottleTask->AddTask( new PickupTask( Item::glassBottle_Id, 1, -1, this, IDS_TUTORIAL_TASK_BREWING_GET_GLASS_BOTTLE ) );
			fillWaterBottleTask->AddTask( new PickupTask( Item::potion_Id, 1, 0, this, IDS_TUTORIAL_TASK_BREWING_FILL_GLASS_BOTTLE ) );
			addTask(e_Tutorial_State_Brewing, fillWaterBottleTask);

			addTask(e_Tutorial_State_Brewing, new InfoTask(this, IDS_TUTORIAL_TASK_BREWING_FILL_CAULDRON, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			
			
			addTask(e_Tutorial_State_Brewing, new ProgressFlagTask( &m_progressFlags, EXTENDED_TUTORIAL_PROGRESS_USE_BREWING_STAND, ProgressFlagTask::e_Progress_Set_Flag, this ) );
			addTask(e_Tutorial_State_Brewing, new CraftTask( potionItems, potionAuxVals, 8, 1, this, IDS_TUTORIAL_TASK_BREWING_CREATE_FIRE_POTION) );

			addTask(e_Tutorial_State_Brewing, new InfoTask(this, IDS_TUTORIAL_TASK_BREWING_USE_POTION, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Brewing, new EffectChangedTask(this, IDS_TUTORIAL_TASK_BREWING_DRINK_FIRE_POTION, MobEffect::fireResistance) );
			addTask(e_Tutorial_State_Brewing, new InfoTask(this, IDS_TUTORIAL_TASK_BREWING_USE_EFFECTS, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		}
	}
	
	/*
	*
	*
	* ENCHANTING
	*
	*/
	if(app.getGameRuleDefinitions() != NULL)
	{
		AABB *area = app.getGameRuleDefinitions()->getNamedArea(L"enchantingArea");
		if(area != NULL)
		{
			eTutorial_State enchantingStates[] = {e_Tutorial_State_Gameplay};
			AddGlobalConstraint( new ChangeStateConstraint(this, e_Tutorial_State_Enchanting, enchantingStates, 1, area->x0,area->y0,area->z0,area->x1,area->y1,area->z1) );

			addTask(e_Tutorial_State_Enchanting, new ChoiceTask(this, IDS_TUTORIAL_TASK_ENCHANTING_OVERVIEW, IDS_TUTORIAL_PROMPT_ENCHANTING_OVERVIEW, true, ACTION_MENU_A, ACTION_MENU_B, e_Tutorial_Completion_Complete_State_Gameplay_Constraints, eTelemetryTutorial_Enchanting) );

			addTask(e_Tutorial_State_Enchanting, new InfoTask(this, IDS_TUTORIAL_TASK_ENCHANTING_SUMMARY, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Enchanting, new InfoTask(this, IDS_TUTORIAL_TASK_ENCHANTING_BOOKS, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Enchanting, new InfoTask(this, IDS_TUTORIAL_TASK_ENCHANTING_BOOKCASES, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Enchanting, new InfoTask(this, IDS_TUTORIAL_TASK_ENCHANTING_EXPERIENCE, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Enchanting, new InfoTask(this, IDS_TUTORIAL_TASK_ENCHANTING_BOTTLE_O_ENCHANTING, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Enchanting, new InfoTask(this, IDS_TUTORIAL_TASK_ENCHANTING_USE_CHESTS, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		}
	}
	
	/*
	*
	*
	* ANVIL
	*
	*/
	if(app.getGameRuleDefinitions() != NULL)
	{
		AABB *area = app.getGameRuleDefinitions()->getNamedArea(L"anvilArea");
		if(area != NULL)
		{
			eTutorial_State enchantingStates[] = {e_Tutorial_State_Gameplay};
			AddGlobalConstraint( new ChangeStateConstraint(this, e_Tutorial_State_Anvil, enchantingStates, 1, area->x0,area->y0,area->z0,area->x1,area->y1,area->z1) );

			addTask(e_Tutorial_State_Anvil, new ChoiceTask(this, IDS_TUTORIAL_TASK_ANVIL_OVERVIEW, IDS_TUTORIAL_PROMPT_ANVIL_OVERVIEW, true, ACTION_MENU_A, ACTION_MENU_B, e_Tutorial_Completion_Complete_State_Gameplay_Constraints, eTelemetryTutorial_Anvil) );
									
			addTask(e_Tutorial_State_Anvil, new InfoTask(this, IDS_TUTORIAL_TASK_ANVIL_SUMMARY, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Anvil, new InfoTask(this, IDS_TUTORIAL_TASK_ANVIL_ENCHANTED_BOOKS, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Anvil, new InfoTask(this, IDS_TUTORIAL_TASK_ANVIL_COST, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Anvil, new InfoTask(this, IDS_TUTORIAL_TASK_ANVIL_COST2, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Anvil, new InfoTask(this, IDS_TUTORIAL_TASK_ANVIL_RENAMING, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Anvil, new InfoTask(this, IDS_TUTORIAL_TASK_ANVIL_USE_CHESTS, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		}
	}

	/*
	*
	*
	* TRADING
	*
	*/
	if(app.getGameRuleDefinitions() != NULL)
	{
		AABB *area = app.getGameRuleDefinitions()->getNamedArea(L"tradingArea");
		if(area != NULL)
		{
			eTutorial_State tradingStates[] = {e_Tutorial_State_Gameplay};
			AddGlobalConstraint( new ChangeStateConstraint(this, e_Tutorial_State_Trading, tradingStates, 1, area->x0,area->y0,area->z0,area->x1,area->y1,area->z1) );

			addTask(e_Tutorial_State_Trading, new ChoiceTask(this, IDS_TUTORIAL_TASK_TRADING_OVERVIEW, IDS_TUTORIAL_PROMPT_TRADING_OVERVIEW, true, ACTION_MENU_A, ACTION_MENU_B, e_Tutorial_Completion_Complete_State_Gameplay_Constraints, eTelemetryTutorial_Trading) );

			addTask(e_Tutorial_State_Trading, new InfoTask(this, IDS_TUTORIAL_TASK_TRADING_SUMMARY, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Trading, new InfoTask(this, IDS_TUTORIAL_TASK_TRADING_TRADES, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Trading, new InfoTask(this, IDS_TUTORIAL_TASK_TRADING_INCREASE_TRADES, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Trading, new InfoTask(this, IDS_TUTORIAL_TASK_TRADING_DECREASE_TRADES, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Trading, new InfoTask(this, IDS_TUTORIAL_TASK_TRADING_USE_CHESTS, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		}
	}

	/*
	*
	*
	* FIREWORKS
	*
	*/
	if(app.getGameRuleDefinitions() != NULL)
	{
		AABB *area = app.getGameRuleDefinitions()->getNamedArea(L"fireworksArea");
		if(area != NULL)
		{
			eTutorial_State fireworkStates[] = {e_Tutorial_State_Gameplay};
			AddGlobalConstraint( new ChangeStateConstraint(this, e_Tutorial_State_Fireworks, fireworkStates, 1, area->x0,area->y0,area->z0,area->x1,area->y1,area->z1) );

			addTask(e_Tutorial_State_Fireworks, new ChoiceTask(this,	IDS_TUTORIAL_TASK_FIREWORK_OVERVIEW,	IDS_TUTORIAL_PROMPT_FIREWORK_OVERVIEW, true, ACTION_MENU_A, ACTION_MENU_B, e_Tutorial_Completion_Complete_State_Gameplay_Constraints, eTelemetryTutorial_Trading) );

			addTask(e_Tutorial_State_Fireworks, new InfoTask(this,		IDS_TUTORIAL_TASK_FIREWORK_PURPOSE,		IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Fireworks, new InfoTask(this,		IDS_TUTORIAL_TASK_FIREWORK_CUSTOMISE,	IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) ); // 
			addTask(e_Tutorial_State_Fireworks, new InfoTask(this,		IDS_TUTORIAL_TASK_FIREWORK_CRAFTING,	IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		}
	}

	/*
	*
	*
	* BEACON
	*
	*/
	if(app.getGameRuleDefinitions() != NULL)
	{
		AABB *area = app.getGameRuleDefinitions()->getNamedArea(L"beaconArea");
		if(area != NULL)
		{
			eTutorial_State beaconStates[] = {e_Tutorial_State_Gameplay};
			AddGlobalConstraint( new ChangeStateConstraint(this, e_Tutorial_State_Beacon, beaconStates, 1, area->x0,area->y0,area->z0,area->x1,area->y1,area->z1) );

			addTask(e_Tutorial_State_Beacon, new ChoiceTask(this, IDS_TUTORIAL_TASK_BEACON_OVERVIEW,		IDS_TUTORIAL_PROMPT_BEACON_OVERVIEW, true, ACTION_MENU_A, ACTION_MENU_B, e_Tutorial_Completion_Complete_State_Gameplay_Constraints, eTelemetryTutorial_Beacon) );

			addTask(e_Tutorial_State_Beacon, new InfoTask(this, IDS_TUTORIAL_TASK_BEACON_PURPOSE,			IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Beacon, new InfoTask(this, IDS_TUTORIAL_TASK_BEACON_DESIGN,			IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Beacon, new InfoTask(this, IDS_TUTORIAL_TASK_BEACON_CHOOSING_POWERS,	IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		}
	}

	/*
	*
	*
	* HOPPER
	*
	*/
	if(app.getGameRuleDefinitions() != NULL)
	{
		AABB *area = app.getGameRuleDefinitions()->getNamedArea(L"hopperArea");
		if(area != NULL)
		{
			eTutorial_State hopperStates[] = {e_Tutorial_State_Gameplay};
			AddGlobalConstraint( new ChangeStateConstraint(this, e_Tutorial_State_Hopper, hopperStates, 1, area->x0,area->y0,area->z0,area->x1,area->y1,area->z1) );

			addTask(e_Tutorial_State_Hopper, new ChoiceTask(this, IDS_TUTORIAL_TASK_HOPPER_OVERVIEW,	IDS_TUTORIAL_PROMPT_HOPPER_OVERVIEW, true, ACTION_MENU_A, ACTION_MENU_B, e_Tutorial_Completion_Complete_State_Gameplay_Constraints, eTelemetryTutorial_Hopper) );

			addTask(e_Tutorial_State_Hopper, new InfoTask(this, IDS_TUTORIAL_TASK_HOPPER_PURPOSE,		IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Hopper, new InfoTask(this, IDS_TUTORIAL_TASK_HOPPER_CONTAINERS,	IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Hopper, new InfoTask(this, IDS_TUTORIAL_TASK_HOPPER_MECHANICS,		IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Hopper, new InfoTask(this, IDS_TUTORIAL_TASK_HOPPER_REDSTONE,		IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Hopper, new InfoTask(this, IDS_TUTORIAL_TASK_HOPPER_OUTPUT,		IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Hopper, new InfoTask(this, IDS_TUTORIAL_TASK_HOPPER_AREA,			IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		}
	}

	/*
	*
	*
	* ENDERCHEST
	*
	*/
	if(app.getGameRuleDefinitions() != NULL)
	{
		AABB *area = app.getGameRuleDefinitions()->getNamedArea(L"enderchestArea");
		if(area != NULL)
		{
			eTutorial_State enchantingStates[] = {e_Tutorial_State_Gameplay};
			AddGlobalConstraint( new ChangeStateConstraint(this, e_Tutorial_State_Enderchests, enchantingStates, 1, area->x0,area->y0,area->z0,area->x1,area->y1,area->z1) );

			addTask(e_Tutorial_State_Enderchests, new ChoiceTask(this, IDS_TUTORIAL_TASK_ENDERCHEST_OVERVIEW, IDS_TUTORIAL_PROMPT_ENDERCHEST_OVERVIEW, true, ACTION_MENU_A, ACTION_MENU_B, e_Tutorial_Completion_Complete_State_Gameplay_Constraints, eTelemetryTutorial_Enderchest) );

			addTask(e_Tutorial_State_Enderchests, new InfoTask(this, IDS_TUTORIAL_TASK_ENDERCHEST_SUMMARY, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Enderchests, new InfoTask(this, IDS_TUTORIAL_TASK_ENDERCHEST_PLAYERS, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Enderchests, new InfoTask(this, IDS_TUTORIAL_TASK_ENDERCHEST_FUNCTION, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		}
	}

	/*
	*
	*
	* FARMING
	*
	*/
	if(app.getGameRuleDefinitions() != NULL)
	{
		AABB *area = app.getGameRuleDefinitions()->getNamedArea(L"farmingArea");
		if(area != NULL)
		{
			eTutorial_State farmingStates[] = {e_Tutorial_State_Gameplay};
			AddGlobalConstraint( new ChangeStateConstraint(this, e_Tutorial_State_Farming, farmingStates, 1, area->x0,area->y0,area->z0,area->x1,area->y1,area->z1) );

			addTask(e_Tutorial_State_Farming, new ChoiceTask(this, IDS_TUTORIAL_FARMING_OVERVIEW, IDS_TUTORIAL_PROMPT_FARMING_OVERVIEW, true, ACTION_MENU_A, ACTION_MENU_B, e_Tutorial_Completion_Complete_State_Gameplay_Constraints, eTelemetryTutorial_Farming) );

			addTask(e_Tutorial_State_Farming, new InfoTask(this, IDS_TUTORIAL_TASK_FARMING_SEEDS, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Farming, new InfoTask(this, IDS_TUTORIAL_TASK_FARMING_FARMLAND, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Farming, new InfoTask(this, IDS_TUTORIAL_TASK_FARMING_WHEAT, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Farming, new InfoTask(this, IDS_TUTORIAL_TASK_FARMING_PUMPKIN_AND_MELON, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Farming, new InfoTask(this, IDS_TUTORIAL_TASK_FARMING_CARROTS_AND_POTATOES, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Farming, new InfoTask(this, IDS_TUTORIAL_TASK_FARMING_SUGARCANE, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Farming, new InfoTask(this, IDS_TUTORIAL_TASK_FARMING_CACTUS, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Farming, new InfoTask(this, IDS_TUTORIAL_TASK_FARMING_MUSHROOM, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Farming, new InfoTask(this, IDS_TUTORIAL_TASK_FARMING_BONEMEAL, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Farming, new InfoTask(this, IDS_TUTORIAL_TASK_FARMING_COMPLETE, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		}
	}
	
	/*
	*
	*
	* BREEDING
	*
	*/
	if(app.getGameRuleDefinitions() != NULL)
	{
		AABB *area = app.getGameRuleDefinitions()->getNamedArea(L"breedingArea");
		if(area != NULL)
		{
			eTutorial_State breedingStates[] = {e_Tutorial_State_Gameplay};
			AddGlobalConstraint( new ChangeStateConstraint(this, e_Tutorial_State_Breeding, breedingStates, 1, area->x0,area->y0,area->z0,area->x1,area->y1,area->z1) );

			addTask(e_Tutorial_State_Breeding, new ChoiceTask(this, IDS_TUTORIAL_BREEDING_OVERVIEW, IDS_TUTORIAL_PROMPT_BREEDING_OVERVIEW, true, ACTION_MENU_A, ACTION_MENU_B, e_Tutorial_Completion_Complete_State_Gameplay_Constraints, eTelemetryTutorial_Breeding) );

			addTask(e_Tutorial_State_Breeding, new InfoTask(this, IDS_TUTORIAL_TASK_BREEDING_FEED, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Breeding, new InfoTask(this, IDS_TUTORIAL_TASK_BREEDING_FEED_FOOD, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Breeding, new InfoTask(this, IDS_TUTORIAL_TASK_BREEDING_BABY, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Breeding, new InfoTask(this, IDS_TUTORIAL_TASK_BREEDING_DELAY, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Breeding, new InfoTask(this, IDS_TUTORIAL_TASK_BREEDING_FOLLOW, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Breeding, new InfoTask(this, IDS_TUTORIAL_TASK_BREEDING_RIDING_PIGS, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Breeding, new InfoTask(this, IDS_TUTORIAL_TASK_BREEDING_WOLF_TAMING, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Breeding, new InfoTask(this, IDS_TUTORIAL_TASK_BREEDING_WOLF_COLLAR, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Breeding, new InfoTask(this, IDS_TUTORIAL_TASK_BREEDING_COMPLETE, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		}
	}

	/*
	*
	*
	* SNOW AND IRON GOLEM
	*
	*/
	if(app.getGameRuleDefinitions() != NULL)
	{
		AABB *area = app.getGameRuleDefinitions()->getNamedArea(L"golemArea");
		if(area != NULL)
		{
			eTutorial_State golemStates[] = {e_Tutorial_State_Gameplay};
			AddGlobalConstraint( new ChangeStateConstraint(this, e_Tutorial_State_Golem, golemStates, 1, area->x0,area->y0,area->z0,area->x1,area->y1,area->z1) );

			addTask(e_Tutorial_State_Golem, new ChoiceTask(this, IDS_TUTORIAL_GOLEM_OVERVIEW, IDS_TUTORIAL_PROMPT_GOLEM_OVERVIEW, true, ACTION_MENU_A, ACTION_MENU_B, e_Tutorial_Completion_Complete_State_Gameplay_Constraints, eTelemetryTutorial_Golem) );

			addTask(e_Tutorial_State_Golem, new InfoTask(this, IDS_TUTORIAL_TASK_GOLEM_PUMPKIN, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Golem, new InfoTask(this, IDS_TUTORIAL_TASK_GOLEM_SNOW, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Golem, new InfoTask(this, IDS_TUTORIAL_TASK_GOLEM_IRON, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
			addTask(e_Tutorial_State_Golem, new InfoTask(this, IDS_TUTORIAL_TASK_GOLEM_IRON_VILLAGE, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );

		}
	}

}

// 4J Stu - All tutorials are onby default in the full tutorial whether the player has previously completed them or not
bool FullTutorial::isStateCompleted( eTutorial_State state )
{
	return m_completedStates[state];
}

void FullTutorial::setStateCompleted( eTutorial_State state )
{
	m_completedStates[state] = true;
	Tutorial::setStateCompleted(state);
}