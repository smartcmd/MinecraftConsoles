#include "stdafx.h"
#include "net.minecraft.world.level.newbiome.layer.h"
#include "net.minecraft.world.level.biome.h"

AddSnowLayer::AddSnowLayer(__int64 seedMixup, shared_ptr<Layer> parent) : Layer(seedMixup)
{
	this->parent = parent;
}

intArray AddSnowLayer::getArea(int xo, int yo, int w, int h)
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
            int c = p[(x + 1) + (y + 1) * pw];
            initRandom(x + xo, y + yo);
            if (c == 0)
			{
                result[x + y * w] = 0;
            }
			else
			{
                int r = nextRandom(5);
                if (r == 0) r = Biome::iceFlats->id;
                else r = 1;
                result[x + y * w] = r;
            }
        }
    }
    return result;
}