#pragma once

#include "Layer.h"

class RiverInitLayer : public Layer
{
public:
	RiverInitLayer(int64_t seed, shared_ptr<Layer>parent);

	intArray getArea(int xo, int yo, int w, int h);
};