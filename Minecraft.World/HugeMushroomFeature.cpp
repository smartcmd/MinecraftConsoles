#include "stdafx.h"
#include "HugeMushroomFeature.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"

HugeMushroomFeature::HugeMushroomFeature(int forcedType) : Feature(true)
{
	this->forcedType = forcedType;
}

HugeMushroomFeature::HugeMushroomFeature() : Feature(false)
{
	this->forcedType = -1;
}

bool HugeMushroomFeature::place(Level *level, Random *random, int x, int y, int z)
{
	int type = random->nextInt(2);
	if (forcedType >= 0) type = forcedType;

	int treeHeight = random->nextInt(3) + 4;

	bool free = true;
	if (y < 1 || y + treeHeight + 1 >= Level::maxBuildHeight) return false;

	for (int yy = y; yy <= y + 1 + treeHeight; yy++)
	{
		int r = 3;
		if (yy <= (y + 3) ) r = 0;
		for (int xx = x - r; xx <= x + r && free; xx++)
		{
			for (int zz = z - r; zz <= z + r && free; zz++)
			{
				if (yy >= 0 && yy < Level::maxBuildHeight)
				{
					int tt = level->getTile(xx, yy, zz);
					if (tt != 0 && tt != Tile::leaves_Id)
					{
						free = false;
					}
				}
				else
				{
					free = false;
				}
			}
		}
	}

	int belowTile = level->getTile(x, y - 1, z);
	if (belowTile != Tile::dirt_Id && belowTile != Tile::grass_Id && belowTile != Tile::mycel_Id)
	{
		return false;
	}

	if (!free) return false;

	int low = y + treeHeight;
	if (type == 1) {
		low = y + treeHeight - 3;
	}
	for (int yy = low; yy <= y + treeHeight; yy++)
	{
		int offs = 1;
		if (yy < y + treeHeight) offs += 1;
		if (type == 0) offs = 3;
		for (int xx = x - offs; xx <= x + offs; xx++)
		{
			for (int zz = z - offs; zz <= z + offs; zz++)
			{
				int data = 5;
				if (xx == x - offs) data--;
				if (xx == x + offs) data++;
				if (zz == z - offs) data -= 3;
				if (zz == z + offs) data += 3;

				if (type == 0 || yy < y + treeHeight)
				{
					if ((xx == x - offs || xx == x + offs) && (zz == z - offs || zz == z + offs)) continue;
					if (xx == x - (offs - 1) && zz == z - offs) data = 1;
					if (xx == x - offs && zz == z - (offs - 1)) data = 1;

					if (xx == x + (offs - 1) && zz == z - offs) data = 3;
					if (xx == x + offs && zz == z - (offs - 1)) data = 3;

					if (xx == x - (offs - 1) && zz == z + offs) data = 7;
					if (xx == x - offs && zz == z + (offs - 1)) data = 7;

					if (xx == x + (offs - 1) && zz == z + offs) data = 9;
					if (xx == x + offs && zz == z + (offs - 1)) data = 9;
				}

				if (data == 5 && yy < y + treeHeight) data = 0;
				if (data != 0 || y >= y + treeHeight - 1)
				{
					if (!Tile::solid[level->getTile(xx, yy, zz)]) placeBlock(level, xx, yy, zz, Tile::hugeMushroom_brown_Id + type, data);
				}
			}
		}
	}
	for (int hh = 0; hh < treeHeight; hh++)
	{
		int t = level->getTile(x, y + hh, z);
		if (!Tile::solid[t]) placeBlock(level, x, y + hh, z, Tile::hugeMushroom_brown_Id + type, 10);
	}
	return true;
}
