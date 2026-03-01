#pragma once
#include "MobRenderer.h"

class ChickenRenderer : public MobRenderer
{
private:
	static ResourceLocation CHICKEN_LOCATION;	

public:
	ChickenRenderer(Model *model, float shadow);
    virtual void render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);

protected:
	virtual float getBob(shared_ptr<LivingEntity> _mob, float a);
};