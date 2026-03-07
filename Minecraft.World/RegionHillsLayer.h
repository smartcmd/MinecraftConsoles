#pragma once

#include "Layer.h"

class RegionHillsLayer : public Layer
{
public:
	RegionHillsLayer(int64_t seed, shared_ptr<Layer> parent);

	intArray getArea(int xo, int yo, int w, int h);
};