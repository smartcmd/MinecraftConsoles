#pragma once
#include "MobRenderer.h"

class SpiderRenderer : public MobRenderer
{
private:
	static ResourceLocation SPIDER_LOCATION;
	static ResourceLocation SPIDER_EYES_LOCATION;

public:
	SpiderRenderer();

protected:
	virtual float getFlipDegrees(shared_ptr<LivingEntity> spider);
	virtual int prepareArmor(shared_ptr<LivingEntity> _spider, int layer, float a);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);
};