#pragma once

#include "Layer.h"

class RiverMixerLayer : public Layer
{
private:
	shared_ptr<Layer>biomes;
	shared_ptr<Layer>rivers;

public:
	RiverMixerLayer(int64_t seed, shared_ptr<Layer>biomes, shared_ptr<Layer>rivers);

	virtual void init(int64_t seed);
	virtual intArray getArea(int xo, int yo, int w, int h);
};