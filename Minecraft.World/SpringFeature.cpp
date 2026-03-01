#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "SpringFeature.h"

SpringFeature::SpringFeature(int tile)
{
	this->tile = tile;
}

bool SpringFeature::place(Level *level, Random *random, int x, int y, int z)
{
	// 4J Stu Added to stop spring features generating areas previously place by game rule generation
	if(app.getLevelGenerationOptions() != NULL)
	{
		LevelGenerationOptions *levelGenOptions = app.getLevelGenerationOptions();
		bool intersects = levelGenOptions->checkIntersects(x, y, z, x, y, z);
		if(intersects)
		{
			//app.DebugPrintf("Skipping spring feature generation as it overlaps a game rule structure\n");
			return false;
		}
	}

	if (level->getTile(x, y + 1, z) != Tile::stone_Id) return false;
	if (level->getTile(x, y - 1, z) != Tile::stone_Id) return false;

	if (level->getTile(x, y, z) != 0 && level->getTile(x, y, z) != Tile::stone_Id) return false;

	int rockCount = 0;
	if (level->getTile(x - 1, y, z) == Tile::stone_Id) rockCount++;
	if (level->getTile(x + 1, y, z) == Tile::stone_Id) rockCount++;
	if (level->getTile(x, y, z - 1) == Tile::stone_Id) rockCount++;
	if (level->getTile(x, y, z + 1) == Tile::stone_Id) rockCount++;

	int holeCount = 0;
	if (level->isEmptyTile(x - 1, y, z)) holeCount++;
	if (level->isEmptyTile(x + 1, y, z)) holeCount++;
	if (level->isEmptyTile(x, y, z - 1)) holeCount++;
	if (level->isEmptyTile(x, y, z + 1)) holeCount++;

	if (rockCount == 3 && holeCount == 1)
	{
		level->setTileAndData(x, y, z, tile, 0, Tile::UPDATE_CLIENTS);
		level->setInstaTick(true);
		Tile::tiles[tile]->tick(level, x, y, z, random);
		level->setInstaTick(false);
	}

	return true;
}