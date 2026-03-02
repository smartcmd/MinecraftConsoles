#pragma once
#include "EntityRenderer.h"

class MinecartRenderer : public EntityRenderer
{
protected:
	Model *model;

public:
	MinecartRenderer();
    void render(shared_ptr<Entity> _cart, double x, double y, double z, float rot, float a);
};