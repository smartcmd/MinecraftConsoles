#pragma once
#include "MobRenderer.h"

class LavaSlimeRenderer : public MobRenderer
{
private:
	int modelVersion;
	static ResourceLocation MAGMACUBE_LOCATION;

public:
	LavaSlimeRenderer();
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);	

protected:
	virtual void scale(shared_ptr<LivingEntity> _slime, float a);
};