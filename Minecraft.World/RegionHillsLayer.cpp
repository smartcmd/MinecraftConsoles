#include "stdafx.h"
#include "net.minecraft.world.level.biome.h"
#include "IntCache.h"
#include "RegionHillsLayer.h"

RegionHillsLayer::RegionHillsLayer(__int64 seed, shared_ptr<Layer> parent) : Layer(seed)
{
	this->parent = parent;
}

intArray RegionHillsLayer::getArea(int xo, int yo, int w, int h)
{
	intArray b = parent->getArea(xo - 1, yo - 1, w + 2, h + 2);

	intArray result = IntCache::allocate(w * h);
	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			initRandom(x + xo, y + yo);
			int old = b[(x + 1) + (y + 1) * (w + 2)];
			if (nextRandom(3) == 0)
			{
				int next = old;
				if (old == Biome::desert->id)
				{
					next = Biome::desertHills->id;
				}
				else if (old == Biome::forest->id)
				{
					next = Biome::forestHills->id;
				}
				else if (old == Biome::taiga->id)
				{
					next = Biome::taigaHills->id;
				}
				else if (old == Biome::plains->id)
				{
					next = Biome::forest->id;
				}
				else if (old == Biome::iceFlats->id)
				{
					next = Biome::iceMountains->id;
				}
				else if (old == Biome::jungle->id)
				{
					next = Biome::jungleHills->id;

				}
				if (next == old)
				{
					result[x + y * w] = old;
				}
				else
				{
					int _n = b[(x + 1) + (y + 1 - 1) * (w + 2)];
					int _e = b[(x + 1 + 1) + (y + 1) * (w + 2)];
					int _w = b[(x + 1 - 1) + (y + 1) * (w + 2)];
					int _s = b[(x + 1) + (y + 1 + 1) * (w + 2)];
					if (_n == old && _e == old && _w == old && _s == old)
					{
						result[x + y * w] = next;
					}
					else
					{
						result[x + y * w] = old;
					}
				}
			}
			else
			{
				result[x + y * w] = old;
			}
		}
	}

	return result;
}