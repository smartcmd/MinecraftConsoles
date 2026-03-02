#include "stdafx.h"
#include "CaveSpiderRenderer.h"

ResourceLocation CaveSpiderRenderer::CAVE_SPIDER_LOCATION = ResourceLocation(TN_MOB_CAVE_SPIDER);
float CaveSpiderRenderer::s_scale = 0.7f;

CaveSpiderRenderer::CaveSpiderRenderer() : SpiderRenderer()
{
    shadowRadius *= s_scale;
}

void CaveSpiderRenderer::scale(shared_ptr<LivingEntity> mob, float a)
{
    glScalef(s_scale, s_scale, s_scale);
}

ResourceLocation *CaveSpiderRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
    return &CAVE_SPIDER_LOCATION;
}