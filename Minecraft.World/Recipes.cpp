#include "stdafx.h"
#include "Container.h"
#include "AbstractContainerMenu.h"
#include "CraftingContainer.h"
#include "CoalItem.h"
#include "Item.h"
#include "ItemInstance.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.item.crafting.h"

Recipes *Recipes::instance = NULL;
ArmorRecipes *Recipes::pArmorRecipes=NULL;
ClothDyeRecipes *Recipes::pClothDyeRecipes=NULL;
FoodRecipies *Recipes::pFoodRecipies=NULL;
OreRecipies *Recipes::pOreRecipies=NULL;
StructureRecipies *Recipes::pStructureRecipies=NULL;
ToolRecipies *Recipes::pToolRecipies=NULL;
WeaponRecipies *Recipes::pWeaponRecipies=NULL;
FireworksRecipe *Recipes::pFireworksRecipes=NULL;

void Recipes::staticCtor()
{
	Recipes::instance = new Recipes();

}

void Recipes::_init()
{
	// 4J Jev: instance = new Recipes();
	recipies = new RecipyList();
}

Recipes::Recipes()
{
	int iCount=0;
	_init();

	pArmorRecipes		= new ArmorRecipes;
	pClothDyeRecipes	= new ClothDyeRecipes;
	pFoodRecipies		= new FoodRecipies;
	pOreRecipies		= new OreRecipies;
	pStructureRecipies	= new StructureRecipies;
	pToolRecipies		= new ToolRecipies;
	pWeaponRecipies		= new WeaponRecipies;

	// 4J Stu - These just don't work with our crafting menu
	//recipies->push_back(new ArmorDyeRecipe());
	//recipies->add(new MapCloningRecipe());
	//recipies->add(new MapExtendingRecipe());
	//recipies->add(new FireworksRecipe());
	pFireworksRecipes = new FireworksRecipe();


	addShapedRecipy(new ItemInstance(Tile::wood, 4, 0), //
		L"sczg",
		L"#", //

		L'#', new ItemInstance(Tile::treeTrunk, 1, 0),
		L'S');

	// TU9 - adding coloured wood
	addShapedRecipy(new ItemInstance(Tile::wood, 4, TreeTile::BIRCH_TRUNK), //
		L"sczg",
		L"#", //

		L'#', new ItemInstance(Tile::treeTrunk, 1, TreeTile::BIRCH_TRUNK),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::wood, 4, TreeTile::DARK_TRUNK), //
		L"sczg",
		L"#", //

		L'#', new ItemInstance(Tile::treeTrunk, 1, TreeTile::DARK_TRUNK),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::wood, 4, TreeTile::JUNGLE_TRUNK), //
		L"sczg",
		L"#", //

		L'#', new ItemInstance(Tile::treeTrunk, 1, TreeTile::JUNGLE_TRUNK),
		L'S');

	addShapedRecipy(new ItemInstance(Item::stick, 4), //
		L"ssctg",
		L"#", //
		L"#", //

		L'#', Tile::wood,
		L'S');

	pToolRecipies->addRecipes(this);
	pFoodRecipies->addRecipes(this);
	pStructureRecipies->addRecipes(this);


	// 4J-PB - changing the order to the way we want to have things in the crafting menu
	// bed
	addShapedRecipy(new ItemInstance(Item::bed, 1), //
		L"ssctctg",
		L"###", //
		L"XXX", //
		L'#', Tile::wool, L'X', Tile::wood,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::enchantTable, 1), //
		L"sssctcicig",
		L" B ", //
		L"D#D", //
		L"###", //

		L'#', Tile::obsidian, L'B', Item::book, L'D', Item::diamond,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::anvil, 1), //
		L"sssctcig",
		L"III", //
		L" i ", //
		L"iii", //

		L'I', Tile::ironBlock, L'i', Item::ironIngot,
		L'S');

	// 4J Stu - Reordered for crafting menu
	addShapedRecipy(new ItemInstance(Tile::ladder, 3), //
		L"ssscig",
		L"# #", //
		L"###", //
		L"# #", //

		L'#', Item::stick,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::fenceGate, 1), //
		L"sscictg",
		L"#W#", //
		L"#W#", //

		L'#', Item::stick, L'W', Tile::wood,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::fence, 2), //
		L"sscig",
		L"###", //
		L"###", //

		L'#', Item::stick,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::netherFence, 6), //
		L"ssctg",
		L"###", //
		L"###", //

		L'#', Tile::netherBrick,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::ironFence, 16), //
		L"sscig",
		L"###", //
		L"###", //

		L'#', Item::ironIngot,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::cobbleWall, 6, WallTile::TYPE_NORMAL), //
		L"ssctg",
		L"###", //
		L"###", //

		L'#', Tile::cobblestone,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::cobbleWall, 6, WallTile::TYPE_MOSSY), //
		L"ssctg",
		L"###", //
		L"###", //

		L'#', Tile::mossyCobblestone,
		L'S');

	addShapedRecipy(new ItemInstance(Item::door_wood, 1), //
		L"sssctg",
		L"##", //
		L"##", //
		L"##", //

		L'#', Tile::wood,
		L'S');

	addShapedRecipy(new ItemInstance(Item::door_iron, 1), //
		L"ssscig",
		L"##", //
		L"##", //
		L"##", //

		L'#', Item::ironIngot,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::stairs_wood, 4), //
		L"sssczg",
		L"#  ", //
		L"## ", //
		L"###", //

		L'#', new ItemInstance(Tile::wood, 1, 0),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::trapdoor, 2), //
		L"ssctg",
		L"###", //
		L"###", //

		L'#', Tile::wood,
		L'S');
	addShapedRecipy(new ItemInstance(Tile::stairs_stone, 4), //
		L"sssctg",
		L"#  ", //
		L"## ", //
		L"###", //

		L'#', Tile::cobblestone,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::stairs_bricks, 4), //
		L"sssctg",
		L"#  ", //
		L"## ", //
		L"###", //

		L'#', Tile::redBrick,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::stairs_stoneBrickSmooth, 4), //
		L"sssctg",
		L"#  ", //
		L"## ", //
		L"###", //

		L'#', Tile::stoneBrick,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::stairs_netherBricks, 4), //
		L"sssctg",
		L"#  ", //
		L"## ", //
		L"###", //

		L'#', Tile::netherBrick,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::stairs_sandstone, 4), //
		L"sssctg",
		L"#  ", //
		L"## ", //
		L"###", //

		L'#', Tile::sandStone,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::woodStairsBirch, 4), //
		L"sssczg",
		L"#  ", //
		L"## ", //
		L"###", //

		L'#', new ItemInstance(Tile::wood, 1, TreeTile::BIRCH_TRUNK),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::woodStairsDark, 4), //
		L"sssczg",
		L"#  ", //
		L"## ", //
		L"###", //

		L'#', new ItemInstance(Tile::wood, 1, TreeTile::DARK_TRUNK),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::woodStairsJungle, 4), //
		L"sssczg",
		L"#  ", //
		L"## ", //
		L"###", //

		L'#', new ItemInstance(Tile::wood, 1, TreeTile::JUNGLE_TRUNK),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::stairs_quartz, 4), //
		L"sssctg",
		L"#  ", //
		L"## ", //
		L"###", //

		L'#', Tile::quartzBlock,
		L'S');

	pArmorRecipes->addRecipes(this);
	//iCount=getRecipies()->size();

	pClothDyeRecipes->addRecipes(this);


	addShapedRecipy(new ItemInstance(Tile::snow, 1), //
		L"sscig",
		L"##", //
		L"##", //

		L'#', Item::snowBall,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::topSnow, 6), //
		L"sctg",
		L"###", //

		L'#', Tile::snow,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::clay, 1), //
		L"sscig",
		L"##", //
		L"##", //

		L'#', Item::clay,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::redBrick, 1), //
		L"sscig",
		L"##", //
		L"##", //

		L'#', Item::brick,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::wool, 1), //
		L"sscig",
		L"##", //
		L"##", //

		L'#', Item::string,
		L'D');

	addShapedRecipy(new ItemInstance(Tile::tnt, 1), //
		L"ssscictg",
		L"X#X", //
		L"#X#", //
		L"X#X", //

		L'X', Item::gunpowder,//
		L'#', Tile::sand,
		L'T');

	addShapedRecipy(new ItemInstance(Tile::stoneSlabHalf, 6, StoneSlabTile::SAND_SLAB), //
		L"sctg",
		L"###", //

		L'#', Tile::sandStone,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::stoneSlabHalf, 6, StoneSlabTile::STONE_SLAB), //
		L"sctg",
		L"###", //

		L'#', Tile::stone,
		L'S');
	addShapedRecipy(new ItemInstance(Tile::stoneSlabHalf, 6, StoneSlabTile::COBBLESTONE_SLAB), //
		L"sctg",
		L"###", //

		L'#', Tile::cobblestone,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::stoneSlabHalf, 6, StoneSlabTile::BRICK_SLAB), //
		L"sctg",
		L"###", //

		L'#', Tile::redBrick,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::stoneSlabHalf, 6, StoneSlabTile::SMOOTHBRICK_SLAB), //
		L"sctg",
		L"###", //

		L'#', Tile::stoneBrick,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::stoneSlabHalf, 6, StoneSlabTile::NETHERBRICK_SLAB), //
		L"sctg",
		L"###", //

		L'#', Tile::netherBrick,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::stoneSlabHalf, 6, StoneSlabTile::QUARTZ_SLAB), //
		L"sctg",
		L"###", //

		L'#', Tile::quartzBlock,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::woodSlabHalf, 6, 0), //
		L"sczg",
		L"###", //

		L'#', new ItemInstance(Tile::wood, 1, 0),
		L'S');
	// TU9 - adding wood slabs

	addShapedRecipy(new ItemInstance(Tile::woodSlabHalf, 6, TreeTile::BIRCH_TRUNK), //
		L"sczg",
		L"###", //

		L'#', new ItemInstance(Tile::wood, 1, TreeTile::BIRCH_TRUNK),
		L'S');


	addShapedRecipy(new ItemInstance(Tile::woodSlabHalf, 6, TreeTile::DARK_TRUNK), //
		L"sczg",
		L"###", //

		L'#', new ItemInstance(Tile::wood, 1, TreeTile::DARK_TRUNK),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::woodSlabHalf, 6, TreeTile::JUNGLE_TRUNK), //
		L"sczg",
		L"###", //

		L'#', new ItemInstance(Tile::wood, 1, TreeTile::JUNGLE_TRUNK),
		L'S');





	//iCount=getRecipies()->size();

	addShapedRecipy(new ItemInstance(Item::cake, 1), //
		L"ssscicicicig",
		L"AAA", //
		L"BEB", //
		L"CCC", //

		L'A', Item::bucket_milk,//
		L'B', Item::sugar,//
		L'C', Item::wheat, L'E', Item::egg,
		L'F');

	addShapedRecipy(new ItemInstance(Item::sugar, 1), //
		L"scig",
		L"#", //

		L'#', Item::reeds,
		L'F');

	addShapedRecipy(new ItemInstance(Tile::rail, 16), //
		L"ssscicig",
		L"X X", //
		L"X#X", //
		L"X X", //

		L'X', Item::ironIngot,//
		L'#', Item::stick,
		L'V');

	addShapedRecipy(new ItemInstance(Tile::goldenRail, 6), //
		L"ssscicicig",
		L"X X", //
		L"X#X", //
		L"XRX", //

		L'X', Item::goldIngot,//
		L'R', Item::redStone,//
		L'#', Item::stick,
		L'V');

	addShapedRecipy(new ItemInstance(Tile::activatorRail, 6), //
		L"ssscictcig",
		L"XSX", //
		L"X#X", //
		L"XSX", //

		L'X', Item::ironIngot,//
		L'#', Tile::redstoneTorch_on,//
		L'S', Item::stick,
		L'V');

	addShapedRecipy(new ItemInstance(Tile::detectorRail, 6), //
		L"ssscicictg",
		L"X X", //
		L"X#X", //
		L"XRX", //

		L'X', Item::ironIngot,//
		L'R', Item::redStone,//
		L'#', Tile::pressurePlate_stone,
		L'V');

	addShapedRecipy(new ItemInstance(Item::minecart, 1), //
		L"sscig",
		L"# #", //
		L"###", //

		L'#', Item::ironIngot,
		L'V');

	addShapedRecipy(new ItemInstance(Item::minecart_chest, 1), //
		L"ssctcig",
		L"A", //
		L"B", //

		L'A', Tile::chest, L'B', Item::minecart,
		L'V');

	addShapedRecipy(new ItemInstance(Item::minecart_furnace, 1), //
		L"ssctcig",
		L"A", //
		L"B", //

		L'A', Tile::furnace, L'B', Item::minecart,
		L'V');

	addShapedRecipy(new ItemInstance(Item::minecart_tnt, 1), //
		L"ssctcig",
		L"A", //
		L"B", //

		L'A', Tile::tnt, L'B', Item::minecart,
		L'V');

	addShapedRecipy(new ItemInstance(Item::minecart_hopper, 1), //
		L"ssctcig",
		L"A", //
		L"B", //

		L'A', Tile::hopper, L'B', Item::minecart,
		L'V');

	addShapedRecipy(new ItemInstance(Item::boat, 1), //
		L"ssctg",
		L"# #", //
		L"###", //

		L'#', Tile::wood,
		L'V');

	addShapedRecipy(new ItemInstance((Item *)Item::fishingRod, 1), //
		L"ssscicig",
		L"  #", //
		L" #X", //
		L"# X", //

		L'#', Item::stick, L'X', Item::string,
		L'T');

	addShapedRecipy(new ItemInstance(Item::carrotOnAStick, 1), //
		L"sscicig",
		L"# ", //
		L" X", //

		L'#', Item::fishingRod, L'X', Item::carrots,
		L'T')->keepTag();

	addShapedRecipy(new ItemInstance(Item::flintAndSteel, 1), //
		L"sscicig",
		L"A ", //
		L" B", //

		L'A', Item::ironIngot, L'B', Item::flint,
		L'T');

	addShapedRecipy(new ItemInstance(Item::bread, 1), //
		L"scig",
		L"###", //

		L'#', Item::wheat,
		L'F');

	// Moved bow and arrow in from weapons to avoid stacking on the group name display
	addShapedRecipy(new ItemInstance((Item *)Item::bow, 1), //
		L"ssscicig",
		L" #X", //
		L"# X", //
		L" #X", //

		L'X', Item::string,//
		L'#', Item::stick,
		L'T');

	addShapedRecipy(new ItemInstance(Item::arrow, 4), //
		L"ssscicicig",
		L"X", //
		L"#", //
		L"Y", //

		L'Y', Item::feather,//
		L'X', Item::flint,//
		L'#', Item::stick,
		L'T');

	pWeaponRecipies->addRecipes(this);

	addShapedRecipy(new ItemInstance(Item::bucket_empty, 1), //
		L"sscig",
		L"# #", //
		L" # ", //

		L'#', Item::ironIngot,
		L'T');

	addShapedRecipy(new ItemInstance(Item::bowl, 4), //
		L"ssctg",
		L"# #", //
		L" # ", //

		L'#', Tile::wood,
		L'T');

	addShapedRecipy(new ItemInstance(Item::glassBottle, 3), //
		L"ssctg",
		L"# #", //
		L" # ", //

		L'#', Tile::glass,
		L'T');

	addShapedRecipy(new ItemInstance(Item::flowerPot, 1), //
		L"sscig",
		L"# #", //
		L" # ", //

		L'#', Item::brick,
		L'D');

	// torch made of charcoal - moved to be the default due to the tutorial using it
	addShapedRecipy(new ItemInstance(Tile::torch, 4), //
		L"ssczcig",
		L"X", //
		L"#", //

		L'X', new ItemInstance(Item::coal, 1, CoalItem::CHAR_COAL),//
		L'#', Item::stick,
		L'T');

	addShapedRecipy(new ItemInstance(Tile::torch, 4), //
		L"ssczcig",
		L"X", //
		L"#", //
		L'X', new ItemInstance(Item::coal, 1, CoalItem::STONE_COAL),//
		L'#', Item::stick,
		L'T');

	addShapedRecipy(new ItemInstance(Tile::glowstone, 1), //
		L"sscig",
		L"##", //
		L"##", //

		L'#', Item::yellowDust,
		L'T');

	addShapedRecipy(new ItemInstance(Tile::quartzBlock, 1), //
		L"sscig",
		L"##", //
		L"##", //

		L'#', Item::netherQuartz,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::lever, 1), //
		L"ssctcig",
		L"X", //
		L"#", //

		L'#', Tile::cobblestone, L'X', Item::stick,
		L'M');

	addShapedRecipy(new ItemInstance(Tile::tripWireSource, 2), //
		L"sssctcicig",
		L"I", //
		L"S", //
		L"#", //

		L'#', Tile::wood, L'S', Item::stick, L'I', Item::ironIngot,
		L'M');

	addShapedRecipy(new ItemInstance(Tile::redstoneTorch_on, 1), //
		L"sscicig",
		L"X", //
		L"#", //

		L'#', Item::stick, L'X', Item::redStone,
		L'M');

	addShapedRecipy(new ItemInstance(Item::repeater, 1), //
		L"ssctcictg",
		L"#X#", //
		L"III", //

		L'#', Tile::redstoneTorch_on, L'X', Item::redStone, L'I', Tile::stone,
		L'M');

	addShapedRecipy(new ItemInstance(Item::comparator, 1), //
		L"sssctcictg",
		L" # ", //
		L"#X#", //
		L"III", //

		L'#', Tile::redstoneTorch_on, L'X', Item::netherQuartz, L'I', Tile::stone,
		L'M');

	addShapedRecipy(new ItemInstance(Tile::daylightDetector),
		L"sssctcictg",
		L"GGG",
		L"QQQ",
		L"WWW",

		L'G', Tile::glass, L'Q', Item::netherQuartz, L'W', Tile::woodSlabHalf,
		L'M');

	addShapedRecipy(new ItemInstance(Tile::hopper),
		L"ssscictg",
		L"I I", //
		L"ICI", //
		L" I ", //

		L'I', Item::ironIngot, L'C', Tile::chest,
		L'M');

	addShapedRecipy(new ItemInstance(Item::clock, 1), //
		L"ssscicig",
		L" # ", //
		L"#X#", //
		L" # ", //
		L'#', Item::goldIngot, L'X', Item::redStone,
		L'T');

	addShapelessRecipy(new ItemInstance(Item::eyeOfEnder, 1), //
		L"iig",
		Item::enderPearl, Item::blazePowder,
		L'T');

	addShapelessRecipy(new ItemInstance(Item::fireball, 3), //
		L"iiig",
		Item::gunpowder, Item::blazePowder,Item::coal,
		L'T');

	addShapelessRecipy(new ItemInstance(Item::fireball, 3), //
		L"iizg",
		Item::gunpowder, Item::blazePowder,new ItemInstance(Item::coal, 1, CoalItem::CHAR_COAL),
		L'T');

	addShapedRecipy(new ItemInstance(Item::lead, 2), //
		L"ssscicig",
		L"~~ ", //
		L"~O ", //
		L"  ~", //

		L'~', Item::string, L'O', Item::slimeBall,
		L'T');


	addShapedRecipy(new ItemInstance(Item::compass, 1), //
		L"ssscicig",
		L" # ", //
		L"#X#", //
		L" # ", //

		L'#', Item::ironIngot, L'X', Item::redStone,
		L'T');

	addShapedRecipy(new ItemInstance(Item::map, 1), //
		L"ssscicig",
		L"###", //
		L"#X#", //
		L"###", //

		L'#', Item::paper, L'X', Item::compass,
		L'T');

	addShapedRecipy(new ItemInstance(Tile::button, 1), //
		L"sctg",
		L"#", //

		L'#', Tile::stone,
		L'M');

	addShapedRecipy(new ItemInstance(Tile::button_wood, 1), //
		L"sctg",
		L"#", //

		L'#', Tile::wood,
		L'M');

	addShapedRecipy(new ItemInstance(Tile::pressurePlate_wood, 1), //
		L"sctg",
		L"##", //
		L'#', Tile::wood,
		L'M');

	addShapedRecipy(new ItemInstance(Tile::pressurePlate_stone, 1), //
		L"sctg",
		L"##", //
		L'#', Tile::stone,
		L'M');

	addShapedRecipy(new ItemInstance(Tile::weightedPlate_heavy, 1), //
		L"scig",
		L"##", //

		L'#', Item::ironIngot,
		L'M');

	addShapedRecipy(new ItemInstance(Tile::weightedPlate_light, 1), //
		L"scig",
		L"##", //

		L'#', Item::goldIngot,
		L'M');

	addShapedRecipy(new ItemInstance(Tile::dispenser, 1), //
		L"sssctcicig",
		L"###", //
		L"#X#", //
		L"#R#", //
		L'#', Tile::cobblestone, L'X', Item::bow, L'R', Item::redStone,
		L'M');

	addShapedRecipy(new ItemInstance(Tile::dropper, 1), //
		L"sssctcig",
		L"###", //
		L"# #", //
		L"#R#", //

		L'#', Tile::cobblestone, L'R', Item::redStone,
		L'M');

	addShapedRecipy(new ItemInstance(Item::cauldron, 1), //
		L"ssscig",
		L"# #", //
		L"# #", //
		L"###", //

		L'#', Item::ironIngot,
		L'T');

	addShapedRecipy(new ItemInstance(Item::brewingStand, 1), //
		L"ssctcig",
		L" B ", //
		L"###", //

		L'#', Tile::cobblestone, L'B', Item::blazeRod,
		L'S');


	addShapedRecipy(new ItemInstance(Tile::litPumpkin, 1), //
		L"ssctctg",
		L"A", //
		L"B", //

		L'A', Tile::pumpkin, L'B', Tile::torch,
		L'T');


	addShapedRecipy(new ItemInstance(Tile::jukebox, 1), //
		L"sssctcig",
		L"###", //
		L"#X#", //
		L"###", //

		L'#', Tile::wood, L'X', Item::diamond,
		'D');

	addShapedRecipy(new ItemInstance(Item::paper, 3), //
		L"scig",
		L"###", //

		L'#', Item::reeds,
		L'D');

	addShapelessRecipy(new ItemInstance(Item::book, 1),
		L"iiiig",
		Item::paper,
		Item::paper,
		Item::paper,
		Item::leather,
		L'D');

	//addShapelessRecipy(new ItemInstance(Item.writingBook, 1), //
	//            Item.book, new ItemInstance(Item.dye_powder, 1, DyePowderItem.BLACK), Item.feather);

	addShapedRecipy(new ItemInstance(Tile::noteblock, 1), //
		L"sssctcig",
		L"###", //
		L"#X#", //
		L"###", //

		L'#', Tile::wood, L'X', Item::redStone,
		L'M');

	addShapedRecipy(new ItemInstance(Tile::bookshelf, 1), //
		L"sssctcig",
		L"###", //
		L"XXX", //
		L"###", //

		L'#', Tile::wood, L'X', Item::book,
		L'D');

	addShapedRecipy(new ItemInstance(Item::painting, 1), //
		L"ssscictg",
		L"###", //
		L"#X#", //
		L"###", //

		L'#', Item::stick, L'X', Tile::wool,
		L'D');


	addShapedRecipy(new ItemInstance(Item::frame, 1), //
		L"ssscicig",
		L"###", //
		L"#X#", //
		L"###", //

		L'#', Item::stick, L'X', Item::leather,
		L'D');

	pOreRecipies->addRecipes(this);

	addShapedRecipy(new ItemInstance(Item::goldIngot), //
		L"ssscig",
		L"###", //
		L"###", //
		L"###", //

		L'#', Item::goldNugget,
		L'D');

	addShapedRecipy(new ItemInstance(Item::goldNugget, 9), //
		L"scig",
		L"#", //
		L'#', Item::goldIngot,
		L'D');

	// 4J-PB - moving into decorations to make the structures list smaller
	addShapedRecipy(new ItemInstance(Item::sign, 3), //
		L"sssctcig",
		L"###", //
		L"###", //
		L" X ", //

		L'#', Tile::wood, L'X', Item::stick,
		L'D');

	// 4J - TODO - put these new 1.7.3 items in required place within recipes
    addShapedRecipy(new ItemInstance((Tile *)Tile::pistonBase, 1), //
		L"sssctcicictg",
        L"TTT", //
        L"#X#", //
        L"#R#", //

        L'#', Tile::cobblestone, L'X', Item::ironIngot, L'R', Item::redStone, L'T', Tile::wood,
		L'M');

    addShapedRecipy(new ItemInstance((Tile *)Tile::pistonStickyBase, 1), //
		L"sscictg",
        L"S", //
        L"P", //

        L'S', Item::slimeBall, L'P', Tile::pistonBase,
		L'M');


	// 4J Stu - Added some dummy firework recipes to allow us to navigate forward to the fireworks scene
	addShapedRecipy(new ItemInstance(Item::fireworks, 1), //
		L"sscicig",
		L" P ", //
		L" G ", //

		L'P', Item::paper, L'G', Item::gunpowder,
		L'D');

	addShapedRecipy(new ItemInstance(Item::fireworksCharge,1), //
		L"sscicig",
		L" D ", //
		L" G ", //

		L'D', Item::dye_powder, L'G', Item::gunpowder,
		L'D');

	addShapedRecipy(new ItemInstance(Item::fireworksCharge,1), //
		L"sscicig",
		L" D ", //
		L" C ", //

		L'D', Item::dye_powder, L'C', Item::fireworksCharge,
		L'D');


	// Sort so the largest recipes get checked first!
	/* 4J-PB - TODO
	Collections.sort(recipies, new Comparator<Recipy>()
	{
	public: int compare(Recipy r0, Recipy r1)
			{

				// shapeless recipes are put in the back of the list
				if (r0 instanceof ShapelessRecipy && r1 instanceof ShapedRecipy)
				{
					return 1;
				}
				if (r1 instanceof ShapelessRecipy && r0 instanceof ShapedRecipy)
				{
					return -1;
				}

				if (r1.size() < r0.size()) return -1;
				if (r1.size() > r0.size()) return 1;
				return 0;
			}
	});
	*/

	// 4J-PB removed System.out.println(recipies->size() + L" recipes");

	// 4J-PB - build the array of ingredients required per recipe
	buildRecipeIngredientsArray();
}

// 4J-PB - this function has been substantially changed due to the differences with a va_list of classes in C++ and Java
ShapedRecipy *Recipes::addShapedRecipy(ItemInstance *result, ...)
{
	wstring map = L"";
	int p = 0;
	int width = 0;
	int height = 0;
	int group = ShapedRecipy::eGroupType_Decoration;
	va_list vl;
	wchar_t *wchTypes;
	wchar_t *pwchString;
	wstring wString;
	wstring *wStringA;
	ItemInstance *pItemInstance;
	Tile *pTile;
	Item *pItem;
	wchar_t wchFrom;
	int iCount;
	ItemInstance **ids = NULL;

	myMap *mappings = new unordered_map<wchar_t, ItemInstance *>();

	va_start(vl,result);
	// 4J-PB - second argument is a list of the types
	// s - string
	// w - string array
	// a - char *
	// c - char
	// z - ItemInstance *
	// i - Item *
	// t - Tile *
	// g - group [wt] - which group does the item created by the recipe belong in. Set a default until all recipes have a group

	wchTypes = va_arg(vl,wchar_t *);

	for(int i = 0; wchTypes[i] != L'\0'; ++i )
	{
		if(wchTypes[i+1]==L'\0' && wchTypes[i]!=L'g')
		{
			app.DebugPrintf("Missing group type\n");
		}

		switch(wchTypes[i])
		{
		case L'a':
			pwchString=va_arg(vl,wchar_t *);
			wString=pwchString;
			height++;
			width = (int)wString.length();
			map += wString;
			break;
		case L's':
			pwchString=va_arg(vl,wchar_t *);
			wString=pwchString;
			height++;
			width = (int)wString.length();
			map += wString;
			break;
		case L'w':
			wStringA=va_arg(vl,wstring *);
			iCount=0;
			do
			{
				wString=wStringA[iCount++];
				if(!wString.empty())
				{
					height++;
					width = (int)wString.length();
					map += wString;
				}
			}
			while(!wString.empty());

			break;
		case L'c':
			wchFrom=va_arg(vl,wchar_t);
			break;
		case L'z':
			pItemInstance=va_arg(vl,ItemInstance *);
			mappings->insert(myMap::value_type(wchFrom,pItemInstance));
			break;
		case L'i':
			pItem=va_arg(vl,Item *);
			pItemInstance= new ItemInstance(pItem,1,ANY_AUX_VALUE);
			mappings->insert(myMap::value_type(wchFrom,pItemInstance));
			break;
		case L't':
			pTile=va_arg(vl,Tile *);
			pItemInstance= new ItemInstance(pTile,1,ANY_AUX_VALUE);
			mappings->insert(myMap::value_type(wchFrom,pItemInstance));
			break;
		case L'g':
			wchFrom=va_arg(vl,wchar_t);
			switch(wchFrom)
			{
// 			case L'W':
// 				group=ShapedRecipy::eGroupType_Weapon;
// 				break;
			case L'T':
				group=ShapedRecipy::eGroupType_Tool;
				break;
			case L'A':
				group=ShapedRecipy::eGroupType_Armour;
				break;
			case L'S':
				group=ShapedRecipy::eGroupType_Structure;
				break;
			case L'V':
				group=ShapedRecipy::eGroupType_Transport;
				break;
			case L'M':
				group=ShapedRecipy::eGroupType_Mechanism;
				break;
			case L'F':
				group=ShapedRecipy::eGroupType_Food;
				break;
			case L'D':
			default:
				group=ShapedRecipy::eGroupType_Decoration;
				break;

			}
			break;

		}


		ids = new ItemInstance *[width * height];

		for (int j = 0; j < width * height; j++)
		{
			wchar_t ch = map[j];
			myMap::iterator it=mappings->find(ch);
			if (it != mappings->end())
			{
				ids[j] =it->second;
			}
			else
			{
				ids[j] = NULL;
			}
		}
	}

	va_end(vl);

	ShapedRecipy *recipe = new ShapedRecipy(width, height, ids, result, group);
	recipies->push_back(recipe);
	return recipe;
}

void Recipes::addShapelessRecipy(ItemInstance *result,... )
{
	va_list vl;
	wchar_t *szTypes;
	wstring String;
	ItemInstance *pItemInstance;
	Tile *pTile;
	Item *pItem;
	Recipy::_eGroupType group = Recipy::eGroupType_Decoration;
	wchar_t wchFrom;
	vector <ItemInstance *> *ingredients = new vector<ItemInstance *>();

	va_start(vl,result);
	// 4J-PB - second argument is a list of the types
	// z - ItemInstance *
	// i - Item *
	// t - Tile *
	szTypes = va_arg(vl,wchar_t *);

	for(int i = 0; szTypes[i] != L'\0'; ++i )
	{
		switch(szTypes[i])
		{
		case L'z':
			pItemInstance=va_arg(vl,ItemInstance *);
			// 4J-PB - original code copies the item instance, copy the pointer isnt the same...
			// TODO
			ingredients->push_back(pItemInstance->copy_not_shared());
			break;
		case L'i':
			pItem=va_arg(vl,Item *);
			pItemInstance= new ItemInstance(pItem);
			ingredients->push_back(pItemInstance);
			break;
		case L't':
			pTile=va_arg(vl,Tile *);
			ingredients->push_back(new ItemInstance(pTile));
			break;
		case L'g':
			wchFrom=va_arg(vl,wchar_t);
			switch(wchFrom)
			{

			case L'T':
				group=Recipy::eGroupType_Tool;
				break;
			case L'A':
				group=Recipy::eGroupType_Armour;
				break;
			case L'S':
				group=Recipy::eGroupType_Structure;
				break;
			case L'V':
				group=Recipy::eGroupType_Transport;
				break;
			case L'M':
				group=Recipy::eGroupType_Mechanism;
				break;
			case L'F':
				group=Recipy::eGroupType_Food;
				break;
			case L'D':
			default:
				group=Recipy::eGroupType_Decoration;
				break;

			}
			break;		}
	}

	recipies->push_back(new ShapelessRecipy(result, ingredients, group));
}

shared_ptr<ItemInstance> Recipes::getItemFor(shared_ptr<CraftingContainer> craftSlots, Level *level, Recipy *recipesClass /*= NULL*/)
{
	int count = 0;
	shared_ptr<ItemInstance> first = nullptr;
	shared_ptr<ItemInstance> second = nullptr;
	for (int i = 0; i < craftSlots->getContainerSize(); i++)
	{
		shared_ptr<ItemInstance> item = craftSlots->getItem(i);
		if (item != NULL)
		{
			if (count == 0) first = item;
			if (count == 1) second = item;
			count++;
		}
	}

	if (count == 2 && first->id == second->id && first->count == 1 && second->count == 1 && Item::items[first->id]->canBeDepleted())
	{
		Item *item = Item::items[first->id];
		int remaining1 = item->getMaxDamage() - first->getDamageValue();
		int remaining2 = item->getMaxDamage() - second->getDamageValue();
		int remaining = (remaining1 + remaining2) + item->getMaxDamage() * 5 / 100;
		int resultDamage = item->getMaxDamage() - remaining;
		if (resultDamage < 0) resultDamage = 0;
		return shared_ptr<ItemInstance>( new ItemInstance(first->id, 1, resultDamage) );
	}

	if(recipesClass != NULL)
	{
		if (recipesClass->matches(craftSlots, level)) return recipesClass->assemble(craftSlots);
	}
	else
	{
		for (auto& r : *recipies)
		{
			if (r->matches(craftSlots, level)) return r->assemble(craftSlots);
		}
	}
	return nullptr;
}

vector <Recipy *> *Recipes::getRecipies()
{
	return recipies;
}

// 4J-PB - added to deal with Xb0x 'crafting'
shared_ptr<ItemInstance> Recipes::getItemForRecipe(Recipy *r)
{
	return r->assemble(nullptr);
}

// 4J-PB - build the required ingredients for recipes
void Recipes::buildRecipeIngredientsArray(void)
{
	//RecipyList *recipes = ((Recipes *)Recipes::getInstance())->getRecipies();

	int iRecipeC=(int)recipies->size();

	m_pRecipeIngredientsRequired= new Recipy::INGREDIENTS_REQUIRED [iRecipeC];

	int iCount=0;
	for (auto& recipe : *recipies)
	{
		recipe->requires(&m_pRecipeIngredientsRequired[iCount++]);
	}

	//printf("Total recipes in buildRecipeIngredientsArray - %d",iCount);
}

Recipy::INGREDIENTS_REQUIRED *Recipes::getRecipeIngredientsArray(void)
{
	return m_pRecipeIngredientsRequired;
}