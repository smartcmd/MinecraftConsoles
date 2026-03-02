#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "DyePowderItem.h"
#include "Tile.h"
#include "Recipy.h"
#include "Recipes.h"
#include "FoodRecipies.h"

void FoodRecipies::addRecipes(Recipes *r) 
{
	// 4J-JEV: Bumped up in the list to avoid a colision with the title.
	r->addShapedRecipy(new ItemInstance(Item::apple_gold, 1, 0), //
		L"ssscicig",
		L"###", //
		L"#X#", //
		L"###", //
		L'#', Item::goldIngot, L'X', Item::apple,
		L'F');

	r->addShapedRecipy(new ItemInstance(Item::apple_gold, 1, 1), //
		L"sssctcig",
		L"###", //
		L"#X#", //
		L"###", //
		L'#', Tile::goldBlock, L'X', Item::apple,
		L'F');

	r->addShapedRecipy(new ItemInstance(Item::speckledMelon, 1), //
		L"ssscicig",
		L"###", //
		L"#X#", //
		L"###", //

		L'#', Item::goldNugget, L'X', Item::melon,
		L'F');
	
	r->addShapelessRecipy(new ItemInstance(Item::mushroomStew), 
		L"ttig", 
		Tile::mushroom_brown, Tile::mushroom_red, Item::bowl,
		L'F');
      
	r->addShapedRecipy(new ItemInstance(Item::cookie, 8), //
				L"sczcig",
                L"#X#", //

                L'X', new ItemInstance(Item::dye_powder, 1, DyePowderItem::BROWN),
				L'#', Item::wheat,
				L'F');

	r->addShapedRecipy(new ItemInstance(Tile::melon), //
		L"ssscig",
		L"MMM", //
		L"MMM", //
		L"MMM", //

		L'M', Item::melon,
		L'F');

	r->addShapedRecipy(new ItemInstance(Item::seeds_melon), //
		L"scig",
		L"M", //

		L'M', Item::melon,
		L'F');

	r->addShapedRecipy(new ItemInstance(Item::seeds_pumpkin, 4), //
		L"sctg",
		L"M", //

		L'M', Tile::pumpkin,
		L'F');

	r->addShapelessRecipy(new ItemInstance(Item::pumpkinPie), //
		L"tiig",
		Tile::pumpkin, Item::sugar, Item::egg,
		L'F');

	r->addShapedRecipy(new ItemInstance(Item::carrotGolden, 1, 0), //
		L"ssscicig",
		L"###", //
		L"#X#", //
		L"###", //

		L'#', Item::goldNugget, L'X', Item::carrots,
		L'F');

	r->addShapelessRecipy(new ItemInstance(Item::fermentedSpiderEye), //
		L"itig",
		Item::spiderEye, Tile::mushroom_brown, Item::sugar,
		L'F');

	r->addShapelessRecipy(new ItemInstance(Item::blazePowder, 2), //
		L"ig",
		Item::blazeRod,
		L'F');

	r->addShapelessRecipy(new ItemInstance(Item::magmaCream), //
		L"iig",
		Item::blazePowder, Item::slimeBall,
		L'F');
}

