#pragma once
#include "MobRenderer.h"

class GhastRenderer : public MobRenderer
{
private:
	static ResourceLocation GHAST_LOCATION;
	static ResourceLocation GHAST_SHOOTING_LOCATION;

public:
	GhastRenderer();	

protected:
	virtual void scale(shared_ptr<LivingEntity> mob, float a);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);
};