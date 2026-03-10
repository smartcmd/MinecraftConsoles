#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "PineFeature.h"

bool PineFeature::place(Level *level, Random *random, int x, int y, int z)
{
    // pines can be quite tall
    int treeHeight = random->nextInt(5) + 7;
    int trunkHeight = treeHeight - random->nextInt(2) - 3;
    int topHeight = treeHeight - trunkHeight;
    int topRadius = 1 + random->nextInt(topHeight + 1);

    bool free = true;
    // may not be outside of y boundaries
    if (y < 1 || y + treeHeight + 1 > Level::genDepth)
	{
        return false;
    }

	// 4J Stu Added to stop tree features generating areas previously place by game rule generation
	if(app.getLevelGenerationOptions() != nullptr)
	{
		LevelGenerationOptions *levelGenOptions = app.getLevelGenerationOptions();
		bool intersects = levelGenOptions->checkIntersects(x - topRadius, y - 1, z - topRadius, x + topRadius, y + treeHeight, z + topRadius);
		if(intersects)
		{
			//app.DebugPrintf("Skipping reeds feature generation as it overlaps a game rule structure\n");
			return false;
		}
	}

    // make sure there is enough space
    for (int yy = y; yy <= y + 1 + treeHeight && free; yy++)
	{

        int r = 1;
        if ((yy - y) < trunkHeight)
		{
            r = 0;
        }
		else
		{
            r = topRadius;
        }
        for (int xx = x - r; xx <= x + r && free; xx++)
		{
            for (int zz = z - r; zz <= z + r && free; zz++)
			{
                if (yy >= 0 && yy < Level::genDepth)
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

    // must stand on ground
    int belowTile = level->getTile(x, y - 1, z);
    if ((belowTile != Tile::grass_Id && belowTile != Tile::dirt_Id) || y >= Level::genDepth - treeHeight - 1) return false;

    placeBlock(level, x, y - 1, z, Tile::dirt_Id);

    // place leaf top
    int currentRadius = 0;
    for (int yy = y + treeHeight; yy >= y + trunkHeight; yy--)
	{
        for (int xx = x - currentRadius; xx <= x + currentRadius; xx++)
		{
            int xo = xx - (x);
            for (int zz = z - currentRadius; zz <= z + currentRadius; zz++)
			{
                int zo = zz - (z);
                if (abs(xo) == currentRadius && abs(zo) == currentRadius && currentRadius > 0) continue;
                if (!Tile::solid[level->getTile(xx, yy, zz)]) placeBlock(level, xx, yy, zz, Tile::leaves_Id, LeafTile::EVERGREEN_LEAF);
            }
        }

        if (currentRadius >= 1 && yy == (y + trunkHeight + 1))
		{
            currentRadius -= 1;
        }
		else if (currentRadius < topRadius)
		{
            currentRadius += 1;
        }
    }
    for (int hh = 0; hh < treeHeight - 1; hh++)
	{
        int t = level->getTile(x, y + hh, z);
        if (t == 0 || t == Tile::leaves_Id) placeBlock(level, x, y + hh, z, Tile::treeTrunk_Id, TreeTile::DARK_TRUNK);
    }

    return true;
}