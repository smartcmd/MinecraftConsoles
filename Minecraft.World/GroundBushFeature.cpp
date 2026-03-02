#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "GroundBushFeature.h"

GroundBushFeature::GroundBushFeature(int trunkType, int leafType)
{
	trunkTileType = trunkType;
	leafTileType = leafType;
}

bool GroundBushFeature::place(Level *level, Random *random, int x, int y, int z)
{
	PIXBeginNamedEvent(0,"Placing GroundBushFeature");
	int t = 0;
	while (((t = level->getTile(x, y, z)) == 0 || t == Tile::leaves_Id) && y > 0)
		y--;

	int tile = level->getTile(x, y, z);
	if (tile == Tile::dirt_Id || tile == Tile::grass_Id)
	{
		y++;
		placeBlock(level, x, y, z, Tile::treeTrunk_Id, trunkTileType);

		for (int yy = y; yy <= y + 2; yy++)
		{
			int yo = yy - y;
			int offs = 2 - yo;
			for (int xx = x - offs; xx <= x + offs; xx++)
			{
				int xo = xx - (x);
				for (int zz = z - offs; zz <= z + offs; zz++)
				{
					int zo = zz - (z);
					if (abs(xo) == offs && abs(zo) == offs && random->nextInt(2) == 0) continue;
					if (!Tile::solid[level->getTile(xx, yy, zz)]) placeBlock(level, xx, yy, zz, Tile::leaves_Id, leafTileType);
				}
			}
		}
	}
	PIXEndNamedEvent();
	return true;
}