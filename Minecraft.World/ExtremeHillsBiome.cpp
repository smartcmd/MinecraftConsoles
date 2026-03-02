#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.levelgen.feature.h"
#include "ExtremeHillsBiome.h"

ExtremeHillsBiome::ExtremeHillsBiome(int id) : Biome(id)
{
	silverfishFeature = new OreFeature(Tile::monsterStoneEgg_Id, 8);
	friendlies.clear();
}

ExtremeHillsBiome::~ExtremeHillsBiome()
{
	delete silverfishFeature;
}

void ExtremeHillsBiome::decorate(Level *level, Random *random, int xo, int zo) {
	Biome::decorate(level, random, xo, zo);

	if (GENERATE_EMERALD_ORE)
	{
		int emeraldCount = 3 + random->nextInt(6);
		for (int d = 0; d < emeraldCount; d++)
		{
			int x = xo + random->nextInt(16);
			int y = random->nextInt((Level::genDepth / 4) - 4) + 4;
			int z = zo + random->nextInt(16);
			int tile = level->getTile(x, y, z);
			if (tile == Tile::stone_Id)
			{
				level->setTileAndData(x, y, z, Tile::emeraldOre_Id, 0, Tile::UPDATE_CLIENTS);
			}
		}
	}

	for (int i = 0; i < 7; i++)
	{
		int x = xo + random->nextInt(16);
		int y = random->nextInt(Level::genDepth / 2);
		int z = zo + random->nextInt(16);
		silverfishFeature->place(level, random, x, y, z);
	}
}
