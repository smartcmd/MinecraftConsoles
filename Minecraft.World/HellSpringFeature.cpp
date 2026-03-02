#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "HellSpringFeature.h"
#include "net.minecraft.world.level.tile.h"

HellSpringFeature::HellSpringFeature(int tile, bool insideRock)
{
	this->tile = tile;
	this->insideRock = insideRock;
}

bool HellSpringFeature::place(Level *level, Random *random, int x, int y, int z)
{
	if (level->getTile(x, y + 1, z) != Tile::netherRack_Id) return false;
	if (level->getTile(x, y - 1, z) != Tile::netherRack_Id) return false;

	if (level->getTile(x, y, z) != 0 && level->getTile(x, y, z) != Tile::netherRack_Id) return false;

	int rockCount = 0;
	if (level->getTile(x - 1, y, z) == Tile::netherRack_Id) rockCount++;
	if (level->getTile(x + 1, y, z) == Tile::netherRack_Id) rockCount++;
	if (level->getTile(x, y, z - 1) == Tile::netherRack_Id) rockCount++;
	if (level->getTile(x, y, z + 1) == Tile::netherRack_Id) rockCount++;
	if (level->getTile(x, y - 1, z) == Tile::netherRack_Id) rockCount++;

	int holeCount = 0;
	if (level->isEmptyTile(x - 1, y, z)) holeCount++;
	if (level->isEmptyTile(x + 1, y, z)) holeCount++;
	if (level->isEmptyTile(x, y, z - 1)) holeCount++;
	if (level->isEmptyTile(x, y, z + 1)) holeCount++;
	if (level->isEmptyTile(x, y - 1, z)) holeCount++;

	if ((!insideRock && rockCount == 4 && holeCount == 1) || rockCount == 5)
	{
		level->setTileAndData(x, y, z, tile, 0, Tile::UPDATE_CLIENTS);
		level->setInstaTick(true);
		Tile::tiles[tile]->tick(level, x, y, z, random);
		level->setInstaTick(false);
	}

	return true;

}