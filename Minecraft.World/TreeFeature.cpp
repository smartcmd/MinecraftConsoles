#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "TreeFeature.h"

TreeFeature::TreeFeature(bool doUpdate) : Feature(doUpdate), baseHeight(4), trunkType(0), leafType(0), addJungleFeatures(false)
{
}

TreeFeature::TreeFeature(bool doUpdate, int baseHeight, int trunkType, int leafType, bool addJungleFeatures) : Feature(doUpdate), baseHeight(baseHeight), trunkType(trunkType), leafType(leafType), addJungleFeatures(addJungleFeatures)
{
}

bool TreeFeature::place(Level *level, Random *random, int x, int y, int z)
{	
	int treeHeight = random->nextInt(3) + baseHeight;

	bool free = true;
	if (y < 1 || y + treeHeight + 1 > Level::maxBuildHeight) return false;

	// 4J Stu Added to stop tree features generating areas previously place by game rule generation
	if(app.getLevelGenerationOptions() != NULL)
	{
		PIXBeginNamedEvent(0,"TreeFeature checking intersects");
		LevelGenerationOptions *levelGenOptions = app.getLevelGenerationOptions();
		bool intersects = levelGenOptions->checkIntersects(x - 2, y - 1, z - 2, x + 2, y + treeHeight, z + 2);
		PIXEndNamedEvent();
		if(intersects)
		{
			//app.DebugPrintf("Skipping reeds feature generation as it overlaps a game rule structure\n");
			return false;
		}
	}

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
					if (tt != 0 && tt != Tile::leaves_Id && tt != Tile::grass_Id && tt != Tile::dirt_Id && tt != Tile::treeTrunk_Id) free = false;
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

	placeBlock(level, x, y - 1, z, Tile::dirt_Id, 0);
	
	PIXBeginNamedEvent(0,"Placing TreeFeature leaves");
	int grassHeight = 3;
	int extraWidth = 0;
	// 4J Stu - Generate leaves from the top down to stop having to recalc heightmaps
	for (int yy = y + treeHeight; yy >= y - grassHeight + treeHeight; yy--)
	{
		int yo = yy - (y + treeHeight);
		int offs = extraWidth + 1 - yo / 2;
		for (int xx = x - offs; xx <= x + offs; xx++)
		{
			int xo = xx - (x);
			for (int zz = z - offs; zz <= z + offs; zz++)
			{
				int zo = zz - (z);
				if (abs(xo) == offs && abs(zo) == offs && (random->nextInt(2) == 0 || yo == 0)) continue;
				int t = level->getTile(xx, yy, zz);
				if (t == 0 || t == Tile::leaves_Id) placeBlock(level, xx, yy, zz, Tile::leaves_Id, leafType);
			}
		}
	}
	PIXEndNamedEvent();
	PIXBeginNamedEvent(0,"Placing TreeFeature trunks");
	for (int hh = 0; hh < treeHeight; hh++)
	{
		int t = level->getTile(x, y + hh, z);
		if (t == 0 || t == Tile::leaves_Id)
		{
			placeBlock(level, x, y + hh, z, Tile::treeTrunk_Id, trunkType);
			if (addJungleFeatures && hh > 0)
			{
				if (random->nextInt(3) > 0 && level->isEmptyTile(x - 1, y + hh, z))
				{
					placeBlock(level, x - 1, y + hh, z, Tile::vine_Id, VineTile::VINE_EAST);
				}
				if (random->nextInt(3) > 0 && level->isEmptyTile(x + 1, y + hh, z))
				{
					placeBlock(level, x + 1, y + hh, z, Tile::vine_Id, VineTile::VINE_WEST);
				}
				if (random->nextInt(3) > 0 && level->isEmptyTile(x, y + hh, z - 1))
				{
					placeBlock(level, x, y + hh, z - 1, Tile::vine_Id, VineTile::VINE_SOUTH);
				}
				if (random->nextInt(3) > 0 && level->isEmptyTile(x, y + hh, z + 1))
				{
					placeBlock(level, x, y + hh, z + 1, Tile::vine_Id, VineTile::VINE_NORTH);
				}
			}
		}
	}
	PIXEndNamedEvent();
	if (addJungleFeatures)
	{
		PIXBeginNamedEvent(0,"TreeFeature adding vines");
		for (int yy = y - 3 + treeHeight; yy <= y + treeHeight; yy++)
		{
			int yo = yy - (y + treeHeight);
			int offs = 2 - yo / 2;
			for (int xx = x - offs; xx <= x + offs; xx++)
			{
				for (int zz = z - offs; zz <= z + offs; zz++)
				{
					if (level->getTile(xx, yy, zz) == Tile::leaves_Id)
					{
						if (random->nextInt(4) == 0 && level->getTile(xx - 1, yy, zz) == 0)
						{
							addVine(level, xx - 1, yy, zz, VineTile::VINE_EAST);
						}
						if (random->nextInt(4) == 0 && level->getTile(xx + 1, yy, zz) == 0)
						{
							addVine(level, xx + 1, yy, zz, VineTile::VINE_WEST);
						}
						if (random->nextInt(4) == 0 && level->getTile(xx, yy, zz - 1) == 0)
						{
							addVine(level, xx, yy, zz - 1, VineTile::VINE_SOUTH);
						}
						if (random->nextInt(4) == 0 && level->getTile(xx, yy, zz + 1) == 0)
						{
							addVine(level, xx, yy, zz + 1, VineTile::VINE_NORTH);
						}
					}
				}
			}
		}
		PIXEndNamedEvent();
		// also chance for cocoa plants around stem
		if (random->nextInt(5) == 0 && treeHeight > 5)
		{
		PIXBeginNamedEvent(0,"TreeFeature adding cocoa");
			for (int rows = 0; rows < 2; rows++)
			{
				for (int dir = 0; dir < 4; dir++)
				{
					if (random->nextInt(4 - rows) == 0)
					{
						int age = random->nextInt(3);
						placeBlock(level, x + Direction::STEP_X[Direction::DIRECTION_OPPOSITE[dir]], y + treeHeight - 5 + rows, z + Direction::STEP_Z[Direction::DIRECTION_OPPOSITE[dir]],
							Tile::cocoa_Id, (age << 2) | dir);
					}
				}
			}
			PIXEndNamedEvent();
		}
	}

	return true;
}

void TreeFeature::addVine(Level *level, int xx, int yy, int zz, int dir)
{
	placeBlock(level, xx, yy, zz, Tile::vine_Id, dir);
	int maxDir = 4;
	while (level->getTile(xx, --yy, zz) == 0 && maxDir > 0)
	{
		placeBlock(level, xx, yy, zz, Tile::vine_Id, dir);
		maxDir--;
	}
}