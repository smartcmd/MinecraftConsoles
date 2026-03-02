#pragma once

#include "Layer.h"

class RiverMixerLayer : public Layer
{
private:
	shared_ptr<Layer>biomes;
	shared_ptr<Layer>rivers;

public:
	RiverMixerLayer(__int64 seed, shared_ptr<Layer>biomes, shared_ptr<Layer>rivers);

	virtual void init(__int64 seed);
	virtual intArray getArea(int xo, int yo, int w, int h);
};