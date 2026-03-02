#pragma once

#include "Layer.h"

class DownfallLayer : public Layer
{
public:
	DownfallLayer(shared_ptr<Layer>parent);
	intArray getArea(int xo, int yo, int w, int h);
};