#pragma once

#include "Layer.h"

class AddIslandLayer : public Layer
{
public:
	AddIslandLayer(__int64 seedMixup, shared_ptr<Layer>parent);

	intArray getArea(int xo, int yo, int w, int h);
};