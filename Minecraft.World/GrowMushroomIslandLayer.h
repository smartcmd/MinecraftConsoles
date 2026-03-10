#pragma once
#include "Layer.h"

class GrowMushroomIslandLayer : public Layer
{
public:
    GrowMushroomIslandLayer(int64_t seedMixup, shared_ptr<Layer> parent);
    virtual intArray getArea(int xo, int yo, int w, int h);
};