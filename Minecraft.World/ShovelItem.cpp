#include "stdafx.h"

#include "net.minecraft.world.level.tile.h"
#include "ShovelItem.h"

TileArray *ShovelItem::diggables = NULL;

void ShovelItem::staticCtor()
{
	ShovelItem::diggables = new TileArray( SHOVEL_DIGGABLES);
	diggables->data[0] = Tile::grass;
	diggables->data[1] = Tile::dirt;
	diggables->data[2] = Tile::sand;
	diggables->data[3] = Tile::gravel;
	diggables->data[4] = Tile::topSnow;
	diggables->data[5] = Tile::snow;
	diggables->data[6] = Tile::clay;
	diggables->data[7] = Tile::farmland;
	diggables->data[8] = Tile::soulsand;
	diggables->data[9] = Tile::mycel;
}

ShovelItem::ShovelItem(int id, const Tier *tier) : DiggerItem(id, 1, tier, diggables)
{
}

bool ShovelItem::canDestroySpecial(Tile *tile)
{
	if (tile == Tile::topSnow) return true;
	if (tile == Tile::snow) return true;
	return false;
}