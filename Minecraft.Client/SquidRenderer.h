#pragma once
#include "MobRenderer.h"

class SquidRenderer : public MobRenderer
{
private:
	static ResourceLocation SQUID_LOCATION;

public:
	SquidRenderer(Model *model, float shadow);
	virtual void render(shared_ptr<Entity> mob, double x, double y, double z, float rot, float a);

protected:
	virtual void setupRotations(shared_ptr<LivingEntity> _mob, float bob, float bodyRot, float a);
    virtual float getBob(shared_ptr<LivingEntity> _mob, float a);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);
};