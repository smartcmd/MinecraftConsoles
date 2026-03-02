#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "CactusFeature.h"
#include "net.minecraft.world.level.tile.h"

bool CactusFeature::place(Level *level, Random *random, int x, int y, int z)
{
	for (int i = 0; i < 10; i++) {
		int x2 = x + random->nextInt(8) - random->nextInt(8);
		int y2 = y + random->nextInt(4) - random->nextInt(4);
		int z2 = z + random->nextInt(8) - random->nextInt(8);
		if (level->isEmptyTile(x2, y2, z2))
		{
			int h = 1 + random->nextInt(random->nextInt(3) + 1);
			for (int yy = 0; yy < h; yy++)
			{
				if (Tile::cactus->canSurvive(level, x2, y2+yy, z2))
				{
					level->setTileAndData(x2, y2+yy, z2, Tile::cactus_Id, 0, Tile::UPDATE_CLIENTS);
				}
			}
		}
	}

	return true;
}

