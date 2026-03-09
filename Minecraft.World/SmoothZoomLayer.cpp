#include "stdafx.h"
#include "net.minecraft.world.level.newbiome.layer.h"
#include "System.h"

SmoothZoomLayer::SmoothZoomLayer(int64_t seedMixup, shared_ptr<Layer>parent) : Layer(seedMixup)
{
	this->parent = parent;
}

intArray SmoothZoomLayer::getArea(int xo, int yo, int w, int h)
{
	int px = xo >> 1;
	int py = yo >> 1;
	int pw = (w >> 1) + 3;
	int ph = (h >> 1) + 3;
	intArray p = parent->getArea(px, py, pw, ph);

	intArray tmp = IntCache::allocate((pw * 2) * (ph * 2));
	int ww = (pw << 1);
	for (int y = 0; y < ph - 1; y++)
	{
		int ry = y << 1;
		int pp = ry * ww;
		int ul = p[(0 + 0) + (y + 0) * pw];
		int dl = p[(0 + 0) + (y + 1) * pw];
		for (int x = 0; x < pw - 1; x++)
		{
			initRandom((x + px) << 1, (y + py) << 1);

			int ur = p[(x + 1) + (y + 0) * pw];
			int dr = p[(x + 1) + (y + 1) * pw];

			tmp[pp] = ul;
			tmp[pp++ + ww] = ul + (dl - ul) * (nextRandom(256)) / 256;
			tmp[pp] = ul + (ur - ul) * (nextRandom(256)) / 256;

			int a = ul + (ur - ul) * (nextRandom(256)) / 256;
			int b = dl + (dr - dl) * (nextRandom(256)) / 256;
			tmp[pp++ + ww] = a + (b - a) * (nextRandom(256)) / 256;

			ul = ur;
			dl = dr;
		}
	}
	intArray result = IntCache::allocate(w * h);
	for (int y = 0; y < h; y++)
	{
		System::arraycopy(tmp, (y + (yo & 1)) * (pw << 1) + (xo & 1), &result, y * w, w);
	}
	return result;
}

shared_ptr<Layer>SmoothZoomLayer::zoom(int64_t seed, shared_ptr<Layer>sup, int count)
{
	shared_ptr<Layer>result = sup;
	for (int i = 0; i < count; i++)
	{
		result = std::make_shared<SmoothZoomLayer>(seed + i, result);
	}
	return result;
}