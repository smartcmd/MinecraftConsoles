#pragma once
#include "MobRenderer.h"

class PigRenderer : public MobRenderer
{
private:
	static ResourceLocation PIG_LOCATION;
	static ResourceLocation SADDLE_LOCATION;

public:
	PigRenderer(Model *model, Model *armor, float shadow);

protected:
	virtual int prepareArmor(shared_ptr<LivingEntity> _pig, int layer, float a);

public:
	virtual void render(shared_ptr<Entity> mob, double x, double y, double z, float rot, float a);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);
};