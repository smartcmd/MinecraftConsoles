#include "stdafx.h"
#include "net.minecraft.world.level.biome.h"
#include "net.minecraft.world.level.newbiome.layer.h"

RiverMixerLayer::RiverMixerLayer(__int64 seed, shared_ptr<Layer>biomes, shared_ptr<Layer>rivers) : Layer(seed)
{
	this->biomes = biomes;
	this->rivers = rivers;
}

void RiverMixerLayer::init(__int64 seed)
{
	biomes->init(seed);
	rivers->init(seed);
	Layer::init(seed);
}

intArray RiverMixerLayer::getArea(int xo, int yo, int w, int h)
{
	intArray b = biomes->getArea(xo, yo, w, h);
	intArray r = rivers->getArea(xo, yo, w, h);

	intArray result = IntCache::allocate(w * h);
	for (int i = 0; i < w * h; i++)
	{
		if (b[i] == Biome::ocean->id)
		{
			result[i] = b[i];

		}
		else
		{
			if (r[i] >= 0)
			{
				if (b[i] == Biome::iceFlats->id) result[i] = Biome::frozenRiver->id;
				else if (b[i] == Biome::mushroomIsland->id || b[i] == Biome::mushroomIslandShore->id) result[i] = Biome::mushroomIsland->id;		// 4J - don't make mushroom island shores as we don't have any island left once we do this as our islands are small (this used to change to mushroomIslandShore)
				else result[i] = r[i];
			}
			else
			{
				result[i] = b[i];
			}
		}
	}

	return result;
}