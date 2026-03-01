#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "LightGemFeature.h"

bool LightGemFeature::place(Level *level, Random *random, int x, int y, int z)
{
	if (!level->isEmptyTile(x, y, z)) return false;
	if (level->getTile(x, y + 1, z) != Tile::netherRack_Id) return false;
	level->setTileAndData(x, y, z, Tile::glowstone_Id, 0, Tile::UPDATE_CLIENTS);

	for (int i = 0; i < 1500; i++)
	{
		int x2 = x + random->nextInt(8) - random->nextInt(8);
		int y2 = y - random->nextInt(12);
		int z2 = z + random->nextInt(8) - random->nextInt(8);
		if (level->getTile(x2, y2, z2) != 0) continue;

		int count = 0;
		for (int t = 0; t < 6; t++)
		{
			int tile = 0;
			if (t == 0) tile = level->getTile(x2 - 1, y2, z2);
			if (t == 1) tile = level->getTile(x2 + 1, y2, z2);
			if (t == 2) tile = level->getTile(x2, y2 - 1, z2);
			if (t == 3) tile = level->getTile(x2, y2 + 1, z2);
			if (t == 4) tile = level->getTile(x2, y2, z2 - 1);
			if (t == 5) tile = level->getTile(x2, y2, z2 + 1);

			if (tile == Tile::glowstone_Id) count++;
		}

		if (count == 1) level->setTileAndData(x2, y2, z2, Tile::glowstone_Id, 0, Tile::UPDATE_CLIENTS);
	}

	return true;
}