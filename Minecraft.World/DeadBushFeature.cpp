#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "DeadBushFeature.h"
#include "net.minecraft.world.level.tile.h"

DeadBushFeature::DeadBushFeature(int tile)
{
	this->tile = tile;
}

bool DeadBushFeature::place(Level *level, Random *random, int x, int y, int z)
{
	int t = 0;
	while (((t = level->getTile(x, y, z)) == 0 || t == Tile::leaves_Id) && y > 0)
		y--;

	for (int i = 0; i < 4; i++)
	{
		int x2 = x + random->nextInt(8) - random->nextInt(8);
		int y2 = y + random->nextInt(4) - random->nextInt(4);
		int z2 = z + random->nextInt(8) - random->nextInt(8);
		if (level->isEmptyTile(x2, y2, z2) )
		{
			if (Tile::tiles[tile]->canSurvive(level, x2, y2, z2))
			{
				level->setTileAndData(x2, y2, z2, tile, 0, Tile::UPDATE_CLIENTS);
			}
		}
	}

	return true;

}