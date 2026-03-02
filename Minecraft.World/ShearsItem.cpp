#include "stdafx.h"
#include "ShearsItem.h"
#include "Tile.h"
#include "net.minecraft.world.entity.h"

ShearsItem::ShearsItem(int itemId) : Item(itemId)
{
	setMaxStackSize(1);
	setMaxDamage(238);
}

bool ShearsItem::mineBlock(shared_ptr<ItemInstance> itemInstance, Level *level, int tile, int x, int y, int z, shared_ptr<LivingEntity> owner)
{
	if (tile == Tile::leaves_Id || tile == Tile::web_Id || tile == Tile::tallgrass_Id || tile == Tile::vine_Id || tile == Tile::tripWire_Id)
	{
		itemInstance->hurtAndBreak(1, owner);
		return true;
	}
	return Item::mineBlock(itemInstance, level, tile, x, y, z, owner);
}

bool ShearsItem::canDestroySpecial(Tile *tile)
{
	return tile->id == Tile::web_Id || tile->id == Tile::redStoneDust_Id || tile->id == Tile::tripWire_Id;
}

float ShearsItem::getDestroySpeed(shared_ptr<ItemInstance> itemInstance, Tile *tile)
{
	if (tile->id == Tile::web_Id || tile->id == Tile::leaves_Id)
	{
		return 15;
	}
	if (tile->id == Tile::wool_Id)
	{
		return 5;
	}
	return Item::getDestroySpeed(itemInstance, tile);
}