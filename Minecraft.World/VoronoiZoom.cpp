#include "stdafx.h"
#include "net.minecraft.world.level.newbiome.layer.h"
#include "System.h"

VoronoiZoom::VoronoiZoom(int64_t seedMixup, shared_ptr<Layer>parent) : Layer(seedMixup)
{
	this->parent = parent;
}

intArray VoronoiZoom::getArea(int xo, int yo, int w, int h)
{
	xo -= 2;
	yo -= 2;
	int bits = 2;
	int ss = 1 << bits;
	int px = xo >> bits;
	int py = yo >> bits;
	int pw = (w >> bits) + 3;
	int ph = (h >> bits) + 3;
	intArray p = parent->getArea(px, py, pw, ph);

	int ww = pw << bits;
	int hh = ph << bits;
	intArray tmp = IntCache::allocate(ww * hh);
	for (int y = 0; y < ph - 1; y++)
	{
		int ul = p[(0 + 0) + (y + 0) * pw];
		int dl = p[(0 + 0) + (y + 1) * pw];
		for (int x = 0; x < pw - 1; x++)
		{
			double s = ss * 0.9;
			initRandom((x + px) << bits, (y + py) << bits);
			double x0 = (nextRandom(1024) / 1024.0 - 0.5) * s;
			double y0 = (nextRandom(1024) / 1024.0 - 0.5) * s;
			initRandom((x + px + 1) << bits, (y + py) << bits);
			double x1 = (nextRandom(1024) / 1024.0 - 0.5) * s + ss;
			double y1 = (nextRandom(1024) / 1024.0 - 0.5) * s;
			initRandom((x + px) << bits, (y + py + 1) << bits);
			double x2 = (nextRandom(1024) / 1024.0 - 0.5) * s;
			double y2 = (nextRandom(1024) / 1024.0 - 0.5) * s + ss;
			initRandom((x + px + 1) << bits, (y + py + 1) << bits);
			double x3 = (nextRandom(1024) / 1024.0 - 0.5) * s + ss;
			double y3 = (nextRandom(1024) / 1024.0 - 0.5) * s + ss;

			int ur = p[(x + 1) + (y + 0) * pw];
			int dr = p[(x + 1) + (y + 1) * pw];

			for (int yy = 0; yy < ss; yy++)
			{
				int pp = ((y << bits) + yy) * ww + ((x << bits));
				for (int xx = 0; xx < ss; xx++)
				{
					double d0 = ((yy - y0) * (yy - y0) + (xx - x0) * (xx - x0));
					double d1 = ((yy - y1) * (yy - y1) + (xx - x1) * (xx - x1));
					double d2 = ((yy - y2) * (yy - y2) + (xx - x2) * (xx - x2));
					double d3 = ((yy - y3) * (yy - y3) + (xx - x3) * (xx - x3));

					if (d0 < d1 && d0 < d2 && d0 < d3)
					{
						tmp[pp++] = ul;
					}
					else if (d1 < d0 && d1 < d2 && d1 < d3)
					{
						tmp[pp++] = ur;
					}
					else if (d2 < d0 && d2 < d1 && d2 < d3)
					{
						tmp[pp++] = dl;
					}
					else
					{
						tmp[pp++] = dr;
					}
				}
			}

			ul = ur;
			dl = dr;
		}
	}
	intArray result = IntCache::allocate(w * h);
	for (int y = 0; y < h; y++)
	{
		System::arraycopy(tmp, (y + (yo & (ss - 1))) * (pw << bits) + (xo & (ss - 1)), &result, y * w, w);
	}
	return result;
}

int VoronoiZoom::random(int a, int b)
{
	return nextRandom(2) == 0 ? a : b;
}

int VoronoiZoom::random(int a, int b, int c, int d)
{
	if (b == c && c == d) return b;
	if (a == b && a == c) return a;
	if (a == b && a == d) return a;
	if (a == c && a == d) return a;

	if (a == b && c != d) return a;
	if (a == c && b != d) return a;
	if (a == d && b != c) return a;

	if (b == a && c != d) return b;
	if (b == c && a != d) return b;
	if (b == d && a != c) return b;

	if (c == a && b != d) return c;
	if (c == b && a != d) return c;
	if (c == d && a != b) return c;

	if (d == a && b != c) return c;
	if (d == b && a != c) return c;
	if (d == c && a != b) return c;

	int s = nextRandom(4);
	if (s == 0) return a;
	if (s == 1) return b;
	if (s == 2) return c;
	return d;
}