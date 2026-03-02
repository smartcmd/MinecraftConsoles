#pragma once

#include "Layer.h"

class TemperatureMixerLayer : public Layer
{
private:
	shared_ptr<Layer>temp;
	int layer;

public:
	TemperatureMixerLayer(shared_ptr<Layer>temp, shared_ptr<Layer>parent, int layer);

	virtual intArray getArea(int xo, int yo, int w, int h);
};