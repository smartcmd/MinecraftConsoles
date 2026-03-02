#include "stdafx.h"
#include "IUIScene_CreativeMenu.h"

#include "..\..\Minecraft.h"
#include "..\..\MultiplayerLocalPlayer.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.inventory.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.tile.entity.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.item.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.item.enchantment.h"

// 4J JEV - Images for each tab.
IUIScene_CreativeMenu::TabSpec **IUIScene_CreativeMenu::specs = NULL;

vector< shared_ptr<ItemInstance> > IUIScene_CreativeMenu::categoryGroups[eCreativeInventoryGroupsCount];

#define ITEM(id) list->push_back( shared_ptr<ItemInstance>(new ItemInstance(id, 1, 0)) );
#define ITEM_AUX(id, aux) list->push_back( shared_ptr<ItemInstance>(new ItemInstance(id, 1, aux)) );
#define DEF(index) list = &categoryGroups[index];


void IUIScene_CreativeMenu::staticCtor()
{
	vector< shared_ptr<ItemInstance> > *list;


	// Building Blocks
	DEF(eCreativeInventory_BuildingBlocks)
		ITEM(Tile::rock_Id)
		ITEM(Tile::grass_Id)
		ITEM(Tile::dirt_Id)
		ITEM(Tile::stoneBrick_Id)
		ITEM(Tile::sand_Id)
		ITEM(Tile::sandStone_Id)
		ITEM_AUX(Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE)
		ITEM_AUX(Tile::sandStone_Id, SandStoneTile::TYPE_HEIROGLYPHS)
		ITEM(Tile::goldBlock_Id)
		ITEM(Tile::ironBlock_Id)
		ITEM(Tile::lapisBlock_Id)
		ITEM(Tile::diamondBlock_Id)
		ITEM(Tile::emeraldBlock_Id)
		ITEM_AUX(Tile::quartzBlock_Id,QuartzBlockTile::TYPE_DEFAULT)
		ITEM(Tile::coalOre_Id)
		ITEM(Tile::lapisOre_Id)
		ITEM(Tile::diamondOre_Id)
		ITEM(Tile::redStoneOre_Id)
		ITEM(Tile::ironOre_Id)
		ITEM(Tile::goldOre_Id)
		ITEM(Tile::emeraldOre_Id)
		ITEM(Tile::netherQuartz_Id)
		ITEM(Tile::unbreakable_Id)
		ITEM_AUX(Tile::wood_Id,0)
		ITEM_AUX(Tile::wood_Id,TreeTile::DARK_TRUNK)
		ITEM_AUX(Tile::wood_Id,TreeTile::BIRCH_TRUNK)
		ITEM_AUX(Tile::wood_Id,TreeTile::JUNGLE_TRUNK)
		ITEM_AUX(Tile::treeTrunk_Id, 0)
		ITEM_AUX(Tile::treeTrunk_Id, TreeTile::DARK_TRUNK)
		ITEM_AUX(Tile::treeTrunk_Id, TreeTile::BIRCH_TRUNK)
		ITEM_AUX(Tile::treeTrunk_Id, TreeTile::JUNGLE_TRUNK)	
		ITEM(Tile::gravel_Id)
		ITEM(Tile::redBrick_Id)
		ITEM(Tile::mossStone_Id)
		ITEM(Tile::obsidian_Id)
		ITEM(Tile::clay)
		ITEM(Tile::ice_Id)
		ITEM(Tile::snow_Id)
		ITEM(Tile::hellRock_Id)
		ITEM(Tile::hellSand_Id)
		ITEM(Tile::lightGem_Id)
		ITEM_AUX(Tile::stoneBrickSmooth_Id,SmoothStoneBrickTile::TYPE_DEFAULT)
		ITEM_AUX(Tile::stoneBrickSmooth_Id,SmoothStoneBrickTile::TYPE_MOSSY)
		ITEM_AUX(Tile::stoneBrickSmooth_Id,SmoothStoneBrickTile::TYPE_CRACKED)
		ITEM_AUX(Tile::stoneBrickSmooth_Id,SmoothStoneBrickTile::TYPE_DETAIL)
		ITEM_AUX(Tile::monsterStoneEgg_Id,StoneMonsterTile::HOST_ROCK)
		ITEM_AUX(Tile::monsterStoneEgg_Id,StoneMonsterTile::HOST_COBBLE)
		ITEM_AUX(Tile::monsterStoneEgg_Id,StoneMonsterTile::HOST_STONEBRICK)
		ITEM(Tile::mycel_Id)
		ITEM(Tile::netherBrick_Id)
		ITEM(Tile::whiteStone_Id)
		ITEM_AUX(Tile::quartzBlock_Id,QuartzBlockTile::TYPE_CHISELED)
		ITEM_AUX(Tile::quartzBlock_Id,QuartzBlockTile::TYPE_LINES_Y)
		ITEM(Tile::trapdoor_Id)
		ITEM(Tile::fenceGate_Id)
		ITEM(Item::door_wood_Id)
		ITEM(Item::door_iron_Id)
		ITEM_AUX(Tile::stoneSlabHalf_Id,StoneSlabTile::STONE_SLAB)
		ITEM_AUX(Tile::stoneSlabHalf_Id,StoneSlabTile::SAND_SLAB)
		// AP - changed oak slab to be wood because it wouldn't burn
//		ITEM_AUX(Tile::stoneSlabHalf_Id,StoneSlabTile::WOOD_SLAB)
		ITEM_AUX(Tile::woodSlabHalf_Id,0)
		ITEM_AUX(Tile::woodSlabHalf_Id,TreeTile::DARK_TRUNK)
		ITEM_AUX(Tile::woodSlabHalf_Id,TreeTile::BIRCH_TRUNK)
		ITEM_AUX(Tile::woodSlabHalf_Id,TreeTile::JUNGLE_TRUNK)
		ITEM_AUX(Tile::stoneSlabHalf_Id,StoneSlabTile::COBBLESTONE_SLAB)
		ITEM_AUX(Tile::stoneSlabHalf_Id,StoneSlabTile::BRICK_SLAB)
		ITEM_AUX(Tile::stoneSlabHalf_Id,StoneSlabTile::SMOOTHBRICK_SLAB)
		ITEM_AUX(Tile::stoneSlabHalf_Id,StoneSlabTile::NETHERBRICK_SLAB)
		ITEM_AUX(Tile::stoneSlabHalf_Id,StoneSlabTile::QUARTZ_SLAB)
		ITEM(Tile::stairs_wood_Id)
		ITEM(Tile::stairs_birchwood_Id)
		ITEM(Tile::stairs_sprucewood_Id)
		ITEM(Tile::stairs_junglewood_Id)
		ITEM(Tile::stairs_stone_Id)
		ITEM(Tile::stairs_bricks_Id)
		ITEM(Tile::stairs_stoneBrickSmooth_Id)
		ITEM(Tile::stairs_netherBricks_Id)
		ITEM(Tile::stairs_sandstone_Id)
		ITEM(Tile::stairs_quartz_Id)


	// Decoration
	DEF(eCreativeInventory_Decoration)
		ITEM_AUX(Item::skull_Id,SkullTileEntity::TYPE_SKELETON)
		ITEM_AUX(Item::skull_Id,SkullTileEntity::TYPE_WITHER)
		ITEM_AUX(Item::skull_Id,SkullTileEntity::TYPE_ZOMBIE)
		ITEM_AUX(Item::skull_Id,SkullTileEntity::TYPE_CHAR)
		ITEM_AUX(Item::skull_Id,SkullTileEntity::TYPE_CREEPER)
		ITEM(Tile::sponge_Id)
		ITEM(Tile::melon_Id)
		ITEM(Tile::pumpkin_Id)
		ITEM(Tile::litPumpkin_Id)	
		ITEM_AUX(Tile::sapling_Id, Sapling::TYPE_DEFAULT)
		ITEM_AUX(Tile::sapling_Id, Sapling::TYPE_EVERGREEN)
		ITEM_AUX(Tile::sapling_Id, Sapling::TYPE_BIRCH)
		ITEM_AUX(Tile::sapling_Id, Sapling::TYPE_JUNGLE)
		ITEM_AUX(Tile::leaves_Id, LeafTile::NORMAL_LEAF)
		ITEM_AUX(Tile::leaves_Id, LeafTile::EVERGREEN_LEAF)
		ITEM_AUX(Tile::leaves_Id, LeafTile::BIRCH_LEAF)
		ITEM_AUX(Tile::leaves_Id, LeafTile::JUNGLE_LEAF)
		ITEM(Tile::vine)
		ITEM(Tile::waterLily_Id)
		ITEM(Tile::torch_Id)
		ITEM_AUX(Tile::tallgrass_Id, TallGrass::DEAD_SHRUB)
		ITEM_AUX(Tile::tallgrass_Id, TallGrass::TALL_GRASS)
		ITEM_AUX(Tile::tallgrass_Id, TallGrass::FERN)
		ITEM(Tile::deadBush_Id)
		ITEM(Tile::flower_Id)
		ITEM(Tile::rose_Id)
		ITEM(Tile::mushroom1_Id)
		ITEM(Tile::mushroom2_Id)
		ITEM(Tile::cactus_Id)
		ITEM(Tile::topSnow_Id)
		// 4J-PB - Already got sugar cane in Materials ITEM_11(Tile::reeds_Id)
		ITEM(Tile::web_Id)
		ITEM(Tile::thinGlass_Id)
		ITEM(Tile::glass_Id)
		ITEM(Item::painting_Id)
		ITEM(Item::itemFrame_Id)
		ITEM(Item::sign_Id)
		ITEM(Tile::bookshelf_Id)
		ITEM(Item::flowerPot_Id)
		ITEM_AUX(Tile::cloth_Id,14)	// Red
		ITEM_AUX(Tile::cloth_Id,1)	// Orange
		ITEM_AUX(Tile::cloth_Id,4)	// Yellow
		ITEM_AUX(Tile::cloth_Id,5)	// Lime
		ITEM_AUX(Tile::cloth_Id,3)	// Light Blue
		ITEM_AUX(Tile::cloth_Id,9)	// Cyan
		ITEM_AUX(Tile::cloth_Id,11)	// Blue
		ITEM_AUX(Tile::cloth_Id,10)	// Purple
		ITEM_AUX(Tile::cloth_Id,2)	// Magenta
		ITEM_AUX(Tile::cloth_Id,6)	// Pink
		ITEM_AUX(Tile::cloth_Id,0)	// White
		ITEM_AUX(Tile::cloth_Id,8)	// Light Gray
		ITEM_AUX(Tile::cloth_Id,7)	// Gray
		ITEM_AUX(Tile::cloth_Id,15)	// Black
		ITEM_AUX(Tile::cloth_Id,13)	// Green
		ITEM_AUX(Tile::cloth_Id,12)	// Brown

		ITEM_AUX(Tile::woolCarpet_Id,14)	// Red
		ITEM_AUX(Tile::woolCarpet_Id,1)	// Orange
		ITEM_AUX(Tile::woolCarpet_Id,4)	// Yellow
		ITEM_AUX(Tile::woolCarpet_Id,5)	// Lime
		ITEM_AUX(Tile::woolCarpet_Id,3)	// Light Blue
		ITEM_AUX(Tile::woolCarpet_Id,9)	// Cyan
		ITEM_AUX(Tile::woolCarpet_Id,11)	// Blue
		ITEM_AUX(Tile::woolCarpet_Id,10)	// Purple
		ITEM_AUX(Tile::woolCarpet_Id,2)	// Magenta
		ITEM_AUX(Tile::woolCarpet_Id,6)	// Pink
		ITEM_AUX(Tile::woolCarpet_Id,0)	// White
		ITEM_AUX(Tile::woolCarpet_Id,8)	// Light Gray
		ITEM_AUX(Tile::woolCarpet_Id,7)	// Gray
		ITEM_AUX(Tile::woolCarpet_Id,15)	// Black
		ITEM_AUX(Tile::woolCarpet_Id,13)	// Green
		ITEM_AUX(Tile::woolCarpet_Id,12)	// Brown


	// Redstone
	DEF(eCreativeInventory_Redstone)
		ITEM(Tile::dispenser_Id)
		ITEM(Tile::musicBlock_Id)
		ITEM(Tile::pistonBase_Id)
		ITEM(Tile::pistonStickyBase_Id)
		ITEM(Tile::tnt_Id)
		ITEM(Tile::lever_Id)
		ITEM(Tile::button_stone_Id)
		ITEM(Tile::button_wood_Id)
		ITEM(Tile::pressurePlate_stone_Id)
		ITEM(Tile::pressurePlate_wood_Id)
		ITEM(Item::redStone_Id)
		ITEM(Tile::notGate_on_Id)
		ITEM(Item::diode_Id)
		ITEM(Tile::redstoneLight_Id)
		ITEM(Tile::tripWireSource_Id)

	// Transport
	DEF(eCreativeInventory_Transport)
		ITEM(Tile::rail_Id)
		ITEM(Tile::goldenRail_Id)
		ITEM(Tile::detectorRail_Id)
		ITEM(Tile::ladder_Id)
		ITEM(Item::minecart_Id)
		ITEM(Item::minecart_chest_Id)
		ITEM(Item::minecart_furnace_Id)
		ITEM(Item::saddle_Id)
		ITEM(Item::boat_Id)

	// Miscellaneous
	DEF(eCreativeInventory_Misc)
		ITEM(Tile::chest_Id)
		ITEM(Tile::enderChest_Id)
		ITEM(Tile::workBench_Id)
		ITEM(Tile::furnace_Id)
		ITEM(Item::brewingStand_Id)
		ITEM(Tile::enchantTable_Id)
		ITEM(Tile::endPortalFrameTile_Id)
		ITEM(Tile::recordPlayer_Id)
		ITEM(Tile::anvil_Id);
		ITEM(Tile::fence_Id)
		ITEM(Tile::netherFence_Id)
		ITEM(Tile::ironFence_Id)
		ITEM_AUX(Tile::cobbleWall_Id, WallTile::TYPE_NORMAL)
		ITEM_AUX(Tile::cobbleWall_Id, WallTile::TYPE_MOSSY)
		ITEM(Item::bed_Id)
		ITEM(Item::bucket_empty_Id)
		ITEM(Item::bucket_lava_Id)	
		ITEM(Item::bucket_water_Id)
		ITEM(Item::milk_Id)
		ITEM(Item::cauldron_Id)
		ITEM(Item::snowBall_Id)
		ITEM(Item::paper_Id)
		ITEM(Item::book_Id)
		ITEM(Item::enderPearl_Id)
		ITEM(Item::eyeOfEnder_Id)
		ITEM(Item::record_01_Id)
		ITEM(Item::record_02_Id)
		ITEM(Item::record_03_Id)
		ITEM(Item::record_04_Id)
		ITEM(Item::record_05_Id)
		ITEM(Item::record_06_Id)
		ITEM(Item::record_07_Id)
		ITEM(Item::record_08_Id)
		ITEM(Item::record_09_Id)
		ITEM(Item::record_10_Id)
		ITEM(Item::record_11_Id)
		ITEM(Item::record_12_Id)
		ITEM_AUX(Item::monsterPlacer_Id, 50); // Creeper
		ITEM_AUX(Item::monsterPlacer_Id, 51); // Skeleton
		ITEM_AUX(Item::monsterPlacer_Id, 52); // Spider
		ITEM_AUX(Item::monsterPlacer_Id, 54); // Zombie
		ITEM_AUX(Item::monsterPlacer_Id, 55); // Slime
		ITEM_AUX(Item::monsterPlacer_Id, 56); // Ghast
		ITEM_AUX(Item::monsterPlacer_Id, 57); // Zombie Pigman
		ITEM_AUX(Item::monsterPlacer_Id, 58); // Enderman
		ITEM_AUX(Item::monsterPlacer_Id, 59); // Cave Spider
		ITEM_AUX(Item::monsterPlacer_Id, 60); // Silverfish
		ITEM_AUX(Item::monsterPlacer_Id, 61); // Blaze
		ITEM_AUX(Item::monsterPlacer_Id, 62); // Magma Cube
		ITEM_AUX(Item::monsterPlacer_Id, 90); // Pig
		ITEM_AUX(Item::monsterPlacer_Id, 91); // Sheep
		ITEM_AUX(Item::monsterPlacer_Id, 92); // Cow
		ITEM_AUX(Item::monsterPlacer_Id, 93); // Chicken
		ITEM_AUX(Item::monsterPlacer_Id, 94); // Squid
		ITEM_AUX(Item::monsterPlacer_Id, 95); // Wolf
		ITEM_AUX(Item::monsterPlacer_Id, 96); // Mooshroom
		ITEM_AUX(Item::monsterPlacer_Id, 98); // Ozelot
		ITEM_AUX(Item::monsterPlacer_Id, 120); // Villager

	// Food
	DEF(eCreativeInventory_Food)
		ITEM(Item::apple_Id)
		ITEM(Item::apple_gold_Id)
		ITEM_AUX(Item::apple_gold_Id,1) // Enchanted
		ITEM(Item::melon_Id)
		ITEM(Item::mushroomStew_Id)
		ITEM(Item::bread_Id)
		ITEM(Item::cake_Id)
		ITEM(Item::cookie_Id)
		ITEM(Item::fish_cooked_Id)
		ITEM(Item::fish_raw_Id)
		ITEM(Item::porkChop_cooked_Id)
		ITEM(Item::porkChop_raw_Id)
		ITEM(Item::beef_cooked_Id)
		ITEM(Item::beef_raw_Id)
		ITEM(Item::chicken_raw_Id)
		ITEM(Item::chicken_cooked_Id)		
		ITEM(Item::rotten_flesh_Id)
		ITEM(Item::spiderEye_Id)
		ITEM(Item::potato_Id)
		ITEM(Item::potatoBaked_Id)
		ITEM(Item::potatoPoisonous_Id)
		ITEM(Item::carrots_Id)
		ITEM(Item::carrotGolden_Id)
		ITEM(Item::pumpkinPie_Id)

	// Tools, Armour and Weapons (Complete)
	DEF(eCreativeInventory_ToolsArmourWeapons)
		ITEM(Item::compass_Id)
		ITEM(Item::helmet_cloth_Id)
		ITEM(Item::chestplate_cloth_Id)
		ITEM(Item::leggings_cloth_Id)
		ITEM(Item::boots_cloth_Id)
		ITEM(Item::sword_wood_Id)
		ITEM(Item::shovel_wood_Id)
		ITEM(Item::pickAxe_wood_Id)
		ITEM(Item::hatchet_wood_Id)
		ITEM(Item::hoe_wood_Id)
		
		ITEM(Item::map_Id)
		ITEM(Item::helmet_chain_Id)
		ITEM(Item::chestplate_chain_Id)
		ITEM(Item::leggings_chain_Id)
		ITEM(Item::boots_chain_Id)
		ITEM(Item::sword_stone_Id)
		ITEM(Item::shovel_stone_Id)
		ITEM(Item::pickAxe_stone_Id)
		ITEM(Item::hatchet_stone_Id)
		ITEM(Item::hoe_stone_Id)
		
		ITEM(Item::bow_Id)
		ITEM(Item::helmet_iron_Id)
		ITEM(Item::chestplate_iron_Id)
		ITEM(Item::leggings_iron_Id)
		ITEM(Item::boots_iron_Id)
		ITEM(Item::sword_iron_Id)
		ITEM(Item::shovel_iron_Id)
		ITEM(Item::pickAxe_iron_Id)
		ITEM(Item::hatchet_iron_Id)
		ITEM(Item::hoe_iron_Id)
		
		ITEM(Item::arrow_Id)
		ITEM(Item::helmet_gold_Id)
		ITEM(Item::chestplate_gold_Id)
		ITEM(Item::leggings_gold_Id)
		ITEM(Item::boots_gold_Id)
		ITEM(Item::sword_gold_Id)
		ITEM(Item::shovel_gold_Id)
		ITEM(Item::pickAxe_gold_Id)
		ITEM(Item::hatchet_gold_Id)
		ITEM(Item::hoe_gold_Id)

		ITEM(Item::flintAndSteel_Id)
		ITEM(Item::helmet_diamond_Id)
		ITEM(Item::chestplate_diamond_Id)
		ITEM(Item::leggings_diamond_Id)
		ITEM(Item::boots_diamond_Id)
		ITEM(Item::sword_diamond_Id)
		ITEM(Item::shovel_diamond_Id)
		ITEM(Item::pickAxe_diamond_Id)
		ITEM(Item::hatchet_diamond_Id)
		ITEM(Item::hoe_diamond_Id)

		ITEM(Item::fireball_Id)
		ITEM(Item::clock_Id)
		ITEM(Item::shears_Id)
		ITEM(Item::fishingRod_Id)
		ITEM(Item::carrotOnAStick_Id)

		for(unsigned int i = 0; i < Enchantment::enchantments.length; ++i)
		{
			Enchantment *enchantment = Enchantment::enchantments[i];
			if (enchantment == NULL || enchantment->category == NULL) continue;
			list->push_back(Item::enchantedBook->createForEnchantment(new EnchantmentInstance(enchantment, enchantment->getMaxLevel())));
		}

	// Materials
	DEF(eCreativeInventory_Materials)
		ITEM(Item::coal_Id)
		ITEM_AUX(Item::coal_Id,1)
		ITEM(Item::diamond_Id)
		ITEM(Item::emerald_Id)
		ITEM(Item::ironIngot_Id)
		ITEM(Item::goldIngot_Id)
		ITEM(Item::netherQuartz_Id)
		ITEM(Item::brick_Id)
		ITEM(Item::netherbrick_Id)
		ITEM(Item::stick_Id)
		ITEM(Item::bowl_Id)		
		ITEM(Item::bone_Id)
		ITEM(Item::string_Id)
		ITEM(Item::feather_Id)
		ITEM(Item::flint_Id)
		ITEM(Item::leather_Id)
		ITEM(Item::sulphur_Id)
		ITEM(Item::clay_Id)
		ITEM(Item::yellowDust_Id)
		ITEM(Item::seeds_wheat_Id)	
		ITEM(Item::seeds_melon_Id)
		ITEM(Item::seeds_pumpkin_Id)
		ITEM(Item::wheat_Id)
		ITEM(Item::reeds_Id)
		ITEM(Item::egg_Id)
		ITEM(Item::sugar_Id)		
		ITEM(Item::slimeBall_Id)
		ITEM(Item::blazeRod_Id)
		ITEM(Item::goldNugget_Id)
		ITEM(Item::netherStalkSeeds_Id)
		ITEM_AUX(Item::dye_powder_Id,1)		// Red
		ITEM_AUX(Item::dye_powder_Id,14)	// Orange
		ITEM_AUX(Item::dye_powder_Id,11)	// Yellow
		ITEM_AUX(Item::dye_powder_Id,10)	// Lime
		ITEM_AUX(Item::dye_powder_Id,12)	// Light Blue
		ITEM_AUX(Item::dye_powder_Id,6)		// Cyan
		ITEM_AUX(Item::dye_powder_Id,4)		// Blue
		ITEM_AUX(Item::dye_powder_Id,5)		// Purple
		ITEM_AUX(Item::dye_powder_Id,13)	// Magenta
		ITEM_AUX(Item::dye_powder_Id,9)		// Pink
		ITEM_AUX(Item::dye_powder_Id,15)	// Bone Meal
		ITEM_AUX(Item::dye_powder_Id,7)		// Light gray
		ITEM_AUX(Item::dye_powder_Id,8)		// Gray
		ITEM_AUX(Item::dye_powder_Id,0)		// black (ink sac)
		ITEM_AUX(Item::dye_powder_Id,2)		// Green
		ITEM_AUX(Item::dye_powder_Id,3)		// Brown

		// Brewing (TODO)
		DEF(eCreativeInventory_Brewing)
		ITEM(Item::expBottle_Id)

		// 4J Stu - Anything else added here also needs to be added to the key handler below
		ITEM(Item::ghastTear_Id)
		ITEM(Item::fermentedSpiderEye_Id)
		ITEM(Item::blazePowder_Id)
		ITEM(Item::magmaCream_Id)
		ITEM(Item::speckledMelon_Id)
		ITEM(Item::glassBottle_Id)
		ITEM_AUX(Item::potion_Id,0) // Water bottle		
		//ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, 0, MASK_TYPE_AWKWARD)) // Awkward Potion


		DEF(eCreativeInventory_Potions_Basic)
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, 0, MASK_REGENERATION))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, 0, MASK_SPEED))
			//ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, 0, MASK_FIRE_RESISTANCE))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, 0, MASK_POISON))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, 0, MASK_INSTANTHEALTH))
			//ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, 0, MASK_WEAKNESS))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, 0, MASK_STRENGTH))
			//ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, 0, MASK_SLOWNESS))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, 0, MASK_INSTANTDAMAGE))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, 0, MASK_REGENERATION))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, 0, MASK_SPEED))
			//ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, 0, MASK_FIRE_RESISTANCE))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, 0, MASK_POISON))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, 0, MASK_INSTANTHEALTH))
			//ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, 0, MASK_WEAKNESS))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, 0, MASK_STRENGTH))
			//ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, 0, MASK_SLOWNESS))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, 0, MASK_INSTANTDAMAGE))

		DEF(eCreativeInventory_Potions_Level2)
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, MASK_LEVEL2, MASK_REGENERATION))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, MASK_LEVEL2, MASK_SPEED))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, 0, MASK_FIRE_RESISTANCE))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, MASK_LEVEL2, MASK_POISON))
			//ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, MASK_LEVEL2, MASK_INSTANTHEALTH))
			//ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, MASK_LEVEL2, MASK_NIGHTVISION))
			//ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, MASK_LEVEL2, MASK_INVISIBILITY))
			
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, 0, MASK_WEAKNESS))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, MASK_LEVEL2, MASK_STRENGTH))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, 0, MASK_SLOWNESS))
			//ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, MASK_LEVEL2, MASK_INSTANTDAMAGE))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, MASK_LEVEL2, MASK_REGENERATION))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, MASK_LEVEL2, MASK_SPEED))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, 0, MASK_FIRE_RESISTANCE))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, MASK_LEVEL2, MASK_POISON))
			//ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, MASK_LEVEL2, MASK_INSTANTHEALTH))
			//ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, MASK_LEVEL2, MASK_NIGHTVISION))
			//ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, MASK_LEVEL2, MASK_INVISIBILITY))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, 0, MASK_WEAKNESS))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, MASK_LEVEL2, MASK_STRENGTH))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, 0, MASK_SLOWNESS))
			//ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, MASK_LEVEL2, MASK_INSTANTDAMAGE))

		DEF(eCreativeInventory_Potions_Extended)
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, MASK_EXTENDED, MASK_REGENERATION))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, MASK_EXTENDED, MASK_SPEED))
			//ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, MASK_EXTENDED, MASK_FIRE_RESISTANCE))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, MASK_EXTENDED, MASK_POISON))
			//ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, MASK_LEVEL2, MASK_INSTANTHEALTH))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, 0, MASK_NIGHTVISION))	// 4J- Moved here as there isn't a weak variant of this potion.
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, 0, MASK_INVISIBILITY))	// 4J- Moved here as there isn't a weak variant of this potion.
			//ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, MASK_EXTENDED, MASK_WEAKNESS))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, MASK_EXTENDED, MASK_STRENGTH))
			//ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, MASK_EXTENDED, MASK_SLOWNESS))
			//ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, MASK_LEVEL2, MASK_INSTANTDAMAGE))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, MASK_EXTENDED, MASK_REGENERATION))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, MASK_EXTENDED, MASK_SPEED))
			//ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, MASK_EXTENDED, MASK_FIRE_RESISTANCE))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, MASK_EXTENDED, MASK_POISON))
			//ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, MASK_LEVEL2, MASK_INSTANTHEALTH))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, 0, MASK_NIGHTVISION))		// 4J- Moved here as there isn't a weak variant of this potion.
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, 0, MASK_INVISIBILITY))	// 4J- Moved here as there isn't a weak variant of this potion.
			//ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, MASK_EXTENDED, MASK_WEAKNESS))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, MASK_EXTENDED, MASK_STRENGTH))
			//ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, MASK_EXTENDED, MASK_SLOWNESS))
			//ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, MASK_LEVEL2, MASK_INSTANTDAMAGE))

		DEF(eCreativeInventory_Potions_Level2_Extended)
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, MASK_LEVEL2EXTENDED, MASK_REGENERATION))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, MASK_LEVEL2EXTENDED, MASK_SPEED))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, MASK_EXTENDED, MASK_FIRE_RESISTANCE))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, MASK_LEVEL2EXTENDED, MASK_POISON))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, MASK_LEVEL2, MASK_INSTANTHEALTH))
			//ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, MASK_LEVEL2EXTENDED, MASK_NIGHTVISION))
			//ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, MASK_LEVEL2EXTENDED, MASK_INVISIBILITY))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, MASK_EXTENDED, MASK_NIGHTVISION))	// 4J- Moved here as there isn't a weak variant of this potion.
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, MASK_EXTENDED, MASK_INVISIBILITY))	// 4J- Moved here as there isn't a weak variant of this potion.
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, MASK_EXTENDED, MASK_WEAKNESS))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, MASK_LEVEL2EXTENDED, MASK_STRENGTH))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, MASK_EXTENDED, MASK_SLOWNESS))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(0, MASK_LEVEL2, MASK_INSTANTDAMAGE))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, MASK_LEVEL2EXTENDED, MASK_REGENERATION))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, MASK_LEVEL2EXTENDED, MASK_SPEED))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, MASK_EXTENDED, MASK_FIRE_RESISTANCE))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, MASK_LEVEL2EXTENDED, MASK_POISON))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, MASK_LEVEL2, MASK_INSTANTHEALTH))
			//ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, MASK_LEVEL2EXTENDED, MASK_NIGHTVISION))
			//ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, MASK_LEVEL2EXTENDED, MASK_INVISIBILITY))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, MASK_EXTENDED, MASK_NIGHTVISION))		// 4J- Moved here as there isn't a weak variant of this potion.
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, MASK_EXTENDED, MASK_INVISIBILITY))	// 4J- Moved here as there isn't a weak variant of this potion.
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, MASK_EXTENDED, MASK_WEAKNESS))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, MASK_LEVEL2EXTENDED, MASK_STRENGTH))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, MASK_EXTENDED, MASK_SLOWNESS))
			ITEM_AUX(Item::potion_Id,MACRO_MAKEPOTION_AUXVAL(MASK_SPLASH, MASK_LEVEL2, MASK_INSTANTDAMAGE))


		specs = new TabSpec*[eCreativeInventoryTab_COUNT];

	// Top Row
	ECreative_Inventory_Groups blocksGroup[] = {eCreativeInventory_BuildingBlocks};
	specs[eCreativeInventoryTab_BuildingBlocks] = new TabSpec(L"Structures", IDS_GROUPNAME_BUILDING_BLOCKS, 1, blocksGroup, 0, NULL);

	ECreative_Inventory_Groups decorationsGroup[] = {eCreativeInventory_Decoration};
	specs[eCreativeInventoryTab_Decorations] = new TabSpec(L"Decoration", IDS_GROUPNAME_DECORATIONS, 1, decorationsGroup, 0, NULL);

	ECreative_Inventory_Groups redAndTranGroup[] = {eCreativeInventory_Transport, eCreativeInventory_Redstone};
	specs[eCreativeInventoryTab_RedstoneAndTransport] = new TabSpec(L"RedstoneAndTransport", IDS_GROUPNAME_REDSTONE_AND_TRANSPORT, 2, redAndTranGroup, 0, NULL);

	ECreative_Inventory_Groups materialsGroup[] = {eCreativeInventory_Materials};
	specs[eCreativeInventoryTab_Materials] = new TabSpec(L"Materials", IDS_GROUPNAME_MATERIALS, 1, materialsGroup, 0, NULL);

	ECreative_Inventory_Groups foodGroup[] = {eCreativeInventory_Food};
	specs[eCreativeInventoryTab_Food] = new TabSpec(L"Food", IDS_GROUPNAME_FOOD, 1, foodGroup, 0, NULL);

	ECreative_Inventory_Groups toolsGroup[] = {eCreativeInventory_ToolsArmourWeapons};
	specs[eCreativeInventoryTab_ToolsWeaponsArmor] = new TabSpec(L"Tools", IDS_GROUPNAME_TOOLS_WEAPONS_ARMOR, 1, toolsGroup, 0, NULL);

	ECreative_Inventory_Groups brewingGroup[] = {eCreativeInventory_Brewing, eCreativeInventory_Potions_Level2_Extended, eCreativeInventory_Potions_Extended, eCreativeInventory_Potions_Level2, eCreativeInventory_Potions_Basic};

	// Just use the text LT - the graphic doesn't fit in splitscreen either
	// In 480p there's not enough room for the LT button, so use text instead
	//if(!RenderManager.IsHiDef() && !RenderManager.IsWidescreen())
	{
		specs[eCreativeInventoryTab_Brewing] = new TabSpec(L"Brewing", IDS_GROUPNAME_POTIONS_480, 5, brewingGroup, 0, NULL);
	}
	// 	else
	// 	{
	// 		specs[eCreativeInventoryTab_Brewing] = new TabSpec(L"icon_brewing.png", IDS_GROUPNAME_POTIONS, 1, brewingGroup, 4, potionsGroup);
	// 	}

	ECreative_Inventory_Groups miscGroup[] = {eCreativeInventory_Misc};
	specs[eCreativeInventoryTab_Misc] = new TabSpec(L"Misc", IDS_GROUPNAME_MISCELLANEOUS, 1, miscGroup, 0, NULL);

}

IUIScene_CreativeMenu::IUIScene_CreativeMenu()
{
	m_bCarryingCreativeItem = false;
	m_creativeSlotX = m_creativeSlotY = m_inventorySlotX = m_inventorySlotY = 0;

	// 4J JEV - Settup Tabs
	for (int i = 0; i < eCreativeInventoryTab_COUNT; i++) 
	{
		m_tabDynamicPos[i] = 0;
		m_tabPage[i] = 0;
	}
}

/* 4J JEV - Switches between tabs.
*/
void IUIScene_CreativeMenu::switchTab(ECreativeInventoryTabs tab)
{
	// Could just be changing page on the current tab
	if(tab != m_curTab) updateTabHighlightAndText(tab);

	m_curTab = tab;
	
	updateScrollCurrentPage(m_tabPage[m_curTab] + 1, specs[m_curTab]->getPageCount());

	specs[tab]->populateMenu(itemPickerMenu,m_tabDynamicPos[m_curTab], m_tabPage[m_curTab]);
}

// 4J JEV - Tab Spec Struct

IUIScene_CreativeMenu::TabSpec::TabSpec(LPCWSTR icon, int descriptionId, int staticGroupsCount, ECreative_Inventory_Groups *staticGroups, int dynamicGroupsCount, ECreative_Inventory_Groups *dynamicGroups)
	: m_icon(icon), m_descriptionId(descriptionId), m_staticGroupsCount(staticGroupsCount), m_dynamicGroupsCount(dynamicGroupsCount)
{
	
	m_pages = 0;
	m_staticGroupsA = NULL;

	unsigned int dynamicItems = 0;
	m_staticItems = 0;

	if(staticGroupsCount > 0)
	{
		m_staticGroupsA  = new ECreative_Inventory_Groups[staticGroupsCount];
		for(int i = 0; i < staticGroupsCount; ++i)
		{
			m_staticGroupsA[i] = staticGroups[i];
			m_staticItems += categoryGroups[m_staticGroupsA[i]].size();
		}
	}

	m_dynamicGroupsA = NULL;
	if(dynamicGroupsCount > 0)
	{
		m_dynamicGroupsA  = new ECreative_Inventory_Groups[dynamicGroupsCount];
		for(int i = 0; i < dynamicGroupsCount; ++i)
		{
			m_dynamicGroupsA[i] = dynamicGroups[i];
			dynamicItems += categoryGroups[m_dynamicGroupsA[i]].size();
		}
	}

	m_staticPerPage = MAX_SIZE - dynamicItems;
	m_pages = (int)ceil((float)m_staticItems / m_staticPerPage);
}

IUIScene_CreativeMenu::TabSpec::~TabSpec()
{
	if(m_staticGroupsA != NULL) delete [] m_staticGroupsA;
	if(m_dynamicGroupsA != NULL) delete [] m_dynamicGroupsA;
}

void IUIScene_CreativeMenu::TabSpec::populateMenu(AbstractContainerMenu *menu, int dynamicIndex, unsigned int page)
{
	int lastSlotIndex = 0;

	// Fill the dynamic group
	if(m_dynamicGroupsCount > 0 && m_dynamicGroupsA != NULL)
	{
		for(AUTO_VAR(it, categoryGroups[m_dynamicGroupsA[dynamicIndex]].rbegin()); it != categoryGroups[m_dynamicGroupsA[dynamicIndex]].rend() && lastSlotIndex < MAX_SIZE; ++it)
		{
			Slot *slot = menu->getSlot(++lastSlotIndex);
			slot->set( *it );
		}
	}

	// Fill from the static groups
	unsigned int startIndex = page * m_staticPerPage;
	int remainingItems = m_staticItems - startIndex;

	// Work out the first group with an item the want to display, and which item in that group
	unsigned int currentIndex = 0;
	unsigned int currentGroup = 0;
	unsigned int currentItem = 0;
	for(; currentGroup < m_staticGroupsCount; ++currentGroup)
	{
		int size = categoryGroups[m_staticGroupsA[currentGroup]].size();
		if( currentIndex + size < startIndex)
		{
			currentIndex += size;
			continue;
		}
		currentItem = size - ((currentIndex + size) - startIndex);
		break;
	}

	for(; lastSlotIndex < MAX_SIZE;)
	{
		Slot *slot = menu->getSlot(lastSlotIndex++);
		slot->set(categoryGroups[m_staticGroupsA[currentGroup]][currentItem]);

		++currentItem;
		if(currentItem >= categoryGroups[m_staticGroupsA[currentGroup]].size())
		{
			currentItem = 0;
			++currentGroup;
			if(currentGroup >= m_staticGroupsCount)
			{
				break;
			}
		}
	}

	for(; lastSlotIndex < MAX_SIZE; ++lastSlotIndex)
	{
		Slot *slot = menu->getSlot(lastSlotIndex);
		slot->remove(1);
	}
}

unsigned int IUIScene_CreativeMenu::TabSpec::getPageCount()
{
	return m_pages;
}


// 4J JEV - Item Picker Menu
IUIScene_CreativeMenu::ItemPickerMenu::ItemPickerMenu(	shared_ptr<SimpleContainer> smp, shared_ptr<Inventory> inv ) : AbstractContainerMenu()
{
	inventory = inv;
	creativeContainer = smp; 

	//int startLength = slots->size();

	Slot *slot = NULL;
	for (int i = 0; i < TabSpec::MAX_SIZE; i++)
	{
		// 4J JEV -  These values get set by addSlot anyway.
		slot = new Slot( creativeContainer, i, -1, -1);

		ItemPickerMenu::addSlot( slot );
	}

	for (int i = 0; i < 9; i++)
	{
		slot = new Slot( inventory, i, -1, -1 );
		ItemPickerMenu::addSlot( slot );
	}

	// 4J Stu - Give the creative menu a unique container id
	containerId = CONTAINER_ID_CREATIVE;
}

bool IUIScene_CreativeMenu::ItemPickerMenu::stillValid(shared_ptr<Player> player)
{
	return true;
}

bool IUIScene_CreativeMenu::ItemPickerMenu::isOverrideResultClick(int slotNum, int buttonNum)
{
	return slotNum >= 0 && slotNum < 9 && buttonNum == 0;
}

IUIScene_AbstractContainerMenu::ESceneSection IUIScene_CreativeMenu::GetSectionAndSlotInDirection( ESceneSection eSection, ETapState eTapDirection, int *piTargetX, int *piTargetY )
{
	ESceneSection newSection = eSection;

	// Find the new section if there is one
	switch( eSection )
	{
	case eSectionInventoryCreativeSelector:
		if (eTapDirection == eTapStateDown || eTapDirection == eTapStateUp) 
		{
			newSection = eSectionInventoryCreativeUsing;
		}
		break;
	case eSectionInventoryCreativeUsing:
		if (eTapDirection == eTapStateDown || eTapDirection == eTapStateUp)
		{
			newSection = eSectionInventoryCreativeSelector;
		}
		break;
#ifndef _XBOX
	case eSectionInventoryCreativeTab_0:
	case eSectionInventoryCreativeTab_1:
	case eSectionInventoryCreativeTab_2:
	case eSectionInventoryCreativeTab_3:
	case eSectionInventoryCreativeTab_4:
	case eSectionInventoryCreativeTab_5:
	case eSectionInventoryCreativeTab_6:
	case eSectionInventoryCreativeTab_7:
	case eSectionInventoryCreativeSlider:
		/* do nothing */
		break;
#endif
	default:
		assert( false );
		break;
	}

	updateSlotPosition(eSection, newSection, eTapDirection, piTargetX, piTargetY, 0);

	return newSection;
}

bool IUIScene_CreativeMenu::handleValidKeyPress(int iPad, int buttonNum, BOOL quickKeyHeld)
{
	// 4J Added - Make pressing the X button clear the hotbar
	if(buttonNum == 1)
	{
		Minecraft *pMinecraft = Minecraft::GetInstance();
		for(unsigned int i = TabSpec::MAX_SIZE; i < TabSpec::MAX_SIZE + 9; ++i)
		{
			shared_ptr<ItemInstance> newItem = m_menu->getSlot(i)->getItem();

			if(newItem != NULL)
			{
				m_menu->getSlot(i)->set(nullptr);
				// call this function to synchronize multiplayer item bar
				pMinecraft->localgameModes[iPad]->handleCreativeModeItemAdd(nullptr, i - (int)m_menu->slots->size() + 9 + InventoryMenu::USE_ROW_SLOT_START);
			}
		}
		return true;
	}
	return false;
}

void IUIScene_CreativeMenu::handleOutsideClicked(int iPad, int buttonNum, BOOL quickKeyHeld)
{
	// Drop items.
	Minecraft *pMinecraft = Minecraft::GetInstance();

	shared_ptr<Inventory> playerInventory = pMinecraft->localplayers[iPad]->inventory;
	if (playerInventory->getCarried() != NULL)
	{
		if (buttonNum == 0)
		{
			pMinecraft->localgameModes[iPad]->handleCreativeModeItemDrop(playerInventory->getCarried());
			playerInventory->setCarried(nullptr);
		}
		if (buttonNum == 1)
		{
			shared_ptr<ItemInstance> removedItem = playerInventory->getCarried()->remove(1);
			pMinecraft->localgameModes[iPad]->handleCreativeModeItemDrop(removedItem);
			if (playerInventory->getCarried()->count == 0) playerInventory->setCarried(nullptr);
		}
	}

	//pMinecraft->localgameModes[m_iPad]->handleInventoryMouseClick(menu->containerId, AbstractContainerMenu::CLICKED_OUTSIDE, buttonNum, quickKeyHeld?true:false, pMinecraft->localplayers[m_iPad] );
}

void IUIScene_CreativeMenu::handleAdditionalKeyPress(int iAction)
{
	int dir = 1;
	switch(iAction)
	{
	case ACTION_MENU_LEFT_SCROLL:
		dir = -1;
		// Fall through intentional
	case ACTION_MENU_RIGHT_SCROLL:
		{		
			ECreativeInventoryTabs tab = (ECreativeInventoryTabs)(m_curTab + dir);
			if (tab < 0) tab = (ECreativeInventoryTabs)(eCreativeInventoryTab_COUNT - 1);
			if (tab >= eCreativeInventoryTab_COUNT) tab = eCreativeInventoryTab_BuildingBlocks;
			switchTab(tab);
			ui.PlayUISFX(eSFX_Focus);
		}
		break;
	case ACTION_MENU_PAGEUP:
		// change the potion strength
		{
			++m_tabDynamicPos[m_curTab];
			if(m_tabDynamicPos[m_curTab] >= specs[m_curTab]->m_dynamicGroupsCount) m_tabDynamicPos[m_curTab] = 0;
			switchTab(m_curTab);
		}
		break;
	case ACTION_MENU_OTHER_STICK_DOWN:
			++m_tabPage[m_curTab];
			if(m_tabPage[m_curTab] >= specs[m_curTab]->getPageCount())
			{
				m_tabPage[m_curTab] = specs[m_curTab]->getPageCount() - 1;
			}
			else
			{
				switchTab(m_curTab);
			}
		break;
	case ACTION_MENU_OTHER_STICK_UP:
			--m_tabPage[m_curTab];
			if(m_tabPage[m_curTab] < 0)
			{
				m_tabPage[m_curTab] = 0;
			}
			else
			{
				switchTab(m_curTab);
			}
		break;
	}
}

void IUIScene_CreativeMenu::handleSlotListClicked(ESceneSection eSection, int buttonNum, BOOL quickKeyHeld)
{
	int currentIndex = getCurrentIndex(eSection);

	Minecraft *pMinecraft = Minecraft::GetInstance();

	bool instantPlace = false;
	if (eSection == eSectionInventoryCreativeSelector)
	{
		if (buttonNum == 0)
		{

			shared_ptr<Inventory> playerInventory = pMinecraft->localplayers[getPad()]->inventory;
			shared_ptr<ItemInstance> carried = playerInventory->getCarried();
			shared_ptr<ItemInstance> clicked = m_menu->getSlot(currentIndex)->getItem();
			if (clicked != NULL)
			{
				playerInventory->setCarried(ItemInstance::clone(clicked));
				carried = playerInventory->getCarried();
				if (quickKeyHeld == TRUE)
				{
					carried->count = carried->getMaxStackSize();
				}
				m_creativeSlotX = m_iCurrSlotX;
				m_creativeSlotY = m_iCurrSlotY;
				m_eCurrSection = eSectionInventoryCreativeUsing;
				m_eCurrTapState = eTapStateJump;

				instantPlace = getEmptyInventorySlot(carried, m_inventorySlotX);
				m_iCurrSlotX = m_inventorySlotX;
				m_iCurrSlotY = m_inventorySlotY;

				m_bCarryingCreativeItem = true;
			}
		}
	}
	if(instantPlace || eSection == eSectionInventoryCreativeUsing)
	{
		if(instantPlace)
		{
			setSectionSelectedSlot(eSectionInventoryCreativeUsing,m_iCurrSlotX,m_iCurrSlotY);
			currentIndex = getCurrentIndex(eSectionInventoryCreativeUsing);
			buttonNum = 0;
			quickKeyHeld = FALSE;
		}
		m_menu->clicked(currentIndex, buttonNum, quickKeyHeld?AbstractContainerMenu::CLICK_QUICK_MOVE:AbstractContainerMenu::CLICK_PICKUP, pMinecraft->localplayers[getPad()]);
		shared_ptr<ItemInstance> newItem = m_menu->getSlot(currentIndex)->getItem();
		// call this function to synchronize multiplayer item bar
		pMinecraft->localgameModes[getPad()]->handleCreativeModeItemAdd(newItem, currentIndex - (int)m_menu->slots->size() + 9 + InventoryMenu::USE_ROW_SLOT_START);

		if(m_bCarryingCreativeItem)
		{
			m_inventorySlotX = m_iCurrSlotX;
			m_inventorySlotY = m_iCurrSlotY;
			m_eCurrSection = eSectionInventoryCreativeSelector;
			m_eCurrTapState = eTapStateJump;
			m_iCurrSlotX = m_creativeSlotX;
			m_iCurrSlotY = m_creativeSlotY;

			shared_ptr<Inventory> playerInventory = pMinecraft->localplayers[getPad()]->inventory;
			playerInventory->setCarried(nullptr);
			m_bCarryingCreativeItem = false;
		}
	}
}

bool IUIScene_CreativeMenu::IsSectionSlotList( ESceneSection eSection )
{
	switch( eSection )
	{
		case eSectionInventoryCreativeUsing:
		case eSectionInventoryCreativeSelector:
			return true;
	}
	return false;
}

bool IUIScene_CreativeMenu::CanHaveFocus( ESceneSection eSection )
{
	switch( eSection )
	{
		case eSectionInventoryCreativeUsing:
		case eSectionInventoryCreativeSelector:
			return true;
	}
	return false;
}

bool IUIScene_CreativeMenu::getEmptyInventorySlot(shared_ptr<ItemInstance> item, int &slotX)
{
	bool sameItemFound = false;
	bool emptySlotFound = false;
	// Jump to the slot with this item already on it, if we can stack more
	for(unsigned int i = TabSpec::MAX_SIZE; i < TabSpec::MAX_SIZE + 9; ++i)
	{
		shared_ptr<ItemInstance> slotItem = m_menu->getSlot(i)->getItem();
		if( slotItem != NULL && slotItem->sameItem(item) && (slotItem->GetCount() + item->GetCount() <= item->getMaxStackSize() ))
		{
			sameItemFound = true;
			slotX = i - TabSpec::MAX_SIZE;
			break;
		}
	}

	if(!sameItemFound)
	{
		// Find an empty slot
		for(unsigned int i = TabSpec::MAX_SIZE; i < TabSpec::MAX_SIZE + 9; ++i)
		{
			if( m_menu->getSlot(i)->getItem() == NULL )
			{
				slotX = i - TabSpec::MAX_SIZE;
				emptySlotFound = true;
				break;
			}
		}
	}
	return sameItemFound || emptySlotFound;
}

int IUIScene_CreativeMenu::getSectionStartOffset(ESceneSection eSection)
{
	int offset = 0;
	switch( eSection )
	{
	case eSectionInventoryCreativeSelector:
		offset = 0;
		break;
	case eSectionInventoryCreativeUsing:
		offset = TabSpec::MAX_SIZE;
		break;
	default:
		assert( false );
		break;
	}
	return offset;
}

bool IUIScene_CreativeMenu::overrideTooltips(ESceneSection sectionUnderPointer, shared_ptr<ItemInstance> itemUnderPointer, bool bIsItemCarried, bool bSlotHasItem, bool bCarriedIsSameAsSlot, int iSlotStackSizeRemaining,
	EToolTipItem &buttonA, EToolTipItem &buttonX, EToolTipItem &buttonY, EToolTipItem &buttonRT)
{
	bool _override = false;

	if(sectionUnderPointer == eSectionInventoryCreativeSelector)
	{
		if(bSlotHasItem)
		{
			buttonA = eToolTipPickUpGeneric;
			buttonRT = eToolTipWhatIsThis;

			if(itemUnderPointer->isStackable())
			{
				buttonY = eToolTipPickUpAll;
			}
			else
			{
				buttonY = eToolTipNone; //eToolTipPickUpGeneric;
			}
		}
	}
	else if(sectionUnderPointer == eSectionInventoryCreativeUsing)
	{
		buttonY = eToolTipNone;
	}
	buttonX = eToolTipClearQuickSelect;
	_override = true;

	return _override;
}
