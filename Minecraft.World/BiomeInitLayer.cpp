#include "stdafx.h"
#include "net.minecraft.world.level.biome.h"
#include "net.minecraft.world.level.newbiome.layer.h"
#include "net.minecraft.world.level.h"
#include "BiomeInitLayer.h"

BiomeInitLayer::BiomeInitLayer(__int64 seed, shared_ptr<Layer>parent, LevelType *levelType) : Layer(seed)
{
	this->parent = parent;


	if(levelType == LevelType::lvl_normal_1_1)
	{
		startBiomes = BiomeArray(6);
		startBiomes[0] = Biome::desert;
		startBiomes[1] = Biome::forest;
		startBiomes[2] = Biome::extremeHills;
		startBiomes[3] = Biome::swampland;
		startBiomes[4] = Biome::plains;
		startBiomes[5] = Biome::taiga;
	}
	else
	{
		startBiomes = BiomeArray(7);
		startBiomes[0] = Biome::desert;
		startBiomes[1] = Biome::forest;
		startBiomes[2] = Biome::extremeHills;
		startBiomes[3] = Biome::swampland;
		startBiomes[4] = Biome::plains;
		startBiomes[5] = Biome::taiga;
		startBiomes[6] = Biome::jungle;
	}
}

BiomeInitLayer::~BiomeInitLayer()
{
	delete [] startBiomes.data;
}

intArray BiomeInitLayer::getArea(int xo, int yo, int w, int h)
{
	intArray b = parent->getArea(xo, yo, w, h);

	intArray result = IntCache::allocate(w * h);
	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			initRandom(x + xo, y + yo);
            int old = b[x + y * w];
            if (old == 0)
			{
                result[x + y * w] = 0;
            }
			else if (old == Biome::mushroomIsland->id)
			{
                result[x + y * w] = old;
            }
			else if (old == 1)
			{
                result[x + y * w] = startBiomes[nextRandom(startBiomes.length)]->id;
            }
			else
			{
				int isTaiga = startBiomes[nextRandom(startBiomes.length)]->id;
				if (isTaiga == Biome::taiga->id)
				{
					result[x + y * w] = isTaiga;
				}
				else
				{
					result[x + y * w] = Biome::iceFlats->id;
				}
			}
		}
	}

	return result;
}