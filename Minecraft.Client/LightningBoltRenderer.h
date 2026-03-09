#pragma once
#include "EntityRenderer.h"

class LightningBoltRenderer : public EntityRenderer
{
public:
	virtual void render(const shared_ptr<Entity> bolt, double x, double y, double z, float rot, float a);
};