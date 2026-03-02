#pragma once

#include "Layer.h"

class SmoothZoomLayer : public Layer
{
public:
	SmoothZoomLayer(__int64 seedMixup, shared_ptr<Layer>parent);

	virtual intArray getArea(int xo, int yo, int w, int h);
	static shared_ptr<Layer>zoom(__int64 seed, shared_ptr<Layer>sup, int count);
};