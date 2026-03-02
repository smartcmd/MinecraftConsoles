#include "stdafx.h"
#include "net.minecraft.world.level.tile.h"
#include "HatchetItem.h"

TileArray *HatchetItem::diggables = NULL;

void HatchetItem::staticCtor()
{
	HatchetItem::diggables = new TileArray( HATCHET_DIGGABLES);
	diggables->data[0] = Tile::wood;
	diggables->data[1] = Tile::bookshelf;
	diggables->data[2] = Tile::treeTrunk;
	diggables->data[3] = Tile::chest;
	// 4J - brought forward from 1.2.3
	diggables->data[4] = Tile::stoneSlab;
	diggables->data[5] = Tile::stoneSlabHalf;
	diggables->data[6] = Tile::pumpkin;
	diggables->data[7] = Tile::litPumpkin;
}

HatchetItem::HatchetItem(int id, const Tier *tier) : DiggerItem (id, 3, tier, diggables)
{
}

// 4J - brought forward from 1.2.3
float HatchetItem::getDestroySpeed(shared_ptr<ItemInstance> itemInstance, Tile *tile)
{
    if (tile != NULL && (tile->material == Material::wood  || tile->material == Material::plant || tile->material == Material::replaceable_plant))
	{
        return speed;
    }
    return DiggerItem::getDestroySpeed(itemInstance, tile);
}