#include "stdafx.h"
#include "net.minecraft.world.entity.monster.h"
#include "HellBiome.h"

HellBiome::HellBiome(int id) : Biome(id)
{
	enemies.clear();
	friendlies.clear();
	friendlies_chicken.clear();	// 4J added
	friendlies_wolf.clear(); 	// 4J added
	waterFriendlies.clear();
	ambientFriendlies.clear();

	enemies.push_back(new MobSpawnerData(eTYPE_GHAST, 50, 4, 4));
	enemies.push_back(new MobSpawnerData(eTYPE_PIGZOMBIE, 100, 4, 4));
	enemies.push_back(new MobSpawnerData(eTYPE_LAVASLIME, 1, 4, 4));
}