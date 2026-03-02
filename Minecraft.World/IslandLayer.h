#pragma once

#include "Layer.h"

class IslandLayer : public Layer
{
public:
	IslandLayer(__int64 seedMixup);

	intArray getArea(int xo, int yo, int w, int h);
};