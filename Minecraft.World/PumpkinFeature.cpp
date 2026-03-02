#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "PumpkinFeature.h"

bool PumpkinFeature::place(Level *level, Random *random, int x, int y, int z)
{
	for (int i = 0; i < 64; i++)
	{
		int x2 = x + random->nextInt(8) - random->nextInt(8);
		int y2 = y + random->nextInt(4) - random->nextInt(4);
		int z2 = z + random->nextInt(8) - random->nextInt(8);
		if (level->isEmptyTile(x2, y2, z2) && level->getTile(x2, y2 - 1, z2) == Tile::grass_Id)
		{
			if (Tile::pumpkin->mayPlace(level, x2, y2, z2))
			{
				level->setTileAndData(x2, y2, z2, Tile::pumpkin_Id, random->nextInt(4), Tile::UPDATE_CLIENTS);
			}
		}
	}

	return true;
}