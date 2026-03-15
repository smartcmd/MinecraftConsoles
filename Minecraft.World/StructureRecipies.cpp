#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.tile.h"
#include "Recipy.h"
#include "Recipes.h"
#include "StructureRecipies.h"

void StructureRecipies::addRecipes(Recipes *r) 
{
	r->addShapedRecipy(new ItemInstance(Tile::sandStone), //
		L"ssctg",
		L"##", //
		L"##", //

		L'#', Tile::sand,
		L'S');

	r->addShapedRecipy(new ItemInstance(Tile::sandStone, 4, SandStoneTile::TYPE_SMOOTHSIDE), //
		L"ssczg",
		L"##", //
		L"##", //

		L'#', new ItemInstance(Tile::sandStone),
		L'S');

	r->addShapedRecipy(new ItemInstance(Tile::sandStone, 1, SandStoneTile::TYPE_HEIROGLYPHS), //
		L"ssczg",
		L"#", //
		L"#", //

		L'#', new ItemInstance(Tile::stoneSlabHalf, 1, StoneSlabTile::SAND_SLAB),
		L'S');

	r->addShapedRecipy(new ItemInstance(Tile::quartzBlock, 1, QuartzBlockTile::TYPE_CHISELED), //
		L"ssczg",
		L"#", //
		L"#", //

		L'#', new ItemInstance(Tile::stoneSlabHalf, 1, StoneSlabTile::QUARTZ_SLAB),
		L'S');

	r->addShapedRecipy(new ItemInstance(Tile::quartzBlock, 2, QuartzBlockTile::TYPE_LINES_Y), //
		L"ssczg",
		L"#", //
		L"#", //

		L'#', new ItemInstance(Tile::quartzBlock, 1, QuartzBlockTile::TYPE_DEFAULT),
		L'S');

	// 4J Stu - Changed the order, as the blocks that go with sandstone cause a 3-icon scroll
	// that touches the text "Structures" in the title in 720 fullscreen.
	r->addShapedRecipy(new ItemInstance(Tile::workBench), //
		L"ssctg",
		L"##", //
		L"##", //

		L'#', Tile::wood,
		L'S');
	
	r->addShapedRecipy(new ItemInstance(Tile::furnace), //
		L"sssctg",
		L"###", //
		L"# #", //
		L"###", //

		L'#', Tile::cobblestone,
		L'S');

	r->addShapedRecipy(new ItemInstance(static_cast<Tile *>(Tile::chest)), //
		L"sssctg",
		L"###", //
		L"# #", //
		L"###", //

		L'#', Tile::wood,
		L'S');

	r->addShapedRecipy(new ItemInstance(Tile::chest_trap), //
		L"sctctg",
		L"#-", //

		L'#', Tile::chest, L'-', Tile::tripWireSource,
		L'S');

	r->addShapedRecipy(new ItemInstance(Tile::enderChest), //
		L"sssctcig",
		L"###", //
		L"#E#", //
		L"###", //

		L'#', Tile::obsidian, L'E', Item::eyeOfEnder,
		L'S');

	r->addShapedRecipy(new ItemInstance(Tile::stoneBrick, 4), //
		L"ssctg",
		L"##", //
		L"##", //

		L'#', Tile::stone,
		L'S');

	// 4J Stu - Move this into "Recipes" to change the order things are displayed on the crafting menu
	//r->addShapedRecipy(new ItemInstance(Tile::ironFence, 16), //
	//	L"sscig",
	//	L"###", //
	//	L"###", //

	//	L'#', Item::ironIngot,
	//	L'S');

	r->addShapedRecipy(new ItemInstance(Tile::thinGlass, 16), //
		L"ssctg",
		L"###", //
		L"###", //

		L'#', Tile::glass,
		L'D');
	



// Stained Glass block + pane per color
for (int i = 0; i < 16; i++)
{
    r->addShapedRecipy(new ItemInstance(Tile::stained_glass, 8, ColoredTile::getItemAuxValueForTileData(i)),
        L"sssczczg",
        L"###",
        L"#X#",
        L"###",
        L'#', new ItemInstance(Tile::glass),
        L'X', new ItemInstance(Item::dye_powder, 1, i),
        L'D');
    r->addShapedRecipy(new ItemInstance(Tile::stained_glass_pane, 16, ColoredTile::getItemAuxValueForTileData(i)),
        L"ssczg",
        L"###",
        L"###",
        L'#', new ItemInstance(Tile::stained_glass, 1, ColoredTile::getItemAuxValueForTileData(i)),
        L'D');
}

	r->addShapedRecipy(new ItemInstance(Tile::netherBrick, 1), //
		L"sscig",
		L"NN", //
		L"NN", //

		L'N', Item::netherbrick,
		L'S');

	r->addShapedRecipy(new ItemInstance(Tile::redstoneLight, 1), //
		L"ssscictg",
		L" R ", //
		L"RGR", //
		L" R ", //
		L'R', Item::redStone, 'G', Tile::glowstone,
		L'M');

	r->addShapedRecipy(new ItemInstance(Tile::beacon, 1), //
		L"sssctcictg",
		L"GGG", //
		L"GSG", //
		L"OOO", //

		L'G', Tile::glass, L'S', Item::netherStar, L'O', Tile::obsidian,
		L'M');
}