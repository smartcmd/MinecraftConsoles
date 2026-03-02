#include "stdafx.h"
#include "net.minecraft.world.level.biome.h"
#include "net.minecraft.world.level.newbiome.layer.h"

RiverLayer::RiverLayer(__int64 seedMixup, shared_ptr<Layer>parent) : Layer(seedMixup)
{
	this->parent = parent;
}

intArray RiverLayer::getArea(int xo, int yo, int w, int h)
{
	int px = xo - 1;
	int py = yo - 1;
	int pw = w + 2;
	int ph = h + 2;
	intArray p = parent->getArea(px, py, pw, ph);

	intArray result = IntCache::allocate(w * h);
	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			int l = p[(x + 0) + (y + 1) * pw];
			int r = p[(x + 2) + (y + 1) * pw];
			int u = p[(x + 1) + (y + 0) * pw];
			int d = p[(x + 1) + (y + 2) * pw];
			int c = p[(x + 1) + (y + 1) * pw];
			if (c == 0 || (l == 0 || r == 0 || u == 0 || d == 0) || c != l || c != u || c != r || c != d)
			{
				result[x + y * w] = Biome::river->id;
			}
			else
			{
				result[x + y * w] = -1;
			}
		}
	}

	return result;
}