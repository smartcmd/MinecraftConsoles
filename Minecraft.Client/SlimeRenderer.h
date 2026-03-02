#pragma once
#include "MobRenderer.h"

class SlimeRenderer : public MobRenderer
{
private:
	Model *armor;
	static ResourceLocation SLIME_LOCATION;

public:
	SlimeRenderer(Model *model, Model *armor, float shadow);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);

protected:
	virtual int prepareArmor(shared_ptr<LivingEntity> _slime, int layer, float a);
    virtual void scale(shared_ptr<LivingEntity> _slime, float a);
};