#include "stdafx.h"
#include "net.minecraft.world.level.tile.h"
#include "BiomeDecorator.h"
#include "BeachBiome.h"

BeachBiome::BeachBiome(int id) : Biome(id)
{
	// remove default mob spawn settings
	friendlies.clear();
	friendlies_chicken.clear();	// 4J added
	topMaterial = (byte) Tile::sand_Id;
	material = (byte) Tile::sand_Id;

	decorator->treeCount = -999;
	decorator->deadBushCount = 0;
	decorator->reedsCount = 0;
	decorator->cactusCount = 0;
}