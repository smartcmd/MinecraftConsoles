#pragma once
#include "EntityRenderer.h"

class FishingHookRenderer : public EntityRenderer
{
private:
	static ResourceLocation PARTICLE_LOCATION;

public:	
	virtual void render(shared_ptr<Entity> _hook, double x, double y, double z, float rot, float a);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);
};