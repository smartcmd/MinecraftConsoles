#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "BirchFeature.h"
#include "net.minecraft.world.level.tile.h"

BirchFeature::BirchFeature(bool doUpdate) : Feature(doUpdate)
{
}

bool BirchFeature::place(Level *level, Random *random, int x, int y, int z)
{
	int treeHeight = random->nextInt(3) + 5;

	bool free = true;
	if (y < 1 || y + treeHeight + 1 > Level::maxBuildHeight) return false;

	for (int yy = y; yy <= y + 1 + treeHeight; yy++)
	{
		int r = 1;
		if (yy == y) r = 0;
		if (yy >= y + 1 + treeHeight - 2) r = 2;
		for (int xx = x - r; xx <= x + r && free; xx++)
		{
			for (int zz = z - r; zz <= z + r && free; zz++)
			{
				if (yy >= 0 && yy < Level::maxBuildHeight)
				{
					int tt = level->getTile(xx, yy, zz);
					if (tt != 0 && tt != Tile::leaves_Id) free = false;
				}
				else
				{
					free = false;
				}
			}
		}
	}

	if (!free) return false;

	int belowTile = level->getTile(x, y - 1, z);
	if ((belowTile != Tile::grass_Id && belowTile != Tile::dirt_Id) || y >= Level::maxBuildHeight - treeHeight - 1) return false;

	// 4J Stu Added to stop tree features generating areas previously place by game rule generation
	if(app.getLevelGenerationOptions() != NULL)
	{
		LevelGenerationOptions *levelGenOptions = app.getLevelGenerationOptions();
		int radius = 3;
		bool intersects = levelGenOptions->checkIntersects(x - radius, y - 1, z - radius, x + radius, y + treeHeight, z + radius);
		if(intersects)
		{
			//app.DebugPrintf("Skipping reeds feature generation as it overlaps a game rule structure\n");
			return false;
		}
	}

	placeBlock(level, x, y - 1, z, Tile::dirt_Id);

	for (int yy = y - 3 + treeHeight; yy <= y + treeHeight; yy++)
	{
		int yo = yy - (y + treeHeight);
		int offs = 1 - yo / 2;
		for (int xx = x - offs; xx <= x + offs; xx++)
		{
			int xo = xx - (x);
			for (int zz = z - offs; zz <= z + offs; zz++)
			{
				int zo = zz - (z);
				if (abs(xo) == offs && abs(zo) == offs && (random->nextInt(2) == 0 || yo == 0)) continue;
				int t = level->getTile(xx, yy, zz);
				if (t == 0 || t == Tile::leaves_Id) placeBlock(level, xx, yy, zz, Tile::leaves_Id, LeafTile::BIRCH_LEAF);
			}
		}
	}
	for (int hh = 0; hh < treeHeight; hh++)
	{
		int t = level->getTile(x, y + hh, z);
		if (t == 0 || t == Tile::leaves_Id) placeBlock(level, x, y + hh, z, Tile::treeTrunk_Id, TreeTile::BIRCH_TRUNK);
	}
	return true;

}