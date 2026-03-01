#pragma once
#include "MobRenderer.h"

class WolfRenderer : public MobRenderer
{
private:
	static ResourceLocation *WOLF_LOCATION;
	static ResourceLocation *WOLF_TAME_LOCATION;
	static ResourceLocation *WOLF_ANGRY_LOCATION;
	static ResourceLocation *WOLF_COLLAR_LOCATION;

public:
	WolfRenderer(Model *model, Model *armor, float shadow);

protected:
	virtual float getBob(shared_ptr<LivingEntity> _mob, float a);
	virtual int prepareArmor(shared_ptr<LivingEntity> mob, int layer, float a);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);
};