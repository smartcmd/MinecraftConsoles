#include "stdafx.h"
#include "GiantMobRenderer.h"	

ResourceLocation GiantMobRenderer::ZOMBIE_LOCATION = ResourceLocation(TN_ITEM_ARROWS);

GiantMobRenderer::GiantMobRenderer(Model *model, float shadow, float _scale) : MobRenderer(model, shadow *_scale)
{
	this->_scale = _scale;
}

void GiantMobRenderer::scale(shared_ptr<LivingEntity> mob, float a)
{
	glScalef(_scale, _scale, _scale);
}

ResourceLocation *GiantMobRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
    return &ZOMBIE_LOCATION;
}