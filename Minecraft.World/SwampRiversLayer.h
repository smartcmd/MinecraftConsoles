#pragma once

#include "Layer.h"

class SwampRiversLayer : public Layer
{
public:
	SwampRiversLayer(__int64 seed, shared_ptr<Layer> parent);

	intArray getArea(int xo, int yo, int w, int h);
};