#include "stdafx.h"
#include "net.minecraft.world.level.newbiome.layer.h"

IslandLayer::IslandLayer(__int64 seedMixup) : Layer(seedMixup)
{
}

intArray IslandLayer::getArea(int xo, int yo, int w, int h)
{
	intArray result = IntCache::allocate(w * h);
	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			initRandom(xo + x, yo + y);
			result[x + y * w] = (nextRandom(10) == 0) ? 1 : 0;
		}
	}
	// if (0, 0) is located here, place an island
	if (xo > -w && xo <= 0 && yo > -h && yo <= 0)
	{
		result[-xo + -yo * w] = 1;
	}
	return result;
}