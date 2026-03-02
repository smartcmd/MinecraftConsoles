#pragma once
#include "EntityRenderer.h"

class ArrowRenderer : public EntityRenderer
{
public:
	virtual void render(shared_ptr<Entity> _arrow, double x, double y, double z, float rot, float a);
};
