#include "stdafx.h"
#include "WaterlilyFeature.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.h"

bool WaterlilyFeature::place(Level *level, Random *random, int x, int y, int z)
{
	for (int i = 0; i < 10; i++)
	{
		int x2 = x + random->nextInt(8) - random->nextInt(8);
		int y2 = y + random->nextInt(4) - random->nextInt(4);
		int z2 = z + random->nextInt(8) - random->nextInt(8);
		if (level->isEmptyTile(x2, y2, z2))
		{
			if (Tile::waterLily->mayPlace(level, x2, y2, z2))
			{
				level->setTileAndData(x2, y2, z2, Tile::waterLily_Id, 0, Tile::UPDATE_CLIENTS);
			}
		}
	}

	return true;
}