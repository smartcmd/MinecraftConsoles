#include "stdafx.h"
#include "net.minecraft.world.level.biome.h"
#include "IntCache.h"
#include "SwampRiversLayer.h"

SwampRiversLayer::SwampRiversLayer(int64_t seed, shared_ptr<Layer> parent) : Layer(seed)
{
	this->parent = parent;
}

intArray SwampRiversLayer::getArea(int xo, int yo, int w, int h)
{
	intArray b = parent->getArea(xo - 1, yo - 1, w + 2, h + 2);

	intArray result = IntCache::allocate(w * h);
	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			initRandom(x + xo, y + yo);
			int old = b[(x + 1) + (y + 1) * (w + 2)];
			if ((old == Biome::swampland->id && nextRandom(6) == 0) || ((old == Biome::jungle->id || old == Biome::jungleHills->id) && nextRandom(8) == 0))
			{
				result[x + y * w] = Biome::river->id;
			}
			else
			{
				result[x + y * w] = old;
			}
		}
	}

	return result;
}