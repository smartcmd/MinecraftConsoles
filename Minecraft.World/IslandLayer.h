#pragma once

#include "Layer.h"

class IslandLayer : public Layer
{
public:
	IslandLayer(int64_t seedMixup);

	intArray getArea(int xo, int yo, int w, int h);
};