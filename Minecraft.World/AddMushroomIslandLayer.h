#pragma once
#include "Layer.h"

class AddMushroomIslandLayer : public Layer
{
public:
    AddMushroomIslandLayer(__int64 seedMixup, shared_ptr<Layer> parent);
    virtual intArray getArea(int xo, int yo, int w, int h);
};