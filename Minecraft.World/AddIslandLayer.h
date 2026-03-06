#pragma once

#include "Layer.h"

class AddIslandLayer : public Layer
{
public:
	AddIslandLayer(int64_t seedMixup, shared_ptr<Layer>parent);

	intArray getArea(int xo, int yo, int w, int h);
};