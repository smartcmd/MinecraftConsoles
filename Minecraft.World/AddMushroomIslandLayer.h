#pragma once
#include "Layer.h"

class AddMushroomIslandLayer : public Layer
{
public:
    AddMushroomIslandLayer(int64_t seedMixup, shared_ptr<Layer> parent);
    virtual intArray getArea(int xo, int yo, int w, int h);
};