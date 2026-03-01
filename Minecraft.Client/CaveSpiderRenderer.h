#pragma once
#include "SpiderRenderer.h"

class CaveSpider;

class CaveSpiderRenderer : public SpiderRenderer
{
private:
	static ResourceLocation CAVE_SPIDER_LOCATION;
	static float s_scale;

public:
	CaveSpiderRenderer();

protected:
    virtual void scale(shared_ptr<LivingEntity> mob, float a);
    virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);
};