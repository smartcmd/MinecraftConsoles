#pragma once
#include "MobRenderer.h"

class MushroomCowRenderer : public MobRenderer
{
private:
	static ResourceLocation MOOSHROOM_LOCATION;

public:
	MushroomCowRenderer(Model *model, float shadow);

	virtual void render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a);

protected:
	virtual void additionalRendering(shared_ptr<LivingEntity> _mob, float a);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);
};