#pragma once
#include "MobRenderer.h"

class SnowManModel;

class SnowManRenderer : public MobRenderer
{
private:
	SnowManModel *model;
	static ResourceLocation SNOWMAN_LOCATION;

public:
	SnowManRenderer();

protected:
	virtual void additionalRendering(shared_ptr<LivingEntity> _mob, float a);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);
};