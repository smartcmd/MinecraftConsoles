#include "stdafx.h"
#include "net.minecraft.world.level.levelgen.feature.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.biome.h"

DesertBiome::DesertBiome(int id) : Biome(id)
{
	// remove default mob spawn settings
	friendlies.clear();
	friendlies_chicken.clear();	// 4J added
	friendlies_wolf.clear(); 	// 4J added
	topMaterial = static_cast<BYTE>(Tile::sand_Id);
	material = static_cast<BYTE>(Tile::sand_Id);

	decorator->treeCount = -999;
	decorator->deadBushCount = 2;
	decorator->reedsCount = 50;
	decorator->cactusCount = 10;
}

void DesertBiome::decorate(Level *level, Random *random, int xo, int zo)
{
	Biome::decorate(level, random, xo, zo);

	if (random->nextInt(1000) == 0)
	{
		int x = xo + random->nextInt(16) + 8;
		int z = zo + random->nextInt(16) + 8;
		Feature *well = new DesertWellFeature();
		well->place(level, random, x, level->getHeightmap(x, z) + 1, z);
	}
}