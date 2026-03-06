#include "stdafx.h"
#include "net.minecraft.world.level.newbiome.layer.h"

RiverInitLayer::RiverInitLayer(int64_t seed, shared_ptr<Layer>parent) : Layer(seed)
{
	this->parent = parent;
}

intArray RiverInitLayer::getArea(int xo, int yo, int w, int h)
{
	intArray b = parent->getArea(xo, yo, w, h);

	intArray result = IntCache::allocate(w * h);
	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			initRandom(x + xo, y + yo);
			result[x + y * w] = b[x + y * w] > 0 ? nextRandom(2) + 2 : 0;
		}
	}

	return result;
}