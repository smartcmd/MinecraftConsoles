#include "stdafx.h"
#include "SandFeature.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"

SandFeature::SandFeature(int radius, int tile)
{
	this->tile = tile;
	this->radius = radius;
}

bool SandFeature::place(Level *level, Random *random, int x, int y, int z)
{
	if (level->getMaterial(x, y, z) != Material::water) return false;

	// 4J - optimisation. Without this, we can end up creating a huge number of HeavyTiles to be ticked
	// a few frames away. I think instatick ought to be fine here - we're only turning rock into gravel,
	// so should instantly know if we've made a rock with nothing underneath and that should fall.
	level->setInstaTick(true);

	int r = random->nextInt(radius-2)+2;
	int yr = 2;

	// 4J Stu Added to stop tree features generating areas previously place by game rule generation
	if(app.getLevelGenerationOptions() != NULL)
	{
		LevelGenerationOptions *levelGenOptions = app.getLevelGenerationOptions();
		bool intersects = levelGenOptions->checkIntersects(x - r, y - yr, z - r, x + r, y + yr, z + r);
		if(intersects)
		{
			level->setInstaTick(false);
			//app.DebugPrintf("Skipping reeds feature generation as it overlaps a game rule structure\n");
			return false;
		}
	}

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
				if (t == Tile::dirt_Id || t == Tile::grass_Id)
				{
					level->setTileAndData(xx, yy, zz, tile, 0, Tile::UPDATE_CLIENTS);
				}
			}
		}
	}

	level->setInstaTick(false);

	return true;
}