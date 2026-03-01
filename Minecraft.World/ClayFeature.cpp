#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.material.h"
#include "ClayFeature.h"
#include "net.minecraft.world.level.tile.h"

ClayFeature::ClayFeature(int radius)
{
	this->tile = Tile::clay_Id;
	this->radius = radius;
}

bool ClayFeature::place(Level *level, Random *random, int x, int y, int z)
{
	if (level->getMaterial(x, y, z) != Material::water) return false;

	int r = random->nextInt(radius - 2) + 2;
	int yr = 1;
	for (int xx = x - r; xx <= x + r; xx++)
	{
		for (int zz = z - r; zz <= z + r; zz++)
		{
			int xd = xx - x;
			int zd = zz - z;
			if (xd * xd + zd * zd > r * r) continue;
			for (int yy = y - yr; yy <= y + yr; yy++)
			{
				int t = level->getTile(xx, yy, zz);
				if (t == Tile::dirt_Id || t == Tile::clay_Id)
				{
					level->setTileAndData(xx, yy, zz, tile, 0, Tile::UPDATE_CLIENTS);
				}
			}
		}
	}

	return true;
}
