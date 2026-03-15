#include "stdafx.h"
#include "..\..\..\Minecraft.World\net.minecraft.stats.h"
#include "..\..\LocalPlayer.h"
#include "..\..\..\Minecraft.World\Entity.h"
#include "..\..\..\Minecraft.World\Level.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.entity.player.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.item.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "..\..\MinecraftServer.h"
#include "..\..\Minecraft.h"
#include "..\..\MultiPlayerLocalPlayer.h"
#include "..\..\MultiPlayerLevel.h"
#include "..\..\SurvivalMode.h"
#include "Tutorial.h"
#include "TutorialMessage.h"
#include "TutorialTasks.h"
#include "TutorialConstraints.h"
#include "TutorialHints.h"
#include "Common/UI/UI.h"

vector<int> Tutorial::s_completableTasks;


int Tutorial::m_iTutorialHintDelayTime = 14000;
int Tutorial::m_iTutorialDisplayMessageTime = 7000;
int Tutorial::m_iTutorialMinimumDisplayMessageTime = 2000;
int Tutorial::m_iTutorialExtraReminderTime = 13000;
int Tutorial::m_iTutorialReminderTime = m_iTutorialDisplayMessageTime + m_iTutorialExtraReminderTime;
int Tutorial::m_iTutorialConstraintDelayRemoveTicks = 15;
int Tutorial::m_iTutorialFreezeTimeValue = 8000;

bool Tutorial::PopupMessageDetails::isSameContent(PopupMessageDetails *other)
{
	if(other == nullptr) return false;

	bool textTheSame = (m_messageId == other->m_messageId) && (m_messageString.compare(other->m_messageString) == 0);
	bool titleTheSame = (m_titleId == other->m_titleId) && (m_titleString.compare(other->m_titleString) == 0);
	bool promptTheSame = (m_promptId == other->m_promptId) && (m_promptString.compare(other->m_promptString) == 0);
	return textTheSame && titleTheSame && promptTheSame;
}

void Tutorial::staticCtor()
{
	//
	/*
	*****
	*****
	THE ORDERING OF THESE SHOULD NOT CHANGE - Although the ordering may not be totally logical due to the order tasks were added, these map
	to bits in the profile data in this order. New tasks/hints should be added at the end.
	*****
	*****
	*/
	s_completableTasks.push_back( e_Tutorial_State_Inventory_Menu );
	s_completableTasks.push_back( e_Tutorial_State_2x2Crafting_Menu );
	s_completableTasks.push_back( e_Tutorial_State_3x3Crafting_Menu );
	s_completableTasks.push_back( e_Tutorial_State_Furnace_Menu );

	s_completableTasks.push_back( e_Tutorial_State_Riding_Minecart );
	s_completableTasks.push_back( e_Tutorial_State_Riding_Boat );
	s_completableTasks.push_back( e_Tutorial_State_Fishing );
	s_completableTasks.push_back( e_Tutorial_State_Bed );

	s_completableTasks.push_back( e_Tutorial_State_Container_Menu );
	s_completableTasks.push_back( e_Tutorial_State_Trap_Menu );
	s_completableTasks.push_back( e_Tutorial_State_Redstone_And_Piston );
	s_completableTasks.push_back( e_Tutorial_State_Portal );
	s_completableTasks.push_back( e_Tutorial_State_Creative_Inventory_Menu );
	s_completableTasks.push_back( e_Tutorial_State_Food_Bar );
	s_completableTasks.push_back( e_Tutorial_State_CreativeMode );
	s_completableTasks.push_back( e_Tutorial_State_Brewing );
	s_completableTasks.push_back( e_Tutorial_State_Brewing_Menu );
	s_completableTasks.push_back( e_Tutorial_State_Enchanting );

	s_completableTasks.push_back( e_Tutorial_Hint_Hold_To_Mine );
	s_completableTasks.push_back( e_Tutorial_Hint_Tool_Damaged );
	s_completableTasks.push_back( e_Tutorial_Hint_Swim_Up );

	s_completableTasks.push_back( e_Tutorial_Hint_Unused_2 );
	s_completableTasks.push_back( e_Tutorial_Hint_Unused_3 );
	s_completableTasks.push_back( e_Tutorial_Hint_Unused_4 );
	s_completableTasks.push_back( e_Tutorial_Hint_Unused_5 );
	s_completableTasks.push_back( e_Tutorial_Hint_Unused_6 );
	s_completableTasks.push_back( e_Tutorial_Hint_Unused_7 );
	s_completableTasks.push_back( e_Tutorial_Hint_Unused_8 );
	s_completableTasks.push_back( e_Tutorial_Hint_Unused_9 );
	s_completableTasks.push_back( e_Tutorial_Hint_Unused_10 );

	s_completableTasks.push_back( e_Tutorial_Hint_Rock );
	s_completableTasks.push_back( e_Tutorial_Hint_Stone );
	s_completableTasks.push_back( e_Tutorial_Hint_Planks );
	s_completableTasks.push_back( e_Tutorial_Hint_Sapling );
	s_completableTasks.push_back( e_Tutorial_Hint_Unbreakable );
	s_completableTasks.push_back( e_Tutorial_Hint_Water );
	s_completableTasks.push_back( e_Tutorial_Hint_Lava );
	s_completableTasks.push_back( e_Tutorial_Hint_Sand );
	s_completableTasks.push_back( e_Tutorial_Hint_Gravel );
	s_completableTasks.push_back( e_Tutorial_Hint_Gold_Ore );
	s_completableTasks.push_back( e_Tutorial_Hint_Iron_Ore );
	s_completableTasks.push_back( e_Tutorial_Hint_Coal_Ore );
	s_completableTasks.push_back( e_Tutorial_Hint_Tree_Trunk );
	s_completableTasks.push_back( e_Tutorial_Hint_Glass );
	s_completableTasks.push_back( e_Tutorial_Hint_Leaves );
	s_completableTasks.push_back( e_Tutorial_Hint_Lapis_Ore );
	s_completableTasks.push_back( e_Tutorial_Hint_Lapis_Block );
	s_completableTasks.push_back( e_Tutorial_Hint_Dispenser );
	s_completableTasks.push_back( e_Tutorial_Hint_Sandstone );
	s_completableTasks.push_back( e_Tutorial_Hint_Note_Block );
	s_completableTasks.push_back( e_Tutorial_Hint_Powered_Rail );
	s_completableTasks.push_back( e_Tutorial_Hint_Detector_Rail );
	s_completableTasks.push_back( e_Tutorial_Hint_Tall_Grass );
	s_completableTasks.push_back( e_Tutorial_Hint_Wool );
	s_completableTasks.push_back( e_Tutorial_Hint_Flower );
	s_completableTasks.push_back( e_Tutorial_Hint_Mushroom );
	s_completableTasks.push_back( e_Tutorial_Hint_Gold_Block );
	s_completableTasks.push_back( e_Tutorial_Hint_Iron_Block );
	s_completableTasks.push_back( e_Tutorial_Hint_Stone_Slab );
	s_completableTasks.push_back( e_Tutorial_Hint_Red_Brick );
	s_completableTasks.push_back( e_Tutorial_Hint_Tnt );
	s_completableTasks.push_back( e_Tutorial_Hint_Bookshelf );
	s_completableTasks.push_back( e_Tutorial_Hint_Moss_Stone );
	s_completableTasks.push_back( e_Tutorial_Hint_Obsidian );
	s_completableTasks.push_back( e_Tutorial_Hint_Torch );
	s_completableTasks.push_back( e_Tutorial_Hint_MobSpawner );
	s_completableTasks.push_back( e_Tutorial_Hint_Chest );
	s_completableTasks.push_back( e_Tutorial_Hint_Redstone );
	s_completableTasks.push_back( e_Tutorial_Hint_Diamond_Ore );
	s_completableTasks.push_back( e_Tutorial_Hint_Diamond_Block );
	s_completableTasks.push_back( e_Tutorial_Hint_Crafting_Table );
	s_completableTasks.push_back( e_Tutorial_Hint_Crops );
	s_completableTasks.push_back( e_Tutorial_Hint_Farmland );
	s_completableTasks.push_back( e_Tutorial_Hint_Furnace );
	s_completableTasks.push_back( e_Tutorial_Hint_Sign );
	s_completableTasks.push_back( e_Tutorial_Hint_Door_Wood );
	s_completableTasks.push_back( e_Tutorial_Hint_Ladder );
	s_completableTasks.push_back( e_Tutorial_Hint_Rail );
	s_completableTasks.push_back( e_Tutorial_Hint_Stairs_Stone );
	s_completableTasks.push_back( e_Tutorial_Hint_Lever );
	s_completableTasks.push_back( e_Tutorial_Hint_PressurePlate );
	s_completableTasks.push_back( e_Tutorial_Hint_Door_Iron );
	s_completableTasks.push_back( e_Tutorial_Hint_Redstone_Ore );
	s_completableTasks.push_back( e_Tutorial_Hint_Redstone_Torch );
	s_completableTasks.push_back( e_Tutorial_Hint_Button );
	s_completableTasks.push_back( e_Tutorial_Hint_Snow );
	s_completableTasks.push_back( e_Tutorial_Hint_Ice );
	s_completableTasks.push_back( e_Tutorial_Hint_Cactus );
	s_completableTasks.push_back( e_Tutorial_Hint_Clay );
	s_completableTasks.push_back( e_Tutorial_Hint_Sugarcane );
	s_completableTasks.push_back( e_Tutorial_Hint_Record_Player );
	s_completableTasks.push_back( e_Tutorial_Hint_Pumpkin );
	s_completableTasks.push_back( e_Tutorial_Hint_Hell_Rock );
	s_completableTasks.push_back( e_Tutorial_Hint_Hell_Sand );
	s_completableTasks.push_back( e_Tutorial_Hint_Glowstone );
	s_completableTasks.push_back( e_Tutorial_Hint_Portal );
	s_completableTasks.push_back( e_Tutorial_Hint_Pumpkin_Lit );
	s_completableTasks.push_back( e_Tutorial_Hint_Cake );
	s_completableTasks.push_back( e_Tutorial_Hint_Redstone_Repeater );
	s_completableTasks.push_back( e_Tutorial_Hint_Trapdoor );
	s_completableTasks.push_back( e_Tutorial_Hint_Piston );
	s_completableTasks.push_back( e_Tutorial_Hint_Sticky_Piston );
	s_completableTasks.push_back( e_Tutorial_Hint_Monster_Stone_Egg );
	s_completableTasks.push_back( e_Tutorial_Hint_Stone_Brick_Smooth );
	s_completableTasks.push_back( e_Tutorial_Hint_Huge_Mushroom );
	s_completableTasks.push_back( e_Tutorial_Hint_Iron_Fence );
	s_completableTasks.push_back( e_Tutorial_Hint_Thin_Glass );
	s_completableTasks.push_back( e_Tutorial_Hint_Melon );
	s_completableTasks.push_back( e_Tutorial_Hint_Vine );
	s_completableTasks.push_back( e_Tutorial_Hint_Fence_Gate );
	s_completableTasks.push_back( e_Tutorial_Hint_Mycel );
	s_completableTasks.push_back( e_Tutorial_Hint_Water_Lily );
	s_completableTasks.push_back( e_Tutorial_Hint_Nether_Brick );
	s_completableTasks.push_back( e_Tutorial_Hint_Nether_Fence );
	s_completableTasks.push_back( e_Tutorial_Hint_Nether_Stalk );
	s_completableTasks.push_back( e_Tutorial_Hint_Enchant_Table );
	s_completableTasks.push_back( e_Tutorial_Hint_Brewing_Stand );
	s_completableTasks.push_back( e_Tutorial_Hint_Cauldron );
	s_completableTasks.push_back( e_Tutorial_Hint_End_Portal );
	s_completableTasks.push_back( e_Tutorial_Hint_End_Portal_Frame );

	s_completableTasks.push_back( e_Tutorial_Hint_Squid );
	s_completableTasks.push_back( e_Tutorial_Hint_Cow );
	s_completableTasks.push_back( e_Tutorial_Hint_Sheep );
	s_completableTasks.push_back( e_Tutorial_Hint_Chicken );
	s_completableTasks.push_back( e_Tutorial_Hint_Pig );
	s_completableTasks.push_back( e_Tutorial_Hint_Wolf );
	s_completableTasks.push_back( e_Tutorial_Hint_Creeper );
	s_completableTasks.push_back( e_Tutorial_Hint_Skeleton );
	s_completableTasks.push_back( e_Tutorial_Hint_Spider );
	s_completableTasks.push_back( e_Tutorial_Hint_Zombie );
	s_completableTasks.push_back( e_Tutorial_Hint_Pig_Zombie );
	s_completableTasks.push_back( e_Tutorial_Hint_Ghast );
	s_completableTasks.push_back( e_Tutorial_Hint_Slime );
	s_completableTasks.push_back( e_Tutorial_Hint_Enderman );
	s_completableTasks.push_back( e_Tutorial_Hint_Silverfish );
	s_completableTasks.push_back( e_Tutorial_Hint_Cave_Spider );
	s_completableTasks.push_back( e_Tutorial_Hint_MushroomCow );
	s_completableTasks.push_back( e_Tutorial_Hint_SnowMan );
	s_completableTasks.push_back( e_Tutorial_Hint_IronGolem );
	s_completableTasks.push_back( e_Tutorial_Hint_EnderDragon );
	s_completableTasks.push_back( e_Tutorial_Hint_Blaze );
	s_completableTasks.push_back( e_Tutorial_Hint_Lava_Slime );

	s_completableTasks.push_back( e_Tutorial_Hint_Ozelot );
	s_completableTasks.push_back( e_Tutorial_Hint_Villager );

	s_completableTasks.push_back( e_Tutorial_Hint_Item_Shovel );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Hatchet );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Pickaxe );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Flint_And_Steel );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Apple );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Bow );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Arrow );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Coal );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Diamond );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Iron_Ingot );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Gold_Ingot );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Sword );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Stick );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Bowl );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Mushroom_Stew );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_String );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Feather );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Sulphur );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Hoe );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Seeds );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Wheat );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Bread );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Helmet );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Chestplate );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Leggings );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Boots );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Flint );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Porkchop_Raw );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Porkchop_Cooked );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Painting );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Apple_Gold );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Sign );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Door_Wood );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Bucket_Empty );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Bucket_Water );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Bucket_Lava );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Minecart );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Saddle );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Door_Iron );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Redstone );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Snowball );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Boat );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Leather );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Milk );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Brick );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Clay );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Reeds );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Paper );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Book );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Slimeball );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Minecart_Chest );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Minecart_Furnace );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Egg );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Compass );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Clock );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Yellow_Dust );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Fish_Raw );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Fish_Cooked );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Dye_Powder );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Bone );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Sugar );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Cake );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Diode );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Cookie );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Map );
	s_completableTasks.push_back( e_Tutorial_Hint_Item_Record );

	s_completableTasks.push_back( e_Tutorial_Hint_White_Stone );
	s_completableTasks.push_back( e_Tutorial_Hint_Dragon_Egg );
	s_completableTasks.push_back( e_Tutorial_Hint_RedstoneLamp );
	s_completableTasks.push_back( e_Tutorial_Hint_Cocoa);

	s_completableTasks.push_back( e_Tutorial_Hint_EmeraldOre );
	s_completableTasks.push_back( e_Tutorial_Hint_EmeraldBlock );
	s_completableTasks.push_back( e_Tutorial_Hint_EnderChest );
	s_completableTasks.push_back( e_Tutorial_Hint_TripwireSource );
	s_completableTasks.push_back( e_Tutorial_Hint_Tripwire );
	s_completableTasks.push_back( e_Tutorial_Hint_CobblestoneWall );
	s_completableTasks.push_back( e_Tutorial_Hint_Flowerpot );
	s_completableTasks.push_back( e_Tutorial_Hint_Anvil );
	s_completableTasks.push_back( e_Tutorial_Hint_QuartzOre );
	s_completableTasks.push_back( e_Tutorial_Hint_QuartzBlock );
	s_completableTasks.push_back( e_Tutorial_Hint_WoolCarpet );

	s_completableTasks.push_back( e_Tutorial_Hint_Potato );
	s_completableTasks.push_back( e_Tutorial_Hint_Carrot );

	s_completableTasks.push_back( e_Tutorial_Hint_CommandBlock );
	s_completableTasks.push_back( e_Tutorial_Hint_Beacon );
	s_completableTasks.push_back( e_Tutorial_Hint_Activator_Rail );

	s_completableTasks.push_back( eTutorial_Telemetry_TrialStart );
	s_completableTasks.push_back( eTutorial_Telemetry_Halfway );
	s_completableTasks.push_back( eTutorial_Telemetry_Complete );

	s_completableTasks.push_back( eTutorial_Telemetry_Unused_1 );
	s_completableTasks.push_back( eTutorial_Telemetry_Unused_2 );
	s_completableTasks.push_back( eTutorial_Telemetry_Unused_3 );
	s_completableTasks.push_back( eTutorial_Telemetry_Unused_4 );
	s_completableTasks.push_back( eTutorial_Telemetry_Unused_5 );
	s_completableTasks.push_back( eTutorial_Telemetry_Unused_6 );
	s_completableTasks.push_back( eTutorial_Telemetry_Unused_7 );
	s_completableTasks.push_back( eTutorial_Telemetry_Unused_8 );
	s_completableTasks.push_back( eTutorial_Telemetry_Unused_9 );
	s_completableTasks.push_back( eTutorial_Telemetry_Unused_10 );

	s_completableTasks.push_back( e_Tutorial_State_Enchanting_Menu );
	s_completableTasks.push_back( e_Tutorial_State_Farming );
	s_completableTasks.push_back( e_Tutorial_State_Breeding );
	s_completableTasks.push_back( e_Tutorial_State_Golem );
	s_completableTasks.push_back( e_Tutorial_State_Trading );
	s_completableTasks.push_back( e_Tutorial_State_Trading_Menu );
	s_completableTasks.push_back( e_Tutorial_State_Anvil );
	s_completableTasks.push_back( e_Tutorial_State_Anvil_Menu );
	s_completableTasks.push_back( e_Tutorial_State_Enderchests );
	s_completableTasks.push_back( e_Tutorial_State_Horse_Menu );
	s_completableTasks.push_back( e_Tutorial_State_Hopper_Menu );

	s_completableTasks.push_back( e_Tutorial_Hint_Wither );
	s_completableTasks.push_back( e_Tutorial_Hint_Witch );
	s_completableTasks.push_back( e_Tutorial_Hint_Bat );
	s_completableTasks.push_back( e_Tutorial_Hint_Horse );

	s_completableTasks.push_back( e_Tutorial_Hint_RedstoneBlock );
	s_completableTasks.push_back( e_Tutorial_Hint_DaylightDetector );
	s_completableTasks.push_back( e_Tutorial_Hint_Dropper );
	s_completableTasks.push_back( e_Tutorial_Hint_Hopper );
	s_completableTasks.push_back( e_Tutorial_Hint_Comparator );
	s_completableTasks.push_back( e_Tutorial_Hint_ChestTrap );
	s_completableTasks.push_back( e_Tutorial_Hint_HayBlock );
	s_completableTasks.push_back( e_Tutorial_Hint_ClayHardened );
	s_completableTasks.push_back( e_Tutorial_Hint_ClayHardenedColored );
	s_completableTasks.push_back( e_Tutorial_Hint_CoalBlock );

	s_completableTasks.push_back( e_Tutorial_State_Beacon_Menu );
	s_completableTasks.push_back( e_Tutorial_State_Fireworks_Menu );
	s_completableTasks.push_back( e_Tutorial_State_Horse );
	s_completableTasks.push_back( e_Tutorial_State_Hopper );
	s_completableTasks.push_back( e_Tutorial_State_Beacon );
	s_completableTasks.push_back( e_Tutorial_State_Fireworks );

	if( s_completableTasks.size() > TUTORIAL_PROFILE_STORAGE_BITS )
	{
		app.DebugPrintf("Warning: Too many tutorial completable tasks added, not enough bits allocated to stored them in the profile data");
		assert(false);
	}
}

Tutorial::Tutorial(int iPad, bool isFullTutorial /*= false*/) : m_iPad( iPad )
{
	m_isFullTutorial = isFullTutorial;
	m_fullTutorialComplete = false;
	m_allTutorialsComplete = false;
	hasRequestedUI = false;
	uiTempDisabled = false;
	m_hintDisplayed = false;
	m_freezeTime = false;
	m_timeFrozen = false;
	m_UIScene = nullptr;
	m_allowShow = true;
	m_bHasTickedOnce = false;
	m_firstTickTime = 0;

	m_lastMessage = nullptr;

	lastMessageTime = 0;
	m_iTaskReminders = 0;
	m_lastMessageState = e_Tutorial_State_Gameplay;

	m_CurrentState = e_Tutorial_State_Gameplay;
	m_hasStateChanged = false;
#ifdef _XBOX
	m_hTutorialScene=nullptr;
#endif

	for(unsigned int i = 0; i < e_Tutorial_State_Max; ++i)
	{
		currentTask[i] = nullptr;
		currentFailedConstraint[i] = nullptr;
	}

	// DEFAULT TASKS THAT ALL TUTORIALS SHARE
	/*
	*
	*
	* GAMEPLAY
	*
	*/

	if(!isHintCompleted(e_Tutorial_Hint_Hold_To_Mine)) addHint(e_Tutorial_State_Gameplay, new TutorialHint(e_Tutorial_Hint_Hold_To_Mine, this, IDS_TUTORIAL_HINT_HOLD_TO_MINE, TutorialHint::e_Hint_HoldToMine) );
	if(!isHintCompleted(e_Tutorial_Hint_Tool_Damaged)) addHint(e_Tutorial_State_Gameplay, new TutorialHint(e_Tutorial_Hint_Tool_Damaged, this, IDS_TUTORIAL_HINT_TOOL_DAMAGED, TutorialHint::e_Hint_ToolDamaged) );
	if(!isHintCompleted(e_Tutorial_Hint_Swim_Up)) addHint(e_Tutorial_State_Gameplay, new TutorialHint(e_Tutorial_Hint_Swim_Up, this, IDS_TUTORIAL_HINT_SWIM_UP, TutorialHint::e_Hint_SwimUp) );

	/*
	* TILE HINTS
	*/
	int rockItems[] = {Tile::stone_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Rock)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Rock, this, rockItems, 1 ) );

	int stoneItems[] = {Tile::cobblestone_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Stone)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Stone, this, stoneItems, 1 ) );

	int plankItems[] = {Tile::wood_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Planks)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Planks, this, plankItems, 1 ) );

	int saplingItems[] = {Tile::sapling_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Sapling)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Sapling, this, saplingItems, 1 ) );

	int unbreakableItems[] = {Tile::unbreakable_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Unbreakable)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Unbreakable, this, unbreakableItems, 1 ) );

	int waterItems[] = {Tile::water_Id, Tile::calmWater_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Water)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Water, this, waterItems, 2 ) );

	int lavaItems[] = {Tile::lava_Id, Tile::calmLava_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Lava)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Lava, this, lavaItems, 2 ) );

	int sandItems[] = {Tile::sand_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Sand)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Sand, this, sandItems, 1 ) );

	int gravelItems[] = {Tile::gravel_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Gravel)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Gravel, this, gravelItems, 1 ) );

	int goldOreItems[] = {Tile::goldOre_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Gold_Ore)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Gold_Ore, this, goldOreItems, 1 ) );

	int ironOreItems[] = {Tile::ironOre_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Iron_Ore)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Iron_Ore, this, ironOreItems, 1 ) );

	int coalOreItems[] = {Tile::coalOre_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Coal_Ore)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Coal_Ore, this, coalOreItems, 1 ) );

	int treeTrunkItems[] = {Tile::treeTrunk_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Tree_Trunk)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Tree_Trunk, this, treeTrunkItems, 1 ) );

	int leavesItems[] = {Tile::leaves_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Leaves)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Leaves, this, leavesItems, 1 ) );

	int glassItems[] = {Tile::glass_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Glass)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Glass, this, glassItems, 1 ) );

	int lapisOreItems[] = {Tile::lapisOre_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Lapis_Ore)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Lapis_Ore, this, lapisOreItems, 1 ) );

	int lapisBlockItems[] = {Tile::lapisBlock_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Lapis_Block)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Lapis_Block, this, lapisBlockItems, 1 ) );

	int dispenserItems[] = {Tile::dispenser_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Dispenser)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Dispenser, this, dispenserItems, 1 ) );

	int sandstoneItems[] = {Tile::sandStone_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Sandstone))
	{
		addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Sandstone, this, sandstoneItems, 1, -1, SandStoneTile::TYPE_DEFAULT ) );
		addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Sandstone, this, sandstoneItems, 1, -1, SandStoneTile::TYPE_HEIROGLYPHS ) );
		addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Sandstone, this, sandstoneItems, 1, -1, SandStoneTile::TYPE_SMOOTHSIDE ) );
	}

	int noteBlockItems[] = {Tile::noteblock_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Note_Block)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Note_Block, this, noteBlockItems, 1 ) );

	int poweredRailItems[] = {Tile::goldenRail_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Powered_Rail)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Powered_Rail, this, poweredRailItems, 1 ) );

	int detectorRailItems[] = {Tile::detectorRail_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Detector_Rail)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Detector_Rail, this, detectorRailItems, 1 ) );

	int tallGrassItems[] = {Tile::tallgrass_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Tall_Grass))
	{
		addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Tall_Grass, this, tallGrassItems, 1, -1, TallGrass::DEAD_SHRUB ) );
		addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Tall_Grass, this, tallGrassItems, 1, -1, TallGrass::TALL_GRASS ) );
		addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Tall_Grass, this, tallGrassItems, 1, -1, TallGrass::FERN ) );
	}

	int woolItems[] = {Tile::wool_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Wool)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Wool, this, woolItems, 1 ) );

	int flowerItems[] = {Tile::flower_Id, Tile::rose_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Flower)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Flower, this, flowerItems, 2 ) );

	int mushroomItems[] = {Tile::mushroom_brown_Id, Tile::mushroom_red_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Mushroom)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Mushroom, this, mushroomItems, 2 ) );

	int goldBlockItems[] = {Tile::goldBlock_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Gold_Block)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Gold_Block, this, goldBlockItems, 1 ) );

	int ironBlockItems[] = {Tile::ironBlock_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Iron_Block)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Iron_Block, this, ironBlockItems, 1 ) );

	int stoneSlabItems[] = {Tile::stoneSlabHalf_Id, Tile::stoneSlab_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Stone_Slab))
	{
		addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Stone_Slab, this, stoneSlabItems, 2, -1, StoneSlabTile::STONE_SLAB ) );
		addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Stone_Slab, this, stoneSlabItems, 2, -1, StoneSlabTile::SAND_SLAB ) );
		addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Stone_Slab, this, stoneSlabItems, 2, -1, StoneSlabTile::WOOD_SLAB ) );
		addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Stone_Slab, this, stoneSlabItems, 2, -1, StoneSlabTile::COBBLESTONE_SLAB ) );
		addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Stone_Slab, this, stoneSlabItems, 2, -1, StoneSlabTile::BRICK_SLAB ) );
		addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Stone_Slab, this, stoneSlabItems, 2, -1, StoneSlabTile::SMOOTHBRICK_SLAB ) );
		addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Stone_Slab, this, stoneSlabItems, 2, -1, StoneSlabTile::NETHERBRICK_SLAB ) );
		addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Stone_Slab, this, stoneSlabItems, 2, -1, StoneSlabTile::QUARTZ_SLAB ) );
	}

	int woodSlabItems[] = {Tile::woodSlabHalf_Id, Tile::woodSlab_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Stone_Slab))
	{
		addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Stone_Slab, this, woodSlabItems, 2, -1, TreeTile::BIRCH_TRUNK ) );
		addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Stone_Slab, this, woodSlabItems, 2, -1, TreeTile::DARK_TRUNK ) );
	}

	int redBrickItems[] = {Tile::redBrick_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Red_Brick)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Red_Brick, this, redBrickItems, 1 ) );

	int tntItems[] = {Tile::tnt_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Tnt)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Tnt, this, tntItems, 1 ) );

	int bookshelfItems[] = {Tile::bookshelf_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Bookshelf)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Bookshelf, this, bookshelfItems, 1 ) );

	int mossStoneItems[] = {Tile::mossyCobblestone_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Moss_Stone)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Moss_Stone, this, mossStoneItems, 1 ) );

	int obsidianItems[] = {Tile::obsidian_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Obsidian)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Obsidian, this, obsidianItems, 1 ) );

	int torchItems[] = {Tile::torch_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Torch)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Torch, this, torchItems, 1 ) );

	int mobSpawnerItems[] = {Tile::mobSpawner_Id};
	if(!isHintCompleted(e_Tutorial_Hint_MobSpawner)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_MobSpawner, this, mobSpawnerItems, 1 ) );

	int chestItems[] = {Tile::chest_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Chest)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Chest, this, chestItems, 1 ) );

	int redstoneItems[] = {Tile::redStoneDust_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Redstone)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Redstone, this, redstoneItems, 1, Item::redStone_Id ) );

	int diamondOreItems[] = {Tile::diamondOre_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Diamond_Ore)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Diamond_Ore, this, diamondOreItems, 1 ) );

	int diamondBlockItems[] = {Tile::diamondBlock_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Diamond_Block)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Diamond_Block, this, diamondBlockItems, 1 ) );

	int craftingTableItems[] = {Tile::workBench_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Crafting_Table)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Crafting_Table, this, craftingTableItems, 1 ) );

	int cropsItems[] = {Tile::wheat_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Crops)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Crops, this, cropsItems, 1, -1, -1, 7 ) );

	int farmlandItems[] = {Tile::farmland_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Farmland)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Farmland, this, farmlandItems, 1 ) );

	int furnaceItems[] = {Tile::furnace_Id, Tile::furnace_lit_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Furnace)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Furnace, this, furnaceItems, 2 ) );

	int signItems[] = {Tile::sign_Id, Tile::wallSign_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Sign)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Sign, this, signItems, 2, Item::sign_Id ) );

	int doorWoodItems[] = {Tile::door_wood_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Door_Wood)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Door_Wood, this, doorWoodItems, 1, Item::door_wood->id ) );

	int ladderItems[] = {Tile::ladder_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Ladder)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Ladder, this, ladderItems, 1 ) );

	int stairsStoneItems[] = {Tile::stairs_stone_Id,Tile::stairs_bricks_Id,Tile::stairs_stoneBrick_Id,Tile::stairs_wood_Id,Tile::stairs_sprucewood_Id,Tile::stairs_birchwood_Id,Tile::stairs_netherBricks_Id,Tile::stairs_sandstone_Id,Tile::stairs_quartz_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Stairs_Stone)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Stairs_Stone, this, stairsStoneItems, 9 ) );

	int railItems[] = {Tile::rail_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Rail)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Rail, this, railItems, 1 ) );

	int leverItems[] = {Tile::lever_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Lever)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Lever, this, leverItems, 1 ) );

	int pressurePlateItems[] = {Tile::pressurePlate_stone_Id, Tile::pressurePlate_wood_Id};
	if(!isHintCompleted(e_Tutorial_Hint_PressurePlate)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_PressurePlate, this, pressurePlateItems, 2 ) );

	int doorIronItems[] = {Tile::door_iron_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Door_Iron)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Door_Iron, this, doorIronItems, 1, Item::door_iron->id ) );

	int redstoneOreItems[] = {Tile::redStoneOre_Id, Tile::redStoneOre_lit_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Redstone_Ore)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Redstone_Ore, this, redstoneOreItems, 2 ) );

	int redstoneTorchItems[] = {Tile::redstoneTorch_off_Id, Tile::redstoneTorch_on_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Redstone_Torch)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Redstone_Torch, this, redstoneTorchItems, 2 ) );

	int buttonItems[] = {Tile::button_stone_Id, Tile::button_wood_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Button)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Button, this, buttonItems, 2 ) );

	int snowItems[] = {Tile::snow_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Snow)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Snow, this, snowItems, 1 ) );

	int iceItems[] = {Tile::ice_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Ice)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Ice, this, iceItems, 1 ) );

	int cactusItems[] = {Tile::cactus_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Cactus)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Cactus, this, cactusItems, 1 ) );

	int clayItems[] = {Tile::clay_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Clay)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Clay, this, clayItems, 1 ) );

	int sugarCaneItems[] = {Tile::reeds_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Sugarcane)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Sugarcane, this, sugarCaneItems, 1 ) );

	int recordPlayerItems[] = {Tile::jukebox_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Record_Player)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Record_Player, this, recordPlayerItems, 1 ) );

	int pumpkinItems[] = {Tile::pumpkin_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Pumpkin)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Pumpkin, this, pumpkinItems, 1, -1, -1, 0 ) );

	int hellRockItems[] = {Tile::netherRack_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Hell_Rock)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Hell_Rock, this, hellRockItems, 1 ) );

	int hellSandItems[] = {Tile::soulsand_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Hell_Sand)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Hell_Sand, this, hellSandItems, 1 ) );

	int glowstoneItems[] = {Tile::glowstone_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Glowstone)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Glowstone, this, glowstoneItems, 1 ) );

	int portalItems[] = {Tile::portalTile_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Portal)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Portal, this, portalItems, 1 ) );

	int pumpkinLitItems[] = {Tile::litPumpkin_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Pumpkin_Lit)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Pumpkin_Lit, this, pumpkinLitItems, 1, -1, -1, 0  ) );

	int cakeItems[] = {Tile::cake_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Cake)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Cake, this, cakeItems, 1 ) );

	int redstoneRepeaterItems[] = {Tile::diode_on_Id, Tile::diode_off_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Redstone_Repeater)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Redstone_Repeater, this, redstoneRepeaterItems, 2, Item::repeater_Id ) );

	int trapdoorItems[] = {Tile::trapdoor_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Trapdoor)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Trapdoor, this, trapdoorItems, 1 ) );

	int pistonItems[] = {Tile::pistonBase_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Piston)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Piston, this, pistonItems, 1 ) );

	int stickyPistonItems[] = {Tile::pistonStickyBase_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Sticky_Piston)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Sticky_Piston, this, stickyPistonItems, 1 ) );

	int monsterStoneEggItems[] = {Tile::monsterStoneEgg_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Monster_Stone_Egg)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Monster_Stone_Egg, this, monsterStoneEggItems, 1 ) );

	int stoneBrickSmoothItems[] = {Tile::stoneBrick_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Stone_Brick_Smooth)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Stone_Brick_Smooth, this, stoneBrickSmoothItems, 1 ) );

	int hugeMushroomItems[] = {Tile::hugeMushroom_brown_Id,Tile::hugeMushroom_red_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Huge_Mushroom)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Huge_Mushroom, this, hugeMushroomItems, 2 ) );

	int ironFenceItems[] = {Tile::ironFence_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Iron_Fence)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Iron_Fence, this, ironFenceItems, 1 ) );

	int thisGlassItems[] = {Tile::thinGlass_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Thin_Glass)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Thin_Glass, this, thisGlassItems, 1 ) );

	int melonItems[] = {Tile::melon_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Melon)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Melon, this, melonItems, 1 ) );

	int vineItems[] = {Tile::vine_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Vine)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Vine, this, vineItems, 1 ) );

	int fenceGateItems[] = {Tile::fenceGate_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Fence_Gate)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Fence_Gate, this, fenceGateItems, 1 ) );

	int mycelItems[] = {Tile::mycel_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Mycel)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Mycel, this, mycelItems, 1 ) );

	int waterLilyItems[] = {Tile::waterLily_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Water_Lily)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Water_Lily, this, waterLilyItems, 1 ) );

	int netherBrickItems[] = {Tile::netherBrick_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Nether_Brick)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Nether_Brick, this, netherBrickItems, 1 ) );

	int netherFenceItems[] = {Tile::netherFence_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Nether_Fence)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Nether_Fence, this, netherFenceItems, 1 ) );

	int netherStalkItems[] = {Tile::netherStalk_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Nether_Stalk)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Nether_Stalk, this, netherStalkItems, 1 ) );

	int enchantTableItems[] = {Tile::enchantTable_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Enchant_Table)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Enchant_Table, this, enchantTableItems, 1 ) );

	int brewingStandItems[] = {Tile::brewingStand_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Brewing_Stand)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Brewing_Stand, this, brewingStandItems, 1, Item::brewingStand_Id ) );

	int cauldronItems[] = {Tile::cauldron_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Cauldron)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Cauldron, this, cauldronItems, 1, Item::cauldron_Id ) );

	int endPortalItems[] = {Tile::endPortalTile_Id};
	if(!isHintCompleted(e_Tutorial_Hint_End_Portal)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_End_Portal, this, endPortalItems, 1, -2 ) );

	int endPortalFrameItems[] = {Tile::endPortalFrameTile_Id};
	if(!isHintCompleted(e_Tutorial_Hint_End_Portal_Frame)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_End_Portal_Frame, this, endPortalFrameItems, 1 ) );

	int whiteStoneItems[] = {Tile::endStone_Id};
	if(!isHintCompleted(e_Tutorial_Hint_White_Stone)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_White_Stone, this, whiteStoneItems, 1 ) );

	int dragonEggItems[] = {Tile::dragonEgg_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Dragon_Egg)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Dragon_Egg, this, dragonEggItems, 1 ) );

	int redstoneLampItems[] = {Tile::redstoneLight_Id, Tile::redstoneLight_lit_Id};
	if(!isHintCompleted(e_Tutorial_Hint_RedstoneLamp)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_RedstoneLamp, this, redstoneLampItems, 2 ) );

	int cocoaItems[] = {Tile::cocoa_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Cocoa)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Cocoa, this, cocoaItems, 1, Item::dye_powder_Id, -1, DyePowderItem::BROWN) );

	int emeraldOreItems[] = {Tile::emeraldOre_Id};
	if(!isHintCompleted(e_Tutorial_Hint_EmeraldOre)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_EmeraldOre, this, emeraldOreItems, 1 ) );

	int emeraldBlockItems[] = {Tile::emeraldBlock_Id};
	if(!isHintCompleted(e_Tutorial_Hint_EmeraldBlock)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_EmeraldBlock, this, emeraldBlockItems, 1 ) );

	int enderChestItems[] = {Tile::enderChest_Id};
	if(!isHintCompleted(e_Tutorial_Hint_EnderChest)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_EnderChest, this, enderChestItems, 1 ) );

	int tripwireSourceItems[] = {Tile::tripWireSource_Id};
	if(!isHintCompleted(e_Tutorial_Hint_TripwireSource)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_TripwireSource, this, tripwireSourceItems, 1 ) );

	int tripwireItems[] = {Tile::tripWire_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Tripwire)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Tripwire, this, tripwireItems, 1, Item::string_Id ) );

	int cobblestoneWallItems[] = {Tile::cobbleWall_Id};
	if(!isHintCompleted(e_Tutorial_Hint_CobblestoneWall))
	{
		addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_CobblestoneWall, this, cobblestoneWallItems, 1, -1, WallTile::TYPE_NORMAL ) );
		addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_CobblestoneWall, this, cobblestoneWallItems, 1, -1, WallTile::TYPE_MOSSY ) );
	}

	int flowerpotItems[] = {Tile::flowerPot_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Flowerpot)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Flowerpot, this, flowerpotItems, 1, Item::flowerPot_Id ) );

	int anvilItems[] = {Tile::anvil_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Anvil)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Anvil, this, anvilItems, 1 ) );

	int quartzOreItems[] = {Tile::netherQuartz_Id};
	if(!isHintCompleted(e_Tutorial_Hint_QuartzOre)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_QuartzOre, this, quartzOreItems, 1 ) );

	int quartzBlockItems[] = {Tile::quartzBlock_Id};
	if(!isHintCompleted(e_Tutorial_Hint_QuartzBlock))
	{
		addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_QuartzBlock, this, quartzBlockItems, 1, -1, QuartzBlockTile::TYPE_DEFAULT ) );
		addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_QuartzBlock, this, quartzBlockItems, 1, -1, QuartzBlockTile::TYPE_CHISELED ) );
		addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_QuartzBlock, this, quartzBlockItems, 1, -1, QuartzBlockTile::TYPE_LINES_Y ) );
		addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_QuartzBlock, this, quartzBlockItems, 1, -1, QuartzBlockTile::TYPE_LINES_X ) );
		addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_QuartzBlock, this, quartzBlockItems, 1, -1, QuartzBlockTile::TYPE_LINES_Z ) );
	}

	int carpetItems[] = {Tile::woolCarpet_Id};
	if(!isHintCompleted(e_Tutorial_Hint_WoolCarpet)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_WoolCarpet, this, carpetItems, 1 ) );

	int potatoItems[] = {Tile::potatoes_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Potato)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Potato, this, potatoItems, 1, -1, -1, 7 ) );

	int carrotItems[] = {Tile::carrots_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Carrot)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Carrot, this, carrotItems, 1, -1, -1, 7 ) );

	int commandBlockItems[] = {Tile::commandBlock_Id};
	if(!isHintCompleted(e_Tutorial_Hint_CommandBlock)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_CommandBlock, this, commandBlockItems, 1 ) );

	int beaconItems[] = {Tile::beacon_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Beacon)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Beacon, this, beaconItems, 1 ) );

	int activatorRailItems[] = {Tile::activatorRail_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Activator_Rail)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Activator_Rail, this, activatorRailItems, 1 ) );

	int redstoneBlockItems[] = {Tile::redstoneBlock_Id};
	if(!isHintCompleted(e_Tutorial_Hint_RedstoneBlock)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_RedstoneBlock, this, redstoneBlockItems, 1 ) );

	int daylightDetectorItems[] = {Tile::daylightDetector_Id};
	if(!isHintCompleted(e_Tutorial_Hint_DaylightDetector)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_DaylightDetector, this, daylightDetectorItems, 1 ) );

	int dropperItems[] = {Tile::dropper_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Dropper)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Dropper, this, dropperItems, 1 ) );

	int hopperItems[] = {Tile::hopper_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Hopper)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Hopper, this, hopperItems, 1 ) );

	int comparatorItems[] = {Tile::comparator_off_Id, Tile::comparator_on_Id};
	if(!isHintCompleted(e_Tutorial_Hint_Comparator)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_Comparator, this, comparatorItems, 2, Item::comparator_Id ) );

	int trappedChestItems[] = {Tile::chest_trap_Id};
	if(!isHintCompleted(e_Tutorial_Hint_ChestTrap)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_ChestTrap, this, trappedChestItems, 1 ) );

	int hayBlockItems[] = {Tile::hayBlock_Id};
	if(!isHintCompleted(e_Tutorial_Hint_HayBlock)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_HayBlock, this, hayBlockItems, 1 ) );

	int clayHardenedItems[] = {Tile::clayHardened_Id};
	if(!isHintCompleted(e_Tutorial_Hint_ClayHardened)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_ClayHardened, this, clayHardenedItems, 1 ) );

	int clayHardenedColoredItems[] = {Tile::clayHardened_colored_Id};
	if(!isHintCompleted(e_Tutorial_Hint_ClayHardenedColored)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_ClayHardenedColored, this, clayHardenedColoredItems, 1 ) );

	int coalBlockItems[] = {Tile::coalBlock_Id};
	if(!isHintCompleted(e_Tutorial_Hint_CoalBlock)) addHint(e_Tutorial_State_Gameplay, new LookAtTileHint(e_Tutorial_Hint_CoalBlock, this, coalBlockItems, 1 ) );

	/*
	* ENTITY HINTS
	*/
	if(!isHintCompleted(e_Tutorial_Hint_Squid)) addHint(e_Tutorial_State_Gameplay, new LookAtEntityHint(e_Tutorial_Hint_Squid, this, IDS_DESC_SQUID, IDS_SQUID, eTYPE_SQUID ) );
	if(!isHintCompleted(e_Tutorial_Hint_Cow)) addHint(e_Tutorial_State_Gameplay, new LookAtEntityHint(e_Tutorial_Hint_Cow, this, IDS_DESC_COW, IDS_COW, eTYPE_COW ) );
	if(!isHintCompleted(e_Tutorial_Hint_Sheep)) addHint(e_Tutorial_State_Gameplay, new LookAtEntityHint(e_Tutorial_Hint_Sheep, this, IDS_DESC_SHEEP, IDS_SHEEP, eTYPE_SHEEP ) );
	if(!isHintCompleted(e_Tutorial_Hint_Chicken)) addHint(e_Tutorial_State_Gameplay, new LookAtEntityHint(e_Tutorial_Hint_Chicken, this, IDS_DESC_CHICKEN, IDS_CHICKEN, eTYPE_CHICKEN ) );
	if(!isHintCompleted(e_Tutorial_Hint_Pig)) addHint(e_Tutorial_State_Gameplay, new LookAtEntityHint(e_Tutorial_Hint_Pig, this, IDS_DESC_PIG, IDS_PIG, eTYPE_PIG ) );
	if(!isHintCompleted(e_Tutorial_Hint_Wolf)) addHint(e_Tutorial_State_Gameplay, new LookAtEntityHint(e_Tutorial_Hint_Wolf, this, IDS_DESC_WOLF, IDS_WOLF, eTYPE_WOLF ) );
	if(!isHintCompleted(e_Tutorial_Hint_Creeper)) addHint(e_Tutorial_State_Gameplay, new LookAtEntityHint(e_Tutorial_Hint_Creeper, this, IDS_DESC_CREEPER, IDS_CREEPER, eTYPE_CREEPER ) );
	if(!isHintCompleted(e_Tutorial_Hint_Skeleton)) addHint(e_Tutorial_State_Gameplay, new LookAtEntityHint(e_Tutorial_Hint_Skeleton, this, IDS_DESC_SKELETON, IDS_SKELETON, eTYPE_SKELETON ) );
	if(!isHintCompleted(e_Tutorial_Hint_Spider)) addHint(e_Tutorial_State_Gameplay, new LookAtEntityHint(e_Tutorial_Hint_Spider, this, IDS_DESC_SPIDER, IDS_SPIDER, eTYPE_SPIDER ) );
	if(!isHintCompleted(e_Tutorial_Hint_Zombie)) addHint(e_Tutorial_State_Gameplay, new LookAtEntityHint(e_Tutorial_Hint_Zombie, this, IDS_DESC_ZOMBIE, IDS_ZOMBIE, eTYPE_ZOMBIE ) );
	if(!isHintCompleted(e_Tutorial_Hint_Pig_Zombie)) addHint(e_Tutorial_State_Gameplay, new LookAtEntityHint(e_Tutorial_Hint_Pig_Zombie, this, IDS_DESC_PIGZOMBIE, IDS_PIGZOMBIE, eTYPE_PIGZOMBIE ) );
	if(!isHintCompleted(e_Tutorial_Hint_Ghast)) addHint(e_Tutorial_State_Gameplay, new LookAtEntityHint(e_Tutorial_Hint_Ghast, this, IDS_DESC_GHAST, IDS_GHAST, eTYPE_GHAST ) );
	if(!isHintCompleted(e_Tutorial_Hint_Slime)) addHint(e_Tutorial_State_Gameplay, new LookAtEntityHint(e_Tutorial_Hint_Slime, this, IDS_DESC_SLIME, IDS_SLIME, eTYPE_SLIME ) );
	if(!isHintCompleted(e_Tutorial_Hint_Enderman)) addHint(e_Tutorial_State_Gameplay, new LookAtEntityHint(e_Tutorial_Hint_Enderman, this, IDS_DESC_ENDERMAN, IDS_ENDERMAN, eTYPE_ENDERMAN ) );
	if(!isHintCompleted(e_Tutorial_Hint_Silverfish)) addHint(e_Tutorial_State_Gameplay, new LookAtEntityHint(e_Tutorial_Hint_Silverfish, this, IDS_DESC_SILVERFISH, IDS_SILVERFISH, eTYPE_SILVERFISH ) );
	if(!isHintCompleted(e_Tutorial_Hint_Cave_Spider)) addHint(e_Tutorial_State_Gameplay, new LookAtEntityHint(e_Tutorial_Hint_Cave_Spider, this, IDS_DESC_CAVE_SPIDER, IDS_CAVE_SPIDER, eTYPE_CAVESPIDER ) );
	if(!isHintCompleted(e_Tutorial_Hint_MushroomCow)) addHint(e_Tutorial_State_Gameplay, new LookAtEntityHint(e_Tutorial_Hint_MushroomCow, this, IDS_DESC_MUSHROOM_COW, IDS_MUSHROOM_COW, eTYPE_MUSHROOMCOW) );
	if(!isHintCompleted(e_Tutorial_Hint_SnowMan)) addHint(e_Tutorial_State_Gameplay, new LookAtEntityHint(e_Tutorial_Hint_SnowMan, this, IDS_DESC_SNOWMAN, IDS_SNOWMAN, eTYPE_SNOWMAN ) );
	if(!isHintCompleted(e_Tutorial_Hint_IronGolem)) addHint(e_Tutorial_State_Gameplay, new LookAtEntityHint(e_Tutorial_Hint_IronGolem, this, IDS_DESC_IRONGOLEM, IDS_IRONGOLEM, eTYPE_VILLAGERGOLEM ) );
	if(!isHintCompleted(e_Tutorial_Hint_EnderDragon)) addHint(e_Tutorial_State_Gameplay, new LookAtEntityHint(e_Tutorial_Hint_EnderDragon, this, IDS_DESC_ENDERDRAGON, IDS_ENDERDRAGON, eTYPE_ENDERDRAGON ) );
	if(!isHintCompleted(e_Tutorial_Hint_Blaze)) addHint(e_Tutorial_State_Gameplay, new LookAtEntityHint(e_Tutorial_Hint_Blaze, this, IDS_DESC_BLAZE, IDS_BLAZE, eTYPE_BLAZE ) );
	if(!isHintCompleted(e_Tutorial_Hint_Lava_Slime)) addHint(e_Tutorial_State_Gameplay, new LookAtEntityHint(e_Tutorial_Hint_Lava_Slime, this, IDS_DESC_LAVA_SLIME, IDS_LAVA_SLIME, eTYPE_LAVASLIME ) );
	if(!isHintCompleted(e_Tutorial_Hint_Ozelot)) addHint(e_Tutorial_State_Gameplay, new LookAtEntityHint(e_Tutorial_Hint_Ozelot, this, IDS_DESC_OZELOT, IDS_OZELOT, eTYPE_OCELOT ) );
	if(!isHintCompleted(e_Tutorial_Hint_Villager)) addHint(e_Tutorial_State_Gameplay, new LookAtEntityHint(e_Tutorial_Hint_Villager, this, IDS_DESC_VILLAGER, IDS_VILLAGER, eTYPE_VILLAGER) );
	if(!isHintCompleted(e_Tutorial_Hint_Wither)) addHint(e_Tutorial_State_Gameplay, new LookAtEntityHint(e_Tutorial_Hint_Wither, this, IDS_DESC_WITHER, IDS_WITHER, eTYPE_WITHERBOSS) );
	if(!isHintCompleted(e_Tutorial_Hint_Witch)) addHint(e_Tutorial_State_Gameplay, new LookAtEntityHint(e_Tutorial_Hint_Witch, this, IDS_DESC_WITCH, IDS_WITCH, eTYPE_WITCH) );
	if(!isHintCompleted(e_Tutorial_Hint_Bat)) addHint(e_Tutorial_State_Gameplay, new LookAtEntityHint(e_Tutorial_Hint_Bat, this, IDS_DESC_BAT, IDS_BAT, eTYPE_BAT) );
	if(!isHintCompleted(e_Tutorial_Hint_Horse)) addHint(e_Tutorial_State_Gameplay, new LookAtEntityHint(e_Tutorial_Hint_Horse, this, IDS_DESC_HORSE, IDS_HORSE, eTYPE_HORSE) );


	/*
	* ITEM HINTS
	*/
	int shovelItems[] = {Item::shovel_wood->id, Item::shovel_stone->id, Item::shovel_iron->id, Item::shovel_gold->id, Item::shovel_diamond->id};
	if(!isHintCompleted(e_Tutorial_Hint_Item_Shovel)) addHint(e_Tutorial_State_Gameplay, new DiggerItemHint(e_Tutorial_Hint_Item_Shovel, this, IDS_TUTORIAL_HINT_DIGGER_ITEM_SHOVEL, shovelItems, 5) );

	int hatchetItems[] = {Item::hatchet_wood->id, Item::hatchet_stone->id, Item::hatchet_iron->id, Item::hatchet_gold->id, Item::hatchet_diamond->id};
	if(!isHintCompleted(e_Tutorial_Hint_Item_Hatchet)) addHint(e_Tutorial_State_Gameplay, new DiggerItemHint(e_Tutorial_Hint_Item_Hatchet, this, IDS_TUTORIAL_HINT_DIGGER_ITEM_HATCHET, hatchetItems, 5 ) );

	int pickaxeItems[] = {Item::pickAxe_wood->id, Item::pickAxe_stone->id, Item::pickAxe_iron->id, Item::pickAxe_gold->id, Item::pickAxe_diamond->id};
	if(!isHintCompleted(e_Tutorial_Hint_Item_Pickaxe)) addHint(e_Tutorial_State_Gameplay, new DiggerItemHint(e_Tutorial_Hint_Item_Pickaxe, this, IDS_TUTORIAL_HINT_DIGGER_ITEM_PICKAXE, pickaxeItems, 5 ) );

	/*
	*
	*
	* INVENTORY
	*
	*/
	if(isFullTutorial || !isStateCompleted( e_Tutorial_State_Inventory_Menu) )
	{
		ProcedureCompoundTask *inventoryOverviewTask = new ProcedureCompoundTask( this );
		inventoryOverviewTask->AddTask( new ChoiceTask(this, IDS_TUTORIAL_TASK_INV_OVERVIEW, IDS_TUTORIAL_PROMPT_INV_OVERVIEW, true, ACTION_MENU_A, ACTION_MENU_B, e_Tutorial_Completion_Complete_State, eTelemetryTutorial_Inventory) );
		inventoryOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_INV_PICK_UP, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		inventoryOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_INV_MOVE, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		inventoryOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_INV_DROP, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		inventoryOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_INV_INFO, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		addTask(e_Tutorial_State_Inventory_Menu, inventoryOverviewTask );
	}

	/*
	*
	*
	* CREATIVE INVENTORY
	*
	*/
	if(isFullTutorial || !isStateCompleted( e_Tutorial_State_Creative_Inventory_Menu) )
	{
		ProcedureCompoundTask *creativeInventoryOverviewTask = new ProcedureCompoundTask( this );
		creativeInventoryOverviewTask->AddTask( new ChoiceTask(this, IDS_TUTORIAL_TASK_CREATIVE_INV_OVERVIEW, IDS_TUTORIAL_PROMPT_CREATIVE_INV_OVERVIEW, true, ACTION_MENU_A, ACTION_MENU_B, e_Tutorial_Completion_Complete_State, eTelemetryTutorial_CreativeInventory) );
		creativeInventoryOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_CREATIVE_INV_PICK_UP, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		creativeInventoryOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_CREATIVE_INV_MOVE, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		creativeInventoryOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_CREATIVE_INV_DROP, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		creativeInventoryOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_CREATIVE_INV_NAV, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		creativeInventoryOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_CREATIVE_INV_INFO, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		addTask(e_Tutorial_State_Creative_Inventory_Menu, creativeInventoryOverviewTask );
	}

	/*
	*
	*
	* CRAFTING
	*
	*/
	if(isFullTutorial || !isStateCompleted( e_Tutorial_State_2x2Crafting_Menu ) )
	{
		ProcedureCompoundTask *craftingOverviewTask = new ProcedureCompoundTask( this );
		craftingOverviewTask->AddTask( new ChoiceTask(this, IDS_TUTORIAL_TASK_CRAFT_OVERVIEW, IDS_TUTORIAL_PROMPT_CRAFT_OVERVIEW, true, ACTION_MENU_A, ACTION_MENU_B, e_Tutorial_Completion_Complete_State, eTelemetryTutorial_Crafting) );
		craftingOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_CRAFT_NAV, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		craftingOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_CRAFT_CREATE, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		craftingOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_CRAFT_CRAFT_TABLE, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		craftingOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_CRAFT_INVENTORY, IDS_TUTORIAL_PROMPT_PRESS_X_TO_TOGGLE_DESCRIPTION, false, ACTION_MENU_X) );
		craftingOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_CRAFT_DESCRIPTION, IDS_TUTORIAL_PROMPT_PRESS_X_TO_TOGGLE_INGREDIENTS, false, ACTION_MENU_X) );
		craftingOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_CRAFT_INGREDIENTS, IDS_TUTORIAL_PROMPT_PRESS_X_TO_TOGGLE_INVENTORY, false, ACTION_MENU_X) );
		addTask(e_Tutorial_State_2x2Crafting_Menu, craftingOverviewTask );
	}
	// Other tasks can be added in the derived classes

	addHint(e_Tutorial_State_2x2Crafting_Menu, new TutorialHint(e_Tutorial_Hint_Always_On, this, IDS_TUTORIAL_HINT_CRAFT_NO_INGREDIENTS, TutorialHint::e_Hint_NoIngredients) );

	addHint(e_Tutorial_State_3x3Crafting_Menu, new TutorialHint(e_Tutorial_Hint_Always_On, this, IDS_TUTORIAL_HINT_CRAFT_NO_INGREDIENTS, TutorialHint::e_Hint_NoIngredients) );

	/*
	*
	*
	* FURNACE
	*
	*/
	if(isFullTutorial || !isStateCompleted( e_Tutorial_State_Furnace_Menu ) )
	{
		ProcedureCompoundTask *furnaceOverviewTask = new ProcedureCompoundTask( this );
		furnaceOverviewTask->AddTask( new ChoiceTask(this, IDS_TUTORIAL_TASK_FURNACE_OVERVIEW, IDS_TUTORIAL_PROMPT_FURNACE_OVERVIEW, true, ACTION_MENU_A, ACTION_MENU_B, e_Tutorial_Completion_Complete_State, eTelemetryTutorial_Furnace) );
		furnaceOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_FURNACE_METHOD, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		furnaceOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_FURNACE_FUELS, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		furnaceOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_FURNACE_INGREDIENTS, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		addTask(e_Tutorial_State_Furnace_Menu, furnaceOverviewTask );
	}
	// Other tasks can be added in the derived classes

	/*
	*
	*
	* BREWING MENU
	*
	*/
	if(isFullTutorial || !isStateCompleted( e_Tutorial_State_Brewing_Menu ) )
	{
		ProcedureCompoundTask *brewingOverviewTask = new ProcedureCompoundTask( this );
		brewingOverviewTask->AddTask( new ChoiceTask(this, IDS_TUTORIAL_TASK_BREWING_MENU_OVERVIEW, IDS_TUTORIAL_PROMPT_BREWING_MENU_OVERVIEW, true, ACTION_MENU_A, ACTION_MENU_B, e_Tutorial_Completion_Complete_State, eTelemetryTutorial_BrewingMenu) );
		brewingOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_BREWING_MENU_METHOD, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		brewingOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_BREWING_MENU_BASIC_INGREDIENTS, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		brewingOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_BREWING_MENU_EXTENDED_INGREDIENTS, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		brewingOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_BREWING_MENU_EXTENDED_INGREDIENTS_2, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		addTask(e_Tutorial_State_Brewing_Menu, brewingOverviewTask );
	}
	// Other tasks can be added in the derived classes

	/*
	*
	*
	* ENCHANTING MENU
	*
	*/
	if(isFullTutorial || !isStateCompleted( e_Tutorial_State_Enchanting_Menu ) )
	{
		ProcedureCompoundTask *enchantingOverviewTask = new ProcedureCompoundTask( this );
		enchantingOverviewTask->AddTask( new ChoiceTask(this, IDS_TUTORIAL_TASK_ENCHANTING_MENU_OVERVIEW, IDS_TUTORIAL_PROMPT_ENCHANTING_MENU_OVERVIEW, true, ACTION_MENU_A, ACTION_MENU_B, e_Tutorial_Completion_Complete_State, eTelemetryTutorial_EnchantingMenu) );
		enchantingOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_ENCHANTING_MENU_START, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		enchantingOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_ENCHANTING_MENU_ENCHANTMENTS, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		enchantingOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_ENCHANTING_MENU_COST, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		enchantingOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_ENCHANTING_MENU_ENCHANT, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		enchantingOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_ENCHANTING_MENU_BETTER_ENCHANTMENTS, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		addTask(e_Tutorial_State_Enchanting_Menu, enchantingOverviewTask );
	}
	// Other tasks can be added in the derived classes

	/*
	*
	*
	* ANVIL MENU
	*
	*/
	if(isFullTutorial || !isStateCompleted( e_Tutorial_State_Anvil_Menu ) )
	{
		ProcedureCompoundTask *anvilOverviewTask = new ProcedureCompoundTask( this );
		anvilOverviewTask->AddTask( new ChoiceTask(this, IDS_TUTORIAL_TASK_ANVIL_MENU_OVERVIEW, IDS_TUTORIAL_PROMPT_ANVIL_MENU_OVERVIEW, true, ACTION_MENU_A, ACTION_MENU_B, e_Tutorial_Completion_Complete_State, eTelemetryTutorial_AnvilMenu) );
		anvilOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_ANVIL_MENU_START, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		anvilOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_ANVIL_MENU_REPAIR, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		anvilOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_ANVIL_MENU_SACRIFICE, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		anvilOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_ANVIL_MENU_ENCHANT, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		anvilOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_ANVIL_MENU_COST, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		anvilOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_ANVIL_MENU_RENAMING, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		anvilOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_ANVIL_MENU_SMITH, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		addTask(e_Tutorial_State_Anvil_Menu, anvilOverviewTask );
	}
	// Other tasks can be added in the derived classes

	/*
	*
	*
	* TRADING MENU
	*
	*/
	if(isFullTutorial || !isStateCompleted( e_Tutorial_State_Trading_Menu ) )
	{
		ProcedureCompoundTask *tradingOverviewTask = new ProcedureCompoundTask( this );
		tradingOverviewTask->AddTask( new ChoiceTask(this, IDS_TUTORIAL_TASK_TRADING_MENU_OVERVIEW, IDS_TUTORIAL_PROMPT_TRADING_MENU_OVERVIEW, true, ACTION_MENU_A, ACTION_MENU_B, e_Tutorial_Completion_Complete_State, eTelemetryTutorial_TradingMenu) );
		tradingOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_TRADING_MENU_START, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		tradingOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_TRADING_MENU_UNAVAILABLE, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		tradingOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_TRADING_MENU_DETAILS, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		tradingOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_TRADING_MENU_INVENTORY, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		tradingOverviewTask->AddTask( new InfoTask(this, IDS_TUTORIAL_TASK_TRADING_MENU_TRADE, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		addTask(e_Tutorial_State_Trading_Menu, tradingOverviewTask );
	}
	// Other tasks can be added in the derived classes

	/*
	*
	*
	* HORSE ENCOUNTER
	*
	*/
	if(isFullTutorial || !isStateCompleted(e_Tutorial_State_Horse) )
	{
		addTask(e_Tutorial_State_Horse,
			new HorseChoiceTask(this, IDS_TUTORIAL_TASK_HORSE_OVERVIEW,	IDS_TUTORIAL_TASK_DONKEY_OVERVIEW, IDS_TUTORIAL_TASK_MULE_OVERVIEW, IDS_TUTORIAL_PROMPT_HORSE_OVERVIEW,
			true, ACTION_MENU_A, ACTION_MENU_B, e_Tutorial_Completion_Complete_State_Gameplay_Constraints, eTelemetryTutorial_Horse) );

		addTask(e_Tutorial_State_Horse, new InfoTask(this, IDS_TUTORIAL_TASK_HORSE_INTRO,		IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		addTask(e_Tutorial_State_Horse, new InfoTask(this, IDS_TUTORIAL_TASK_HORSE_PURPOSE,		IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		addTask(e_Tutorial_State_Horse, new InfoTask(this, IDS_TUTORIAL_TASK_HORSE_TAMING,		IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		addTask(e_Tutorial_State_Horse, new InfoTask(this, IDS_TUTORIAL_TASK_HORSE_TAMING2,		IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );

		// 4J-JEV: Only force the RideEntityTask if we're on the full-tutorial.
		if (isFullTutorial)	addTask(e_Tutorial_State_Horse, new RideEntityTask(eTYPE_HORSE, this, IDS_TUTORIAL_TASK_HORSE_RIDE, true, nullptr, false, false, false) );
		else				addTask(e_Tutorial_State_Horse, new InfoTask(this, IDS_TUTORIAL_TASK_HORSE_RIDE,	IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );

		addTask(e_Tutorial_State_Horse, new InfoTask(this, IDS_TUTORIAL_TASK_HORSE_SADDLES,		IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		addTask(e_Tutorial_State_Horse, new InfoTask(this, IDS_TUTORIAL_TASK_HORSE_SADDLEBAGS,	IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		addTask(e_Tutorial_State_Horse, new InfoTask(this, IDS_TUTORIAL_TASK_HORSE_BREEDING,	IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
	}

	/*
	*
	*
	* HORSE MENU
	*
	*/
	if(isFullTutorial || !isStateCompleted( e_Tutorial_State_Horse_Menu ) )
	{
		ProcedureCompoundTask *horseMenuTask = new ProcedureCompoundTask( this );
		horseMenuTask->AddTask( new ChoiceTask(this,	IDS_TUTORIAL_TASK_HORSE_MENU_OVERVIEW,		IDS_TUTORIAL_PROMPT_HORSE_MENU_OVERVIEW,	true, ACTION_MENU_A, ACTION_MENU_B, e_Tutorial_Completion_Complete_State, eTelemetryTutorial_HorseMenu) );
		horseMenuTask->AddTask( new InfoTask(this,		IDS_TUTORIAL_TASK_HORSE_MENU_LAYOUT,		IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE,	true, ACTION_MENU_A) );
		horseMenuTask->AddTask( new InfoTask(this,		IDS_TUTORIAL_TASK_HORSE_MENU_EQUIPMENT,		IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE,	true, ACTION_MENU_A) );
		horseMenuTask->AddTask( new InfoTask(this,		IDS_TUTORIAL_TASK_HORSE_MENU_SADDLEBAGS,	IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE,	true, ACTION_MENU_A) );
		addTask(e_Tutorial_State_Horse_Menu, horseMenuTask );
	}

	/*
	*
	*
	* FIREWORKS MENU
	*
	*/
	if(isFullTutorial || !isStateCompleted( e_Tutorial_State_Fireworks_Menu ) )
	{
		ProcedureCompoundTask *fireworksMenuTask = new ProcedureCompoundTask( this );
		fireworksMenuTask->AddTask( new ChoiceTask(this,	IDS_TUTORIAL_TASK_FIREWORK_MENU_OVERVIEW,		IDS_TUTORIAL_PROMPT_FIREWORK_MENU_OVERVIEW,	true, ACTION_MENU_A, ACTION_MENU_B, e_Tutorial_Completion_Complete_State, eTelemetryTutorial_FireworksMenu) );
		fireworksMenuTask->AddTask( new InfoTask(this,		IDS_TUTORIAL_TASK_FIREWORK_MENU_BASIC_START,	IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE,	true, ACTION_MENU_A) );
		fireworksMenuTask->AddTask( new InfoTask(this,		IDS_TUTORIAL_TASK_FIREWORK_MENU_BASIC_STARS,	IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE,	true, ACTION_MENU_A) );
		fireworksMenuTask->AddTask( new InfoTask(this,		IDS_TUTORIAL_TASK_FIREWORK_MENU_BASIC_HEIGHT,	IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE,	true, ACTION_MENU_A) );
		fireworksMenuTask->AddTask( new InfoTask(this,		IDS_TUTORIAL_TASK_FIREWORK_MENU_BASIC_CRAFT,	IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE,	true, ACTION_MENU_A) );
		fireworksMenuTask->AddTask( new InfoTask(this,		IDS_TUTORIAL_TASK_FIREWORK_MENU_ADV_START,		IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE,	true, ACTION_MENU_A) );
		fireworksMenuTask->AddTask( new InfoTask(this,		IDS_TUTORIAL_TASK_FIREWORK_MENU_ADV_COLOUR,		IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE,	true, ACTION_MENU_A) );
		fireworksMenuTask->AddTask( new InfoTask(this,		IDS_TUTORIAL_TASK_FIREWORK_MENU_ADV_SHAPE,		IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE,	true, ACTION_MENU_A) );
		fireworksMenuTask->AddTask( new InfoTask(this,		IDS_TUTORIAL_TASK_FIREWORK_MENU_ADV_EFFECT,		IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE,	true, ACTION_MENU_A) );
		fireworksMenuTask->AddTask( new InfoTask(this,		IDS_TUTORIAL_TASK_FIREWORK_MENU_ADV_FADE,		IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE,	true, ACTION_MENU_A) );
		addTask(e_Tutorial_State_Fireworks_Menu, fireworksMenuTask );
	}

	/*
	*
	*
	* BEACON MENU
	*
	*/
	if(isFullTutorial || !isStateCompleted( e_Tutorial_State_Beacon_Menu ) )
	{
		ProcedureCompoundTask *beaconMenuTask = new ProcedureCompoundTask( this );
		beaconMenuTask->AddTask( new ChoiceTask(this,	IDS_TUTORIAL_TASK_BEACON_MENU_OVERVIEW,			IDS_TUTORIAL_PROMPT_BEACON_MENU_OVERVIEW,	true, ACTION_MENU_A, ACTION_MENU_B, e_Tutorial_Completion_Complete_State, eTelemetryTutorial_BeaconMenu) );
		beaconMenuTask->AddTask( new InfoTask(this,		IDS_TUTORIAL_TASK_BEACON_MENU_PRIMARY_POWERS,	IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE,	true, ACTION_MENU_A) );
		beaconMenuTask->AddTask( new InfoTask(this,		IDS_TUTORIAL_TASK_BEACON_MENU_SECONDARY_POWER,	IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE,	true, ACTION_MENU_A) );
		beaconMenuTask->AddTask( new InfoTask(this,		IDS_TUTORIAL_TASK_BEACON_MENU_ACTIVATION,		IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE,	true, ACTION_MENU_A) );
		addTask(e_Tutorial_State_Beacon_Menu, beaconMenuTask );
	}

	/*
	*
	*
	* MINECART
	*
	*/
	if(isFullTutorial || !isStateCompleted( e_Tutorial_State_Riding_Minecart ) )
	{
		addTask(e_Tutorial_State_Riding_Minecart, new ChoiceTask(this, IDS_TUTORIAL_TASK_MINECART_OVERVIEW, IDS_TUTORIAL_PROMPT_MINECART_OVERVIEW, true, ACTION_MENU_A, ACTION_MENU_B, e_Tutorial_Completion_Complete_State_Gameplay_Constraints, eTelemetryTutorial_Minecart) );
		addTask(e_Tutorial_State_Riding_Minecart, new InfoTask(this, IDS_TUTORIAL_TASK_MINECART_RAILS, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		addTask(e_Tutorial_State_Riding_Minecart, new InfoTask(this, IDS_TUTORIAL_TASK_MINECART_POWERED_RAILS, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		addTask(e_Tutorial_State_Riding_Minecart, new InfoTask(this, IDS_TUTORIAL_TASK_MINECART_PUSHING, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
	}

	/*
	*
	*
	* BOAT
	*
	*/
	if(isFullTutorial || !isStateCompleted( e_Tutorial_State_Riding_Boat ) )
	{
		addTask(e_Tutorial_State_Riding_Boat, new ChoiceTask(this, IDS_TUTORIAL_TASK_BOAT_OVERVIEW, IDS_TUTORIAL_PROMPT_BOAT_OVERVIEW, true, ACTION_MENU_A, ACTION_MENU_B, e_Tutorial_Completion_Complete_State_Gameplay_Constraints, eTelemetryTutorial_Boat) );
		addTask(e_Tutorial_State_Riding_Boat, new InfoTask(this, IDS_TUTORIAL_TASK_BOAT_STEER, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
	}

	/*
	*
	*
	* FISHING
	*
	*/
	if(isFullTutorial || !isStateCompleted( e_Tutorial_State_Fishing ) )
	{
		addTask(e_Tutorial_State_Fishing, new ChoiceTask(this, IDS_TUTORIAL_TASK_FISHING_OVERVIEW, IDS_TUTORIAL_PROMPT_FISHING_OVERVIEW, true, ACTION_MENU_A, ACTION_MENU_B, e_Tutorial_Completion_Complete_State_Gameplay_Constraints, eTelemetryTutorial_Fishing) );
		addTask(e_Tutorial_State_Fishing, new InfoTask(this, IDS_TUTORIAL_TASK_FISHING_CAST, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		addTask(e_Tutorial_State_Fishing, new InfoTask(this, IDS_TUTORIAL_TASK_FISHING_FISH, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		addTask(e_Tutorial_State_Fishing, new InfoTask(this, IDS_TUTORIAL_TASK_FISHING_USES, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
	}

	/*
	*
	*
	* BED
	*
	*/
	if(isFullTutorial || !isStateCompleted( e_Tutorial_State_Bed ) )
	{
		addTask(e_Tutorial_State_Bed, new ChoiceTask(this, IDS_TUTORIAL_TASK_BED_OVERVIEW, IDS_TUTORIAL_PROMPT_BED_OVERVIEW, true, ACTION_MENU_A, ACTION_MENU_B, e_Tutorial_Completion_Complete_State_Gameplay_Constraints, eTelemetryTutorial_Bed) );
		addTask(e_Tutorial_State_Bed, new InfoTask(this, IDS_TUTORIAL_TASK_BED_PLACEMENT, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		addTask(e_Tutorial_State_Bed, new InfoTask(this, IDS_TUTORIAL_TASK_BED_MULTIPLAYER, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
	}

	/*
	*
	*
	* FOOD BAR
	*
	*/
	if(!isFullTutorial && !isStateCompleted( e_Tutorial_State_Food_Bar ) )
	{
		addTask(e_Tutorial_State_Food_Bar, new ChoiceTask(this, IDS_TUTORIAL_TASK_FOOD_BAR_OVERVIEW, IDS_TUTORIAL_PROMPT_FOOD_BAR_OVERVIEW, true, ACTION_MENU_A, ACTION_MENU_B, e_Tutorial_Completion_Complete_State_Gameplay_Constraints, eTelemetryTutorial_FoodBar) );
		addTask(e_Tutorial_State_Food_Bar,  new InfoTask(this, IDS_TUTORIAL_TASK_FOOD_BAR_DEPLETE, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		addTask(e_Tutorial_State_Food_Bar,  new InfoTask(this, IDS_TUTORIAL_TASK_FOOD_BAR_HEAL, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
		addTask(e_Tutorial_State_Food_Bar,  new InfoTask(this, IDS_TUTORIAL_TASK_FOOD_BAR_FEED, IDS_TUTORIAL_PROMPT_PRESS_A_TO_CONTINUE, true, ACTION_MENU_A) );
	}
}

Tutorial::~Tutorial()
{
	for(auto& it : m_globalConstraints)
	{
		delete it;
	}
	for(auto& message : messages)
	{
		delete message.second;
	}
	for(unsigned int i = 0; i < e_Tutorial_State_Max; ++i)
	{
		for(auto& it : activeTasks[i])
		{
			delete it;
		}
		for(auto& it : hints[i])
		{
			delete it;
		}

		currentTask[i] = nullptr;
		currentFailedConstraint[i] = nullptr;
	}
}

void Tutorial::debugResetPlayerSavedProgress(int iPad)
{
#if ( defined  __PS3__ || defined __ORBIS__ || defined _DURANGO || defined __PSVITA__)
	GAME_SETTINGS *pGameSettings = (GAME_SETTINGS *)StorageManager.GetGameDefinedProfileData(iPad);
#else
	GAME_SETTINGS *pGameSettings = static_cast<GAME_SETTINGS *>(ProfileManager.GetGameDefinedProfileData(iPad));
#endif
	ZeroMemory( pGameSettings->ucTutorialCompletion, TUTORIAL_PROFILE_STORAGE_BYTES );
	pGameSettings->uiSpecialTutorialBitmask = 0;
}

void Tutorial::setCompleted( int completableId )
{
	//if(app.GetGameSettingsDebugMask(m_iPad) && app.GetGameSettingsDebugMask()&(1L<<eDebugSetting_TipsAlwaysOn))
	//{
	//	return;
	//}


	int completableIndex = -1;
    for (int task : s_completableTasks)
    {
		++completableIndex;
		if( task == completableId )
		{
			break;
		}
	}
	if( completableIndex >= 0 && completableIndex < TUTORIAL_PROFILE_STORAGE_BITS )
	{
		// Set the bit for this position
#if (defined __PS3__ || defined __ORBIS__ || defined _DURANGO  || defined __PSVITA__)
		GAME_SETTINGS *pGameSettings = (GAME_SETTINGS *)StorageManager.GetGameDefinedProfileData(m_iPad);
#else
		GAME_SETTINGS *pGameSettings = static_cast<GAME_SETTINGS *>(ProfileManager.GetGameDefinedProfileData(m_iPad));
#endif
		int arrayIndex = completableIndex >> 3;
		int bitIndex = 7 - (completableIndex % 8);
		pGameSettings->ucTutorialCompletion[arrayIndex] |= 1<<bitIndex;
		// flag a profile write needed
		pGameSettings->bSettingsChanged=true;
	}
}

bool Tutorial::getCompleted( int completableId )
{
	//if(app.GetGameSettingsDebugMask(m_iPad) && app.GetGameSettingsDebugMask()&(1L<<eDebugSetting_TipsAlwaysOn))
	//{
	//	return false;
	//}

	int completableIndex = -1;
    for (int it : s_completableTasks)
    {
		++completableIndex;
		if( it == completableId )
		{
			break;
		}
	}
	if( completableIndex >= 0 && completableIndex < TUTORIAL_PROFILE_STORAGE_BITS )
	{
		// Read the bit for this position
		//Retrieve the data pointer from the profile
#if ( defined __PS3__ || defined __ORBIS__ || defined _DURANGO  || defined __PSVITA__)
		GAME_SETTINGS *pGameSettings = (GAME_SETTINGS *)StorageManager.GetGameDefinedProfileData(m_iPad);
#else
		GAME_SETTINGS *pGameSettings = static_cast<GAME_SETTINGS *>(ProfileManager.GetGameDefinedProfileData(m_iPad));
#endif
		int arrayIndex = completableIndex >> 3;
		int bitIndex = 7 - (completableIndex % 8);
		return (pGameSettings->ucTutorialCompletion[arrayIndex] & 1<<bitIndex) == (1<<bitIndex);
	}
	else
	{
		return false;
	}
}

bool Tutorial::isStateCompleted( eTutorial_State state )
{
	return getCompleted(state);
}

void Tutorial::setStateCompleted( eTutorial_State state )
{
	setCompleted( state );
}

bool Tutorial::isHintCompleted( eTutorial_Hint hint )
{
	return getCompleted(hint);
}

void Tutorial::setHintCompleted( eTutorial_Hint hint )
{
	//if(app.GetGameSettingsDebugMask(m_iPad) && app.GetGameSettingsDebugMask()&(1L<<eDebugSetting_TipsAlwaysOn))
	//{
	//	return;
	//}

	setCompleted( hint );
}

void Tutorial::setHintCompleted( TutorialHint *hint )
{
	//if(app.GetGameSettingsDebugMask(m_iPad) && app.GetGameSettingsDebugMask()&(1L<<eDebugSetting_TipsAlwaysOn))
	//{
	//	return;
	//}

	eTutorial_Hint hintId = hint->getId();

	if( hintId != e_Tutorial_Hint_Always_On )
	{
		setHintCompleted( hint->getId() );
		hints[m_CurrentState].erase( find(hints[m_CurrentState].begin(), hints[m_CurrentState].end(), hint) );
		delete hint;
	}
	// 	else
	// 	{
	// 		find(hints[m_CurrentState].begin(), hints[m_CurrentState].end(), hint);
	// 	}
}

void Tutorial::tick()
{
	// Don't do anything for the first 2 seconds so that the loading screen is gone
	if(!m_bHasTickedOnce)
	{
		int time = GetTickCount();
		if(m_firstTickTime == 0)
		{
			m_firstTickTime = time;
		}
		else if ( time - m_firstTickTime > 1500 )
		{
			m_bHasTickedOnce = true;
		}
	}
	if(!m_bHasTickedOnce)
	{
		return;
	}

	bool constraintChanged = false;
	bool taskChanged = false;

	for(unsigned int state = 0; state < e_Tutorial_State_Max; ++state)
	{
        auto it = constraintsToRemove[state].begin();
        while(it != constraintsToRemove[state].end() )
		{
			++(*it).second;
			if( (*it).second > m_iTutorialConstraintDelayRemoveTicks )
			{
				TutorialConstraint *c = (*it).first;
				constraints[state].erase( find( constraints[state].begin(), constraints[state].end(), c) );
				c->setQueuedForRemoval(false);
				it = constraintsToRemove[state].erase( it );

				if( c->getDeleteOnDeactivate() )
				{
					delete c;
				}
			}
			else
			{
				++it;
			}
		}
	}

	// 4J Stu TODO - Make this a constraint
	Minecraft *pMinecraft = Minecraft::GetInstance();
	if(m_freezeTime && !m_timeFrozen && !m_fullTutorialComplete )
	{
		// Need to set the time on both levels to stop the flickering as the local level
		// tries to predict the time
		MinecraftServer::SetTimeOfDay(m_iTutorialFreezeTimeValue);
		pMinecraft->level->setDayTime(m_iTutorialFreezeTimeValue); // Always daytime
		app.SetGameHostOption(eGameHostOption_DoDaylightCycle,0);
		m_timeFrozen = true;
	}
	else if(m_freezeTime && m_timeFrozen && m_fullTutorialComplete)
	{
		MinecraftServer::SetTimeOfDay(m_iTutorialFreezeTimeValue);
		pMinecraft->level->setDayTime(m_iTutorialFreezeTimeValue);
		app.SetGameHostOption(eGameHostOption_DoDaylightCycle,1);
		m_timeFrozen = false;
	}

	if(!m_allowShow)
	{
		if( currentTask[m_CurrentState] != nullptr && (!currentTask[m_CurrentState]->AllowFade() || (lastMessageTime + m_iTutorialDisplayMessageTime ) > GetTickCount() ) )
		{
			uiTempDisabled = true;
		}
		ui.SetTutorialVisible( m_iPad, false );
		return;
	}


	if(!hasRequestedUI )
	{
#ifdef _XBOX
		m_bSceneIsSplitscreen=app.GetLocalPlayerCount()>1;
		if(m_bSceneIsSplitscreen)
		{
			app.NavigateToScene(m_iPad, eUIComponent_TutorialPopup,(void *)this, false, false, &m_hTutorialScene);
		}
		else
		{
			app.NavigateToScene(m_iPad, eUIComponent_TutorialPopup,(void *)this, false, false, &m_hTutorialScene);
		}
#else
		ui.SetTutorial(m_iPad, this);
#endif
		hasRequestedUI = true;
	}
	else
	{
		// if we've changed mode, we may need to change scene
		if(m_bSceneIsSplitscreen!=(app.GetLocalPlayerCount()>1))
		{
#ifdef _XBOX
			app.TutorialSceneNavigateBack(m_iPad);
			m_bSceneIsSplitscreen=app.GetLocalPlayerCount()>1;
			if(m_bSceneIsSplitscreen)
			{
				app.NavigateToScene(m_iPad, eUIComponent_TutorialPopup,(void *)this, false, false, &m_hTutorialScene);
			}
			else
			{
				app.NavigateToScene(m_iPad, eUIComponent_TutorialPopup,(void *)this, false, false, &m_hTutorialScene);
			}
#else
			ui.SetTutorial(m_iPad, this);
#endif
		}
	}

	if(ui.IsPauseMenuDisplayed( m_iPad ) )
	{
		if( currentTask[m_CurrentState] != nullptr && (!currentTask[m_CurrentState]->AllowFade() || (lastMessageTime + m_iTutorialDisplayMessageTime ) > GetTickCount() ) )
		{
			uiTempDisabled = true;
		}
		ui.SetTutorialVisible( m_iPad, false );
		return;
	}
	if( uiTempDisabled )
	{
		ui.SetTutorialVisible( m_iPad, true );
		lastMessageTime = GetTickCount();
		uiTempDisabled = false;
	}

	// Check constraints
    for (auto& constraint : m_globalConstraints)
    {
		constraint->tick(m_iPad);
	}

	// Check hints
	int hintNeeded = -1;
	if(!m_hintDisplayed)
	{
		// 4J Stu - TU-1 interim
		// Allow turning off all the hints
		bool hintsOn = m_isFullTutorial || app.GetGameSettings(m_iPad,eGameSetting_Hints);

		if(hintsOn)
		{
            for (auto& hint : hints[m_CurrentState])
            {
				hintNeeded = hint->tick();
				if(hintNeeded >= 0)
				{
					PopupMessageDetails *message = new PopupMessageDetails();
					message->m_messageId = hintNeeded;
					message->m_allowFade = hint->allowFade();
					message->m_forceDisplay = true;
					setMessage( hint, message );
					break;
				}
			}
		}
	}

	// Check constraints
	// Only need to update these if we aren't already failing something
	if( !m_allTutorialsComplete && (currentFailedConstraint[m_CurrentState] == nullptr || currentFailedConstraint[m_CurrentState]->isConstraintSatisfied(m_iPad)) )
	{
		if( currentFailedConstraint[m_CurrentState] != nullptr && currentFailedConstraint[m_CurrentState]->isConstraintSatisfied(m_iPad) )
		{
			constraintChanged = true;
			currentFailedConstraint[m_CurrentState] = nullptr;
		}
        for (auto& constraint : constraints[m_CurrentState])
        {
			if( !constraint->isConstraintSatisfied(m_iPad) && constraint->isConstraintRestrictive(m_iPad) )
			{
				constraintChanged = true;
				currentFailedConstraint[m_CurrentState] = constraint;
			}
		}
	}

	if( !m_allTutorialsComplete && currentFailedConstraint[m_CurrentState] == nullptr )
	{
		// Update tasks
		bool isCurrentTask = true;
        auto it = activeTasks[m_CurrentState].begin();
        while(activeTasks[m_CurrentState].size() > 0 && it != activeTasks[m_CurrentState].end())
		{
			TutorialTask *task = *it;
			if( isCurrentTask || task->isPreCompletionEnabled() )
			{
				isCurrentTask = false;
				if(
					( !task->ShowMinimumTime() || ( task->hasBeenActivated() && (lastMessageTime + m_iTutorialMinimumDisplayMessageTime ) < GetTickCount() ) )
					&& task->isCompleted()
					)
				{
					eTutorial_CompletionAction compAction = task->getCompletionAction();
					it = activeTasks[m_CurrentState].erase( it );
					delete task;
					task = nullptr;

					if( activeTasks[m_CurrentState].size() > 0 )
					{
						switch( compAction )
						{
						case e_Tutorial_Completion_Complete_State_Gameplay_Constraints:
							{
								// 4J Stu - Move the delayed constraints to the gameplay state so that they are in
								// effect for a bit longer
                                auto itCon = constraintsToRemove[m_CurrentState].begin();
                                while(itCon != constraintsToRemove[m_CurrentState].end() )
								{
									constraints[e_Tutorial_State_Gameplay].push_back(itCon->first);
									constraintsToRemove[e_Tutorial_State_Gameplay].push_back( pair<TutorialConstraint *, unsigned char>(itCon->first, itCon->second) );

									constraints[m_CurrentState].erase( find( constraints[m_CurrentState].begin(), constraints[m_CurrentState].end(), itCon->first) );
									itCon = constraintsToRemove[m_CurrentState].erase(itCon);
								}
							}
							// Fall through the the normal complete state
						case e_Tutorial_Completion_Complete_State:
                            for (auto& itRem : activeTasks[m_CurrentState])
                            {
								delete itRem;
							}
							activeTasks[m_CurrentState].clear();
							break;
						case e_Tutorial_Completion_Jump_To_Last_Task:
							{
								TutorialTask *lastTask = activeTasks[m_CurrentState].at( activeTasks[m_CurrentState].size() - 1 );
								activeTasks[m_CurrentState].pop_back();
								for(auto& itRem : activeTasks[m_CurrentState])
								{
									delete itRem;
								}
								activeTasks[m_CurrentState].clear();
								activeTasks[m_CurrentState].push_back( lastTask );
								it = activeTasks[m_CurrentState].begin();
							}
							break;
						case e_Tutorial_Completion_None:
						default:
							break;
						}
					}

					if( activeTasks[m_CurrentState].size() > 0 )
					{
						currentTask[m_CurrentState] = activeTasks[m_CurrentState][0];
						currentTask[m_CurrentState]->setAsCurrentTask();
					}
					else
					{
						setStateCompleted( m_CurrentState );

						currentTask[m_CurrentState] = nullptr;
					}
					taskChanged = true;

					// If we can complete this early, check if we can complete it right now
					if( currentTask[m_CurrentState] != nullptr && currentTask[m_CurrentState]->isPreCompletionEnabled() )
					{
						isCurrentTask = true;
					}
				}
				else
				{
					++it;
				}
				if( task != nullptr && task->ShowMinimumTime() && task->hasBeenActivated() && (lastMessageTime + m_iTutorialMinimumDisplayMessageTime ) < GetTickCount() )
				{
					task->setShownForMinimumTime();

					if( !m_hintDisplayed )
					{
						PopupMessageDetails *message = new PopupMessageDetails();
						message->m_messageId = task->getDescriptionId();
						message->m_promptId = task->getPromptId();
						message->m_allowFade = task->AllowFade();
						message->m_replaceCurrent = true;
						setMessage( message );
					}
				}
			}
			else
			{
				++it;
			}
		}

		if( currentTask[m_CurrentState] == nullptr && activeTasks[m_CurrentState].size() > 0 )
		{
			currentTask[m_CurrentState] = activeTasks[m_CurrentState][0];
			currentTask[m_CurrentState]->setAsCurrentTask();
			taskChanged = true;
		}
	}

	if(!m_allTutorialsComplete && (taskChanged || m_hasStateChanged) )
	{
		bool allComplete = true;
		for(unsigned int state = 0; state < e_Tutorial_State_Max; ++state)
		{
			if(activeTasks[state].size() > 0 )
			{
				allComplete = false;
				break;
			}
			if(state==e_Tutorial_State_Gameplay)
			{
				m_fullTutorialComplete = true;
				Minecraft::GetInstance()->playerLeftTutorial(m_iPad);
			}
		}
		if(allComplete)
			m_allTutorialsComplete = true;
	}

	if( constraintChanged || taskChanged || m_hasStateChanged ||
		(currentFailedConstraint[m_CurrentState] == nullptr && currentTask[m_CurrentState] != nullptr && (m_lastMessage == nullptr || currentTask[m_CurrentState]->getDescriptionId() != m_lastMessage->m_messageId) && !m_hintDisplayed)
		)
	{
		if( currentFailedConstraint[m_CurrentState] != nullptr )
		{
			PopupMessageDetails *message = new PopupMessageDetails();
			message->m_messageId = currentFailedConstraint[m_CurrentState]->getDescriptionId();
			message->m_allowFade = false;
			setMessage( message );
		}
		else if( currentTask[m_CurrentState] != nullptr )
		{
			PopupMessageDetails *message = new PopupMessageDetails();
			message->m_messageId = currentTask[m_CurrentState]->getDescriptionId();
			message->m_promptId = currentTask[m_CurrentState]->getPromptId();
			message->m_allowFade = currentTask[m_CurrentState]->AllowFade();
			setMessage( message );
			currentTask[m_CurrentState]->TaskReminders()? m_iTaskReminders = 1 : m_iTaskReminders = 0;
		}
		else
		{
			setMessage( nullptr );
		}
	}

	if(m_hintDisplayed && (lastMessageTime + m_iTutorialDisplayMessageTime ) < GetTickCount() )
	{
		m_hintDisplayed = false;
	}

	if( currentFailedConstraint[m_CurrentState] == nullptr && currentTask[m_CurrentState] != nullptr && (m_iTaskReminders!=0) && (lastMessageTime + (m_iTaskReminders * m_iTutorialReminderTime) ) < GetTickCount() )
	{
		// Reminder
		PopupMessageDetails *message = new PopupMessageDetails();
		message->m_messageId = currentTask[m_CurrentState]->getDescriptionId();
		message->m_promptId = currentTask[m_CurrentState]->getPromptId();
		message->m_allowFade = currentTask[m_CurrentState]->AllowFade();
		message->m_isReminder = true;
		setMessage( message );
		++m_iTaskReminders;
		if( m_iTaskReminders > 1 )
			m_iTaskReminders = 1;
	}

	m_hasStateChanged = false;

	// If we have completed this state, and it is one that occurs during normal gameplay then change back to the gameplay track
	if( m_CurrentState != e_Tutorial_State_Gameplay && activeTasks[m_CurrentState].size() == 0 && (isSelectedItemState() || !ui.GetMenuDisplayed(m_iPad) ) )
	{
		this->changeTutorialState( e_Tutorial_State_Gameplay );
	}
}

bool Tutorial::setMessage(PopupMessageDetails *message)
{
	if(message != nullptr && !message->m_forceDisplay &&
		m_lastMessageState == m_CurrentState &&
		message->isSameContent(m_lastMessage) &&
		( !message->m_isReminder || ( (lastMessageTime + m_iTutorialReminderTime ) > GetTickCount() && message->m_isReminder ) )
		)
	{
		delete message;
		return false;
	}

	if(message != nullptr && (message->m_messageId > 0 || !message->m_messageString.empty()) )
	{
		m_lastMessageState = m_CurrentState;

		if(!message->m_replaceCurrent) lastMessageTime = GetTickCount();

		wstring text;
		if(!message->m_messageString.empty())
		{
			text = message->m_messageString;
		}
		else
		{
            auto it = messages.find(message->m_messageId);
            if( it != messages.end() && it->second != nullptr )
			{
				TutorialMessage *messageString = it->second;
				text = wstring( messageString->getMessageForDisplay() );

				// 4J Stu - Quick fix for boat tutorial being incorrect
				if(message->m_messageId == IDS_TUTORIAL_TASK_BOAT_OVERVIEW)
				{
					text = replaceAll(text, L"{*CONTROLLER_ACTION_USE*}", L"{*CONTROLLER_ACTION_DISMOUNT*}");
				}
			}
			else
			{
				text = wstring( app.GetString(message->m_messageId) );

				// 4J Stu - Quick fix for boat tutorial being incorrect
				if(message->m_messageId == IDS_TUTORIAL_TASK_BOAT_OVERVIEW)
				{
					text = replaceAll(text, L"{*CONTROLLER_ACTION_USE*}", L"{*CONTROLLER_ACTION_DISMOUNT*}");
				}
			}
		}

		if(!message->m_promptString.empty())
		{
			text.append(message->m_promptString);
		}
		else if(message->m_promptId >= 0)
		{
            auto it = messages.find(message->m_promptId);
            if(it != messages.end() && it->second != nullptr)
			{
				TutorialMessage *prompt = it->second;
				text.append( prompt->getMessageForDisplay() );
			}
		}

		wstring title;
		TutorialPopupInfo popupInfo;
		popupInfo.interactScene = m_UIScene;
		popupInfo.desc = text.c_str();
		popupInfo.icon = message->m_icon;
		popupInfo.iAuxVal = message->m_iAuxVal;
		popupInfo.allowFade = message->m_allowFade;
		popupInfo.isReminder = message->m_isReminder;
		popupInfo.tutorial = this;
		if( !message->m_titleString.empty() || message->m_titleId > 0 )
		{
			if(message->m_titleString.empty()) title = wstring( app.GetString(message->m_titleId) );
			else title = message->m_titleString;

			popupInfo.title = title.c_str();
			ui.SetTutorialDescription( m_iPad, &popupInfo );
		}
		else
		{
			ui.SetTutorialDescription( m_iPad, &popupInfo );
		}
	}
	else if( (m_lastMessage != nullptr && m_lastMessage->m_messageId != -1) ) //&& (lastMessageTime + m_iTutorialReminderTime ) > GetTickCount() )
	{
		// This should cause the popup to dissappear
		TutorialPopupInfo popupInfo;
		popupInfo.interactScene = m_UIScene;
		popupInfo.tutorial = this;
		ui.SetTutorialDescription( m_iPad, &popupInfo );
	}

	if(m_lastMessage != nullptr) delete m_lastMessage;
	m_lastMessage = message;

	return true;
}

bool Tutorial::setMessage(TutorialHint *hint, PopupMessageDetails *message)
{
	// 4J Stu - TU-1 interim
	// Allow turning off all the hints
	bool hintsOn = m_isFullTutorial || (app.GetGameSettings(m_iPad,eGameSetting_Hints) && app.GetGameSettings(m_iPad,eGameSetting_DisplayHUD));

	bool messageShown = false;
	DWORD time = GetTickCount();
	if(message != nullptr && (message->m_forceDisplay || hintsOn) &&
		(!message->m_delay ||
		(
		(m_hintDisplayed && (time - m_lastHintDisplayedTime) > m_iTutorialHintDelayTime ) ||
		(!m_hintDisplayed && (time - lastMessageTime) > m_iTutorialMinimumDisplayMessageTime )
		)
		)
		)
	{
		messageShown = setMessage( message );

		if(messageShown)
		{
			m_lastHintDisplayedTime = time;
			m_hintDisplayed = true;
			if(hint!=nullptr) setHintCompleted( hint );
		}
	}
	return messageShown;
}

bool Tutorial::setMessage(const wstring &messageString, int icon, int auxValue)
{
	PopupMessageDetails *message = new PopupMessageDetails();
	message->m_messageString = messageString;
	message->m_icon = icon;
	message->m_iAuxVal = auxValue;
	message->m_forceDisplay = true;

	return setMessage(message);
}

void Tutorial::showTutorialPopup(bool show)
{
	m_allowShow = show;

	if(!show)
	{
		if( currentTask[m_CurrentState] != nullptr && (!currentTask[m_CurrentState]->AllowFade() || (lastMessageTime + m_iTutorialDisplayMessageTime ) > GetTickCount() ) )
		{
			uiTempDisabled = true;
		}
		ui.SetTutorialVisible( m_iPad, show );
	}
}

void Tutorial::useItemOn(Level *level, shared_ptr<ItemInstance> item, int x, int y, int z, bool bTestUseOnly)
{
	for(auto& task : activeTasks[m_CurrentState])
	{
		task->useItemOn(level, item, x, y, z, bTestUseOnly);
	}
}

void Tutorial::useItemOn(shared_ptr<ItemInstance> item, bool bTestUseOnly)
{
	for(auto& task : activeTasks[m_CurrentState])
	{
		task->useItem(item, bTestUseOnly);
	}
}

void Tutorial::completeUsingItem(shared_ptr<ItemInstance> item)
{
	for(auto task : activeTasks[m_CurrentState])
	{
		task->completeUsingItem(item);
	}

	// Fix for #46922 - TU5: UI: Player receives a reminder that he is hungry while "hunger bar" is full (triggered in split-screen mode)
	if(m_CurrentState != e_Tutorial_State_Gameplay)
	{
		for(auto task : activeTasks[e_Tutorial_State_Gameplay])
		{
			task->completeUsingItem(item);
		}
	}
}

void Tutorial::startDestroyBlock(shared_ptr<ItemInstance> item, Tile *tile)
{
	int hintNeeded = -1;
	for(auto& hint : hints[m_CurrentState])
	{
		hintNeeded =  hint->startDestroyBlock(item, tile);
		if(hintNeeded >= 0)
		{
			PopupMessageDetails *message = new PopupMessageDetails();
			message->m_messageId = hintNeeded;
			setMessage( hint, message );
			break;
		}
	}
}

void Tutorial::destroyBlock(Tile *tile)
{
	int hintNeeded = -1;
	for(auto& hint : hints[m_CurrentState])
	{
		hintNeeded =  hint->destroyBlock(tile);
		if(hintNeeded >= 0)
		{
			PopupMessageDetails *message = new PopupMessageDetails();
			message->m_messageId = hintNeeded;
			setMessage( hint, message );
			break;
		}
	}
}

void Tutorial::attack(shared_ptr<Player> player, shared_ptr<Entity> entity)
{
	int hintNeeded = -1;
	for(auto& hint : hints[m_CurrentState])
	{
		hintNeeded = hint->attack(player->inventory->getSelected(), entity);
		if(hintNeeded >= 0)
		{
			PopupMessageDetails *message = new PopupMessageDetails();
			message->m_messageId = hintNeeded;
			setMessage( hint, message );
			break;
		}

	}
}

void Tutorial::itemDamaged(shared_ptr<ItemInstance> item)
{
	int hintNeeded = -1;
	for(auto& hint : hints[m_CurrentState])
	{
		hintNeeded = hint->itemDamaged(item);
		if(hintNeeded >= 0)
		{
			PopupMessageDetails *message = new PopupMessageDetails();
			message->m_messageId = hintNeeded;
			setMessage( hint, message );
			break;
		}

	}
}

void Tutorial::handleUIInput(int iAction)
{
	if( m_hintDisplayed ) return;

	if(currentTask[m_CurrentState] != nullptr)
		currentTask[m_CurrentState]->handleUIInput(iAction);
}

void Tutorial::createItemSelected(shared_ptr<ItemInstance> item, bool canMake)
{
	int hintNeeded = -1;
	for(auto& hint : hints[m_CurrentState])
	{
		hintNeeded = hint->createItemSelected(item, canMake);
		if(hintNeeded >= 0)
		{
			PopupMessageDetails *message = new PopupMessageDetails();
			message->m_messageId = hintNeeded;
			setMessage( hint, message );
			break;
		}

	}
}

void Tutorial::onCrafted(shared_ptr<ItemInstance> item)
{
	for(auto& subtasks : activeTasks)
	{
		for(auto& task : subtasks)
		{
			task->onCrafted(item);
		}
	}
}

void Tutorial::onTake(shared_ptr<ItemInstance> item, unsigned int invItemCountAnyAux, unsigned int invItemCountThisAux)
{
	if( !m_hintDisplayed )
	{
		bool hintNeeded = false;
		for(auto hint : hints[m_CurrentState])
		{
			hintNeeded = hint->onTake(item);
			if(hintNeeded)
			{
				break;
			}
		}
	}

	for(auto& subtasks : activeTasks)
	{
		for(auto& task : subtasks)
		{
			task->onTake(item, invItemCountAnyAux, invItemCountThisAux);
		}
	}
}

void Tutorial::onSelectedItemChanged(shared_ptr<ItemInstance> item)
{
	// We only handle this if we are in a state that allows changing based on the selected item
	// Menus and states like riding in a minecart will NOT allow this
	if( isSelectedItemState() )
	{
		if(item != nullptr)
		{
			switch(item->id)
			{
			case Item::fishingRod_Id:
				changeTutorialState(e_Tutorial_State_Fishing);
				break;
			default:
				changeTutorialState(e_Tutorial_State_Gameplay);
				break;
			}
		}
		else
		{
			changeTutorialState(e_Tutorial_State_Gameplay);
		}
	}
}

void Tutorial::onLookAt(int id, int iData)
{
	if( m_hintDisplayed ) return;

	bool hintNeeded = false;
	for(auto& hint : hints[m_CurrentState])
	{
		hintNeeded = hint->onLookAt(id, iData);
		if(hintNeeded)
		{
			break;
		}
	}

	if( m_CurrentState == e_Tutorial_State_Gameplay )
	{
		if(id > 0)
		{
			switch(id)
			{
			case Tile::bed_Id:
				changeTutorialState(e_Tutorial_State_Bed);
				break;
			default:
				break;
			}
		}
	}
}

void Tutorial::onLookAtEntity(shared_ptr<Entity> entity)
{
	if( m_hintDisplayed ) return;

	bool hintNeeded = false;
	for(auto& hint : hints[m_CurrentState])
	{
		hintNeeded = hint->onLookAtEntity(entity->GetType());
		if(hintNeeded)
		{
			break;
		}
	}

	if ( (m_CurrentState == e_Tutorial_State_Gameplay) && entity->instanceof(eTYPE_HORSE) )
	{
		changeTutorialState(e_Tutorial_State_Horse);
	}

	for (auto& it : activeTasks[m_CurrentState])
	{
		it->onLookAtEntity(entity);
	}
}

void Tutorial::onRideEntity(shared_ptr<Entity> entity)
{
	if(m_CurrentState == e_Tutorial_State_Gameplay)
	{
		switch (entity->GetType())
		{
		case eTYPE_MINECART:	changeTutorialState(e_Tutorial_State_Riding_Minecart);	break;
		case eTYPE_BOAT:		changeTutorialState(e_Tutorial_State_Riding_Boat);		break;
		}
	}

	for (auto& it : activeTasks[m_CurrentState])
	{
		it->onRideEntity(entity);
	}
}

void Tutorial::onEffectChanged(MobEffect *effect, bool bRemoved)
{
	for(auto& task : activeTasks[m_CurrentState])
	{
		task->onEffectChanged(effect,bRemoved);
	}
}

bool Tutorial::canMoveToPosition(double xo, double yo, double zo, double xt, double yt, double zt)
{
	bool allowed = true;
	for(auto& constraint : constraints[m_CurrentState])
	{
		if( !constraint->isConstraintSatisfied(m_iPad) && !constraint->canMoveToPosition(xo,yo,zo,xt,yt,zt) )
		{
			allowed = false;
			break;
		}
	}
	return allowed;
}

bool Tutorial::isInputAllowed(int mapping)
{
	if( m_hintDisplayed ) return true;

	// If the player is under water then allow all keypresses so they can jump out
	if( Minecraft::GetInstance()->localplayers[m_iPad]->isUnderLiquid(Material::water) ) return true;

	// If we're in a riding state but the player has dismounted, allow all input (covers race where
	// ride(nullptr) hasn't fired changeTutorialState yet, or alternate dismount paths).
	if( (m_CurrentState == e_Tutorial_State_Riding_Minecart || m_CurrentState == e_Tutorial_State_Riding_Boat) &&
		Minecraft::GetInstance()->localplayers[m_iPad]->riding == nullptr )
		return true;

	bool allowed = true;
	for(auto& constraint : constraints[m_CurrentState])
	{
		if( constraint->isMappingConstrained( m_iPad, mapping ) )
		{
			allowed = false;
			break;
		}
	}
	return allowed;
}

vector<TutorialTask *> *Tutorial::getTasks()
{
	return &tasks;
}

unsigned int Tutorial::getCurrentTaskIndex()
{
	unsigned int index = 0;
	for(const auto& task : tasks)
	{
		if(task == currentTask[e_Tutorial_State_Gameplay])
			break;

		++index;
	}
	return index;
}

void Tutorial::AddGlobalConstraint(TutorialConstraint *c)
{
	m_globalConstraints.push_back(c);
}

void Tutorial::AddConstraint(TutorialConstraint *c)
{
	constraints[m_CurrentState].push_back(c);
}

void Tutorial::RemoveConstraint(TutorialConstraint *c, bool delayedRemove /*= false*/)
{
	if( currentFailedConstraint[m_CurrentState] == c )
		currentFailedConstraint[m_CurrentState] = nullptr;

	if( c->getQueuedForRemoval() )
	{
		// If it is already queued for removal, remove it on the next tick
		/*for(auto& it : constraintsToRemove[m_CurrentState])
		{
		if( it.first == c )
		{
		it.second = m_iTutorialConstraintDelayRemoveTicks;
		break;
		}
		}*/
	}
	else if(delayedRemove)
	{
		c->setQueuedForRemoval(true);
		constraintsToRemove[m_CurrentState].emplace_back(c, 0);
	}
	else
	{
        for (auto it = constraintsToRemove[m_CurrentState].begin(); it != constraintsToRemove[m_CurrentState].end(); ++it)
        {
			if( it->first == c )
			{
				constraintsToRemove[m_CurrentState].erase( it );
				break;
			}
		}

        auto it = find(constraints[m_CurrentState].begin(), constraints[m_CurrentState].end(), c);
        if( it != constraints[m_CurrentState].end() ) constraints[m_CurrentState].erase( find( constraints[m_CurrentState].begin(), constraints[m_CurrentState].end(), c) );

		// It may be in the gameplay list, so remove it from there if it is
		it = find( constraints[e_Tutorial_State_Gameplay].begin(), constraints[e_Tutorial_State_Gameplay].end(), c);
		if( it != constraints[e_Tutorial_State_Gameplay].end() ) constraints[e_Tutorial_State_Gameplay].erase( find( constraints[e_Tutorial_State_Gameplay].begin(), constraints[e_Tutorial_State_Gameplay].end(), c) );
	}
}

void Tutorial::addTask(eTutorial_State state, TutorialTask *t)
{
	if( state == e_Tutorial_State_Gameplay )
	{
		tasks.push_back(t);
	}
	activeTasks[state].push_back(t);
}

void Tutorial::addHint(eTutorial_State state, TutorialHint *h)
{
	hints[state].push_back(h);
}

void Tutorial::addMessage(int messageId, bool limitRepeats /*= false*/, unsigned char numRepeats /*= TUTORIAL_MESSAGE_DEFAULT_SHOW*/)
{
	if(messageId >= 0 && messages.find(messageId)==messages.end())
		messages[messageId] = new TutorialMessage(messageId, limitRepeats, numRepeats);
}

#ifdef _XBOX
void Tutorial::changeTutorialState(eTutorial_State newState, CXuiScene *scene /*= nullptr*/)
#else
void Tutorial::changeTutorialState(eTutorial_State newState, UIScene *scene /*= nullptr*/)
#endif
{
	if(newState == m_CurrentState)
	{
		// If clearing the scene, make sure that the tutorial popup has its reference to this scene removed
#ifndef _XBOX
		if( scene == nullptr )
		{
			ui.RemoveInteractSceneReference(m_iPad, m_UIScene);
		}
#endif
		m_UIScene = scene;
		return;
	}
	// 4J Stu - TU-1 interim
	// Allow turning off all the hints
	bool hintsOn = m_isFullTutorial || app.GetGameSettings(m_iPad,eGameSetting_Hints);

	if(hintsOn)
	{
		// If we have completed this state, and it is one that occurs during normal gameplay then change back to the gameplay track
		if( newState != e_Tutorial_State_Gameplay && activeTasks[newState].size() == 0 && !ui.GetMenuDisplayed(m_iPad) )
		{
			return;
		}

		// The action that caused the change of state may also have completed the current task
		if( currentTask[m_CurrentState] != nullptr && currentTask[m_CurrentState]->isCompleted() )
		{
			activeTasks[m_CurrentState].erase( find( activeTasks[m_CurrentState].begin(), activeTasks[m_CurrentState].end(), currentTask[m_CurrentState]) );

			if( activeTasks[m_CurrentState].size() > 0 )
			{
				currentTask[m_CurrentState] = activeTasks[m_CurrentState][0];
				currentTask[m_CurrentState]->setAsCurrentTask();
			}
			else
			{
				currentTask[m_CurrentState] = nullptr;
			}
		}

		if( currentTask[m_CurrentState] != nullptr )
		{
			currentTask[m_CurrentState]->onStateChange(newState);
		}

		// Make sure that the current message is cleared
		setMessage( nullptr );

		// If clearing the scene, make sure that the tutorial popup has its reference to this scene removed
#ifndef _XBOX
		if( scene == nullptr )
		{
			ui.RemoveInteractSceneReference(m_iPad, m_UIScene);
		}
#endif
		m_UIScene = scene;


		if( m_CurrentState != newState )
		{
            for (auto& task : activeTasks[newState] )
            {
				task->onStateChange(newState);
			}
			m_CurrentState = newState;
			m_hasStateChanged = true;
			m_hintDisplayed = false;
		}
	}
}

bool Tutorial::isSelectedItemState()
{
	bool isSelectedItemState = false;
	switch(m_CurrentState)
	{
	case e_Tutorial_State_Gameplay:
	case e_Tutorial_State_Fishing:
		isSelectedItemState = true;
		break;
	default:
		break;
	}
	return isSelectedItemState;
}
