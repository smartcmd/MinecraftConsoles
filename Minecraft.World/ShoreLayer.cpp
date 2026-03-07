#include "stdafx.h"
#include "net.minecraft.world.level.newbiome.layer.h"
#include "net.minecraft.world.level.biome.h"

ShoreLayer::ShoreLayer(int64_t seed, shared_ptr<Layer> parent) : Layer(seed)
{
	this->parent = parent;
}

intArray ShoreLayer::getArea(int xo, int yo, int w, int h)
{
    intArray b = parent->getArea(xo - 1, yo - 1, w + 2, h + 2);

    intArray result = IntCache::allocate(w * h);
    for (int y = 0; y < h; y++)
	{
        for (int x = 0; x < w; x++)
		{
            initRandom(x + xo, y + yo);
            int old = b[(x + 1) + (y + 1) * (w + 2)];
            if (old == Biome::mushroomIsland->id)
			{
                int _n = b[(x + 1) + (y + 1 - 1) * (w + 2)];
                int _e = b[(x + 1 + 1) + (y + 1) * (w + 2)];
                int _w = b[(x + 1 - 1) + (y + 1) * (w + 2)];
                int _s = b[(x + 1) + (y + 1 + 1) * (w + 2)];
                if (_n == Biome::ocean->id || _e == Biome::ocean->id || _w == Biome::ocean->id || _s == Biome::ocean->id)
				{
                    result[x + y * w] = Biome::mushroomIslandShore->id;
                }
				else
				{
                    result[x + y * w] = old;
                }
			}
			else if (old != Biome::ocean->id && old != Biome::river->id && old != Biome::swampland->id && old != Biome::extremeHills->id)
			{
				int _n = b[(x + 1) + (y + 1 - 1) * (w + 2)];
				int _e = b[(x + 1 + 1) + (y + 1) * (w + 2)];
				int _w = b[(x + 1 - 1) + (y + 1) * (w + 2)];
				int _s = b[(x + 1) + (y + 1 + 1) * (w + 2)];
				if (_n == Biome::ocean->id || _e == Biome::ocean->id || _w == Biome::ocean->id || _s == Biome::ocean->id)
				{
					result[x + y * w] = Biome::beaches->id;
				}
				else
				{
					result[x + y * w] = old;
				}
			}
			else if (old == Biome::extremeHills->id)
			{
				int _n = b[(x + 1) + (y + 1 - 1) * (w + 2)];
				int _e = b[(x + 1 + 1) + (y + 1) * (w + 2)];
				int _w = b[(x + 1 - 1) + (y + 1) * (w + 2)];
				int _s = b[(x + 1) + (y + 1 + 1) * (w + 2)];
				if (_n != Biome::extremeHills->id || _e != Biome::extremeHills->id || _w != Biome::extremeHills->id || _s != Biome::extremeHills->id)
				{
					result[x + y * w] = Biome::smallerExtremeHills->id;
				}
				else
				{
					result[x + y * w] = old;
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