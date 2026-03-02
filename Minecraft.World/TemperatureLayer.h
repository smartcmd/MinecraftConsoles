#pragma once

#include "Layer.h"

class TemperatureLayer : public Layer
{
public:
	TemperatureLayer(shared_ptr<Layer> parent);

	virtual intArray getArea(int xo, int yo, int w, int h);
};