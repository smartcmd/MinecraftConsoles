#include "stdafx.h"
#include "net.minecraft.world.level.biome.h"

PlainsBiome::PlainsBiome(int id) : Biome(id)
{
	friendlies.push_back(new MobSpawnerData(eTYPE_HORSE, 5, 2, 6));

	decorator->treeCount = -999;
	decorator->flowerCount = 4;
	decorator->grassCount = 10;
}