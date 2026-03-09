#pragma once

#include "Layer.h"

class VoronoiZoom : public Layer
{
public:
	VoronoiZoom(int64_t seedMixup, shared_ptr<Layer>parent);

	virtual intArray getArea(int xo, int yo, int w, int h);

protected:
	int random(int a, int b);
	int random(int a, int b, int c, int d);
};