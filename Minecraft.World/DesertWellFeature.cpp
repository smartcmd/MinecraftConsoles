#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "DesertWellFeature.h"

bool DesertWellFeature::place(Level *level, Random *random, int x, int y, int z)
{
	while (level->isEmptyTile(x, y, z) && y > 2)
		y--;

	int tile = level->getTile(x, y, z);
	if (tile != Tile::sand_Id)
	{
		return false;
	}

	// the surrounding 5x5 area may not be lower than y-1
	for (int ox = -2; ox <= 2; ox++)
	{
		for (int oz = -2; oz <= 2; oz++)
		{
			if (level->isEmptyTile(x + ox, y - 1, z + oz) && level->isEmptyTile(x + ox, y - 2, z + oz))
			{
				return false;
			}
		}
	}

	// place floor
	for (int oy = -1; oy <= 0; oy++)
	{
		for (int ox = -2; ox <= 2; ox++)
		{
			for (int oz = -2; oz <= 2; oz++)
			{
				level->setTileAndData(x + ox, y + oy, z + oz, Tile::sandStone_Id, 0, Tile::UPDATE_CLIENTS);
			}
		}
	}

	// place water cross
	level->setTileAndData(x, y, z, Tile::water_Id, 0, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x - 1, y, z, Tile::water_Id, 0, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x + 1, y, z, Tile::water_Id, 0, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x, y, z - 1, Tile::water_Id, 0, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x, y, z + 1, Tile::water_Id, 0, Tile::UPDATE_CLIENTS);

	// place "fence"
	for (int ox = -2; ox <= 2; ox++)
	{
		for (int oz = -2; oz <= 2; oz++)
		{
			if (ox == -2 || ox == 2 || oz == -2 || oz == 2)
			{
				level->setTileAndData(x + ox, y + 1, z + oz, Tile::sandStone_Id, 0, Tile::UPDATE_CLIENTS);
			}
		}
	}
	level->setTileAndData(x + 2, y + 1, z, Tile::stoneSlabHalf_Id, StoneSlabTile::SAND_SLAB, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x - 2, y + 1, z, Tile::stoneSlabHalf_Id, StoneSlabTile::SAND_SLAB, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x, y + 1, z + 2, Tile::stoneSlabHalf_Id, StoneSlabTile::SAND_SLAB, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x, y + 1, z - 2, Tile::stoneSlabHalf_Id, StoneSlabTile::SAND_SLAB, Tile::UPDATE_CLIENTS);

	// place roof
	for (int ox = -1; ox <= 1; ox++)
	{
		for (int oz = -1; oz <= 1; oz++)
		{
			if (ox == 0 && oz == 0)
			{
				level->setTileAndData(x + ox, y + 4, z + oz, Tile::sandStone_Id, 0, Tile::UPDATE_CLIENTS);
			}
			else
			{
				level->setTileAndData(x + ox, y + 4, z + oz, Tile::stoneSlabHalf_Id, StoneSlabTile::SAND_SLAB, Tile::UPDATE_CLIENTS);
			}
		}
	}

	// place pillars
	for (int oy = 1; oy <= 3; oy++)
	{
		level->setTileAndData(x - 1, y + oy, z - 1, Tile::sandStone_Id, 0, Tile::UPDATE_CLIENTS);
		level->setTileAndData(x - 1, y + oy, z + 1, Tile::sandStone_Id, 0, Tile::UPDATE_CLIENTS);
		level->setTileAndData(x + 1, y + oy, z - 1, Tile::sandStone_Id, 0, Tile::UPDATE_CLIENTS);
		level->setTileAndData(x + 1, y + oy, z + 1, Tile::sandStone_Id, 0, Tile::UPDATE_CLIENTS);
	}

	return true;
}