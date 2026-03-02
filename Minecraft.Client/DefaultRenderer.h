#pragma once
#include "EntityRenderer.h"

class DefaultRenderer : public EntityRenderer
{
public:
    virtual void render(shared_ptr<Entity> entity, double x, double y, double z, float rot, float a);
};