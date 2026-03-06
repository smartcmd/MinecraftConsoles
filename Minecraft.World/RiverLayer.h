#pragma once

#include "Layer.h"

class RiverLayer : public Layer
{
public:
	RiverLayer(int64_t seedMixup, shared_ptr<Layer>parent);
	intArray getArea(int xo, int yo, int w, int h);
};