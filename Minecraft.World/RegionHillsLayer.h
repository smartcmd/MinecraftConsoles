#pragma once

#include "Layer.h"

class RegionHillsLayer : public Layer
{
public:
	RegionHillsLayer(__int64 seed, shared_ptr<Layer> parent);

	intArray getArea(int xo, int yo, int w, int h);
};