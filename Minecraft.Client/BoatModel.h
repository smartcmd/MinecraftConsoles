#pragma once
#include "Model.h"
#include "ModelPart.h"

class BoatModel : public Model
{
public:
    ModelPart *cubes[5];
    BoatModel();
    virtual void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled);
};