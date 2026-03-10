#include "stdafx.h"
#include "MushroomIslandBiome.h"
#include "BiomeDecorator.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.level.tile.h"

MushroomIslandBiome::MushroomIslandBiome(int id) : Biome(id)
{
    decorator->treeCount = -100;
    decorator->flowerCount = -100;
    decorator->grassCount = -100;

    decorator->mushroomCount = 1;
    decorator->hugeMushrooms = 1;

    topMaterial = static_cast<byte>(Tile::mycel_Id);

    enemies.clear();
    friendlies.clear();
	friendlies_chicken.clear();	// 4J added
	friendlies_wolf.clear(); 	// 4J added
    waterFriendlies.clear();

    friendlies_mushroomcow.push_back(new MobSpawnerData(eTYPE_MUSHROOMCOW, 8, 4, 8));		// 4J moved to own category
}