#pragma once

#include "Layer.h"

class DownfallMixerLayer : public Layer
{
private:
	shared_ptr<Layer> downfall;
	int layer;

public:
	DownfallMixerLayer(shared_ptr<Layer> downfall, shared_ptr<Layer> parent, int layer);
	intArray getArea(int xo, int yo, int w, int h);
};