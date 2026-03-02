#pragma once
#include "EntityRenderer.h"

class FishingHookRenderer : public EntityRenderer
{
public:	
	virtual void render(shared_ptr<Entity> _hook, double x, double y, double z, float rot, float a);
};