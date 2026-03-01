#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.biome.h"
#include "SwampTreeFeature.h"

SwampBiome::SwampBiome(int id) : Biome(id)
{
	decorator->treeCount = 2;
	decorator->flowerCount = -999;
	decorator->deadBushCount = 1;
	decorator->mushroomCount = 8;
	decorator->reedsCount = 10;
	decorator->clayCount = 1;
	decorator->waterlilyCount = 4;
	
	// waterColor = 0xe0ffae;

	enemies.push_back(new MobSpawnerData(eTYPE_SLIME, 1, 1, 1));
}


Feature *SwampBiome::getTreeFeature(Random *random)
{
	return new SwampTreeFeature(); // 4J used to return member swampTree, now returning newly created object so that caller can be consistently resposible for cleanup
}

// 4J Stu - Not using these any more
//int SwampBiome::getGrassColor()
//{
//    double temp = getTemperature();
//    double rain = getDownfall();
//
//    return ((GrassColor::get(temp, rain) & 0xfefefe) + 0x4e0e4e) / 2;
//}
//
//int SwampBiome::getFolageColor()
//{
//    double temp = getTemperature();
//    double rain = getDownfall();
//
//    return ((FoliageColor::get(temp, rain) & 0xfefefe) + 0x4e0e4e) / 2;
//}