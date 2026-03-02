#include "stdafx.h"
#include "net.minecraft.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.item.h"
#include "Facing.h"
#include "GenericStats.h"
#include "BedItem.h"

BedItem::BedItem(int id) : Item( id )
{
}

bool BedItem::useOn(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly)
{
	if (face != Facing::UP)
	{
		return false;
	}

	// place on top of tile
	y = y + 1;

	BedTile *tile = (BedTile *) Tile::bed;

	int dir = ( Mth::floor(player->yRot * 4 / (360) + 0.5f) ) & 3;
	int xra = 0;
	int zra = 0;

	if (dir == Direction::SOUTH) zra = 1;
	if (dir == Direction::WEST) xra = -1;
	if (dir == Direction::NORTH) zra = -1;
	if (dir == Direction::EAST) xra = 1;

	if (!player->mayBuild(x, y, z) || !player->mayBuild(x + xra, y, z + zra)) return false;

	if (level->isEmptyTile(x, y, z) && level->isEmptyTile(x + xra, y, z + zra) && level->isTopSolidBlocking(x, y - 1, z) && level->isTopSolidBlocking(x + xra, y - 1, z + zra))
	{
		// 4J-PB - Adding a test only version to allow tooltips to be displayed
		if(!bTestUseOnOnly)
		{
			level->setTileAndData(x, y, z, tile->id, dir);
			// double-check that the bed was successfully placed
			if (level->getTile(x, y, z) == tile->id)
			{
				// 4J-JEV: Hook for durango 'BlockPlaced' event.
				player->awardStat(GenericStats::blocksPlaced(tile->id), GenericStats::param_blocksPlaced(tile->id,itemInstance->getAuxValue(),1));

				level->setTileAndData(x + xra, y, z + zra, tile->id, dir + BedTile::HEAD_PIECE_DATA);
			}

			itemInstance->count--;
		}
		return true;
	}

	return false;
}
