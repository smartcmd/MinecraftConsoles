#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "MegaTreeFeature.h"

MegaTreeFeature::MegaTreeFeature(bool doUpdate, int baseHeight, int trunkType, int leafType) : Feature(doUpdate), baseHeight(baseHeight), trunkType(trunkType), leafType(leafType)
{
}

bool MegaTreeFeature::place(Level *level, Random *random, int x, int y, int z)
{
	int treeHeight = random->nextInt(3) + baseHeight;

	bool free = true;
	if (y < 1 || y + treeHeight + 1 > Level::maxBuildHeight) return false;

	// 4J Stu Added to stop tree features generating areas previously place by game rule generation
	if(app.getLevelGenerationOptions() != nullptr)
	{
		PIXBeginNamedEvent(0, "MegaTreeFeature Checking intersects");
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
		int r = 2;
		if (yy == y) r = 1;
		if (yy >= y + 1 + treeHeight - 2) r = 2;
		for (int xx = x - r; xx <= x + r && free; xx++)
		{
			for (int zz = z - r; zz <= z + r && free; zz++)
			{
				if (yy >= 0 && yy < Level::maxBuildHeight)
				{
					int tt = level->getTile(xx, yy, zz);
					if (tt != 0 && tt != Tile::leaves_Id && tt != Tile::grass_Id && tt != Tile::dirt_Id && tt != Tile::treeTrunk_Id && tt != Tile::sapling_Id) free = false;
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

	level->setTileAndData(x, y - 1, z, Tile::dirt_Id, 0, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x + 1, y - 1, z, Tile::dirt_Id, 0, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x, y - 1, z + 1, Tile::dirt_Id, 0, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x + 1, y - 1, z + 1, Tile::dirt_Id, 0, Tile::UPDATE_CLIENTS);

	PIXBeginNamedEvent(0,"MegaTree placing leaves, %d, %d, %d", x, z, y+treeHeight);
	placeLeaves(level, x, z, y + treeHeight, 2, random);
	PIXEndNamedEvent();

	PIXBeginNamedEvent(0,"MegaTree placing branches");
	int branchHeight = y + treeHeight - 2 - random->nextInt(4);
	while (branchHeight > y + treeHeight / 2)
	{
		float angle = random->nextFloat() * PI * 2.0f;
		int bx = x + static_cast<int>(0.5f + Mth::cos(angle) * 4.0f);
		int bz = z + static_cast<int>(0.5f + Mth::sin(angle) * 4.0f);
		placeLeaves(level, bx, bz, branchHeight, 0, random);

		for (int b = 0; b < 5; b++)
		{
			bx = x + static_cast<int>(1.5f + Mth::cos(angle) * b);
			bz = z + static_cast<int>(1.5f + Mth::sin(angle) * b);
			placeBlock(level, bx, branchHeight - 3 + b / 2, bz, Tile::treeTrunk_Id, trunkType);
		}

		branchHeight -= 2 + random->nextInt(4);
	}
	PIXEndNamedEvent();

	PIXBeginNamedEvent(0, "MegaTree placing vines");
	for (int hh = 0; hh < treeHeight; hh++)
	{
		int t = level->getTile(x, y + hh, z);
		if (t == 0 || t == Tile::leaves_Id)
		{
			placeBlock(level, x, y + hh, z, Tile::treeTrunk_Id, trunkType);
			if (hh > 0)
			{
				if (random->nextInt(3) > 0 && level->isEmptyTile(x - 1, y + hh, z))
				{
					placeBlock(level, x - 1, y + hh, z, Tile::vine_Id, VineTile::VINE_EAST);
				}
				if (random->nextInt(3) > 0 && level->isEmptyTile(x, y + hh, z - 1))
				{
					placeBlock(level, x, y + hh, z - 1, Tile::vine_Id, VineTile::VINE_SOUTH);
				}
			}
		}
		if (hh < (treeHeight - 1))
		{
			t = level->getTile(x + 1, y + hh, z);
			if (t == 0 || t == Tile::leaves_Id)
			{
				placeBlock(level, x + 1, y + hh, z, Tile::treeTrunk_Id, trunkType);
				if (hh > 0)
				{
					if (random->nextInt(3) > 0 && level->isEmptyTile(x + 2, y + hh, z))
					{
						placeBlock(level, x + 2, y + hh, z, Tile::vine_Id, VineTile::VINE_WEST);
					}
					if (random->nextInt(3) > 0 && level->isEmptyTile(x + 1, y + hh, z - 1))
					{
						placeBlock(level, x + 1, y + hh, z - 1, Tile::vine_Id, VineTile::VINE_SOUTH);
					}
				}
			}
			t = level->getTile(x + 1, y + hh, z + 1);
			if (t == 0 || t == Tile::leaves_Id)
			{
				placeBlock(level, x + 1, y + hh, z + 1, Tile::treeTrunk_Id, trunkType);
				if (hh > 0)
				{
					if (random->nextInt(3) > 0 && level->isEmptyTile(x + 2, y + hh, z + 1))
					{
						placeBlock(level, x + 2, y + hh, z + 1, Tile::vine_Id, VineTile::VINE_WEST);
					}
					if (random->nextInt(3) > 0 && level->isEmptyTile(x + 1, y + hh, z + 2))
					{
						placeBlock(level, x + 1, y + hh, z + 2, Tile::vine_Id, VineTile::VINE_NORTH);
					}
				}
			}
			t = level->getTile(x, y + hh, z + 1);
			if (t == 0 || t == Tile::leaves_Id)
			{
				placeBlock(level, x, y + hh, z + 1, Tile::treeTrunk_Id, trunkType);
				if (hh > 0)
				{
					if (random->nextInt(3) > 0 && level->isEmptyTile(x - 1, y + hh, z + 1))
					{
						placeBlock(level, x - 1, y + hh, z + 1, Tile::vine_Id, VineTile::VINE_EAST);
					}
					if (random->nextInt(3) > 0 && level->isEmptyTile(x, y + hh, z + 2))
					{
						placeBlock(level, x, y + hh, z + 2, Tile::vine_Id, VineTile::VINE_NORTH);
					}
				}
			}
		}
	}
	PIXEndNamedEvent();

	return true;
}

void MegaTreeFeature::placeLeaves(Level *level, int x, int z, int topPosition, int baseRadius, Random *random)
{
	int grassHeight = 2;
	// 4J Stu - Generate from top down so that we don't have to keep adjusting the heightmaps
	for (int yy = topPosition; yy >= topPosition - grassHeight; yy--)
	{
		int yo = yy - (topPosition);
		int radius = baseRadius + 1 - yo;
		for (int xx = x - radius; xx <= x + radius + 1; xx++)
		{
			int xo = xx - (x);
			for (int zz = z - radius; zz <= z + radius + 1; zz++)
			{
				int zo = zz - (z);
				if ((xo < 0 && zo < 0) && (xo * xo + zo * zo) > (radius * radius))
				{
					continue;
				}
				if ((xo > 0 || zo > 0) && (xo * xo + zo * zo) > ((radius + 1) * (radius + 1)))
				{
					continue;
				}
				if (random->nextInt(4) == 0 && (xo * xo + zo * zo) > ((radius - 1) * (radius - 1)))
				{
					continue;
				}
				PIXBeginNamedEvent(0,"Getting tile");
				int t = level->getTile(xx, yy, zz);
				PIXEndNamedEvent();
				if (t == 0 || t == Tile::leaves_Id)
				{
					PIXBeginNamedEvent(0,"Placing block");
					placeBlock(level, xx, yy, zz, Tile::leaves_Id, leafType);
					PIXEndNamedEvent();
				}
			}
		}
	}
}