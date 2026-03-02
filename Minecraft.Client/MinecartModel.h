#pragma once
#include "Model.h"

class MinecartModel : public Model
{
public:
	static const int MINECART_LENGTH=6;

	ModelPart *cubes[MINECART_LENGTH];

    MinecartModel();
    virtual void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled);
};
