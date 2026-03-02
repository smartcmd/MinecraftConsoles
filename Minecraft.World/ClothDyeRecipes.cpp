#include "stdafx.h"
#include "net.minecraft.world.Item.h"
#include "net.minecraft.world.level.tile.h"
#include "Recipy.h"
#include "Recipes.h"
#include "ClothDyeRecipes.h"

void ClothDyeRecipes::addRecipes(Recipes *r) 
{
	// recipes for converting cloth to colored cloth using dye
	for (int i = 0; i < 16; i++) 
	{
		r->addShapelessRecipy(new ItemInstance(Tile::wool, 1, ColoredTile::getItemAuxValueForTileData(i)), //
			L"zzg",
			new ItemInstance(Item::dye_powder, 1, i), new ItemInstance(Item::items[Tile::wool_Id], 1, 0),L'D');
		r->addShapedRecipy(new ItemInstance(Tile::clayHardened_colored, 8, ColoredTile::getItemAuxValueForTileData(i)), //
			L"sssczczg",
			L"###",
			L"#X#",
			L"###",
			L'#', new ItemInstance(Tile::clayHardened),
			L'X', new ItemInstance(Item::dye_powder, 1, i),L'D');

#if 0
		r->addShapedRecipy(new ItemInstance(Tile::stained_glass, 8, ColoredTile::getItemAuxValueForTileData(i)), //
			L"sssczczg",
			L"###",
			L"#X#",
			L"###",
			L'#', new ItemInstance(Tile::glass),
			L'X', new ItemInstance(Item::dye_powder, 1, i), L'D');
		r->addShapedRecipy(new ItemInstance(Tile::stained_glass_pane, 16, i), //
			L"ssczg",
			L"###",
			L"###",
			L'#', new ItemInstance(Tile::stained_glass, 1, i), L'D');
#endif
	}

	// some dye recipes
	r->addShapelessRecipy(new ItemInstance(Item::dye_powder, 2, DyePowderItem::YELLOW),
		L"tg", 
		Tile::flower,L'D');

	r->addShapelessRecipy(new ItemInstance(Item::dye_powder, 2, DyePowderItem::RED),
		L"tg", 
		Tile::rose,L'D');

	r->addShapelessRecipy(new ItemInstance(Item::dye_powder, 3, DyePowderItem::WHITE),
		L"ig", 
		Item::bone,L'D');

	r->addShapelessRecipy(new ItemInstance(Item::dye_powder, 2, DyePowderItem::PINK), //
		L"zzg",
		new ItemInstance(Item::dye_powder, 1, DyePowderItem::RED), 
		new ItemInstance(Item::dye_powder, 1, DyePowderItem::WHITE),L'D');

	r->addShapelessRecipy(new ItemInstance(Item::dye_powder, 2, DyePowderItem::ORANGE), //
		L"zzg",
		new ItemInstance(Item::dye_powder, 1, DyePowderItem::RED), 
		new ItemInstance(Item::dye_powder, 1, DyePowderItem::YELLOW),L'D');

	r->addShapelessRecipy(new ItemInstance(Item::dye_powder, 2, DyePowderItem::LIME), //
		L"zzg",
		new ItemInstance(Item::dye_powder, 1, DyePowderItem::GREEN), 
		new ItemInstance(Item::dye_powder, 1, DyePowderItem::WHITE),L'D');

	r->addShapelessRecipy(new ItemInstance(Item::dye_powder, 2, DyePowderItem::GRAY), //
		L"zzg",
		new ItemInstance(Item::dye_powder, 1, DyePowderItem::BLACK), 
		new ItemInstance(Item::dye_powder, 1, DyePowderItem::WHITE),L'D');

	r->addShapelessRecipy(new ItemInstance(Item::dye_powder, 2, DyePowderItem::SILVER), //
		L"zzg",
		new ItemInstance(Item::dye_powder, 1, DyePowderItem::GRAY), 
		new ItemInstance(Item::dye_powder, 1, DyePowderItem::WHITE),L'D');

	r->addShapelessRecipy(new ItemInstance(Item::dye_powder, 3, DyePowderItem::SILVER), //
		L"zzzg",
		new ItemInstance(Item::dye_powder, 1, DyePowderItem::BLACK), 
		new ItemInstance(Item::dye_powder, 1, DyePowderItem::WHITE), 
		new ItemInstance(Item::dye_powder, 1, DyePowderItem::WHITE),L'D');

	r->addShapelessRecipy(new ItemInstance(Item::dye_powder, 2, DyePowderItem::LIGHT_BLUE), //
		L"zzg",
		new ItemInstance(Item::dye_powder, 1, DyePowderItem::BLUE), 
		new ItemInstance(Item::dye_powder, 1, DyePowderItem::WHITE),L'D');

	r->addShapelessRecipy(new ItemInstance(Item::dye_powder, 2, DyePowderItem::CYAN), //
		L"zzg",
		new ItemInstance(Item::dye_powder, 1, DyePowderItem::BLUE), 
		new ItemInstance(Item::dye_powder, 1, DyePowderItem::GREEN),L'D');

	r->addShapelessRecipy(new ItemInstance(Item::dye_powder, 2, DyePowderItem::PURPLE), //
		L"zzg",
		new ItemInstance(Item::dye_powder, 1, DyePowderItem::BLUE), 
		new ItemInstance(Item::dye_powder, 1, DyePowderItem::RED),L'D');

	r->addShapelessRecipy(new ItemInstance(Item::dye_powder, 2, DyePowderItem::MAGENTA), //
		L"zzg",
		new ItemInstance(Item::dye_powder, 1, DyePowderItem::PURPLE), 
		new ItemInstance(Item::dye_powder, 1, DyePowderItem::PINK),L'D');

	r->addShapelessRecipy(new ItemInstance(Item::dye_powder, 3, DyePowderItem::MAGENTA), //
		L"zzzg",
		new ItemInstance(Item::dye_powder, 1, DyePowderItem::BLUE), 
		new ItemInstance(Item::dye_powder, 1, DyePowderItem::RED), 
		new ItemInstance(Item::dye_powder, 1, DyePowderItem::PINK),L'D');

	r->addShapelessRecipy(new ItemInstance(Item::dye_powder, 4, DyePowderItem::MAGENTA), //
		L"zzzzg",
		new ItemInstance(Item::dye_powder, 1, DyePowderItem::BLUE), 
		new ItemInstance(Item::dye_powder, 1, DyePowderItem::RED), 
		new ItemInstance(Item::dye_powder, 1, DyePowderItem::RED),
		new ItemInstance(Item::dye_powder, 1, DyePowderItem::WHITE),L'D');

	for (int i = 0; i < 16; i++)
	{
		r->addShapedRecipy(new ItemInstance(Tile::woolCarpet, 3, i),
			L"sczg",
			L"##",
			L'#', new ItemInstance(Tile::wool, 1, i),
			L'D'
			);
	}
}

