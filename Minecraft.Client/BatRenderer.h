#pragma once
#include "MobRenderer.h"

class BatModel;

class BatRenderer : public MobRenderer
{
	static ResourceLocation BAT_LOCATION;
	int modelVersion;

public:
	BatRenderer();
	virtual void render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a);

protected:
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);
	virtual void scale(shared_ptr<LivingEntity> mob, float a);
	virtual void setupPosition(shared_ptr<LivingEntity> mob, double x, double y, double z);
	virtual void setupRotations(shared_ptr<LivingEntity> mob, float bob, float bodyRot, float a);
};