#include "stdafx.h"
#include "GhastRenderer.h"
#include "GhastModel.h"
#include "..\Minecraft.World\net.minecraft.world.entity.monster.h"

ResourceLocation GhastRenderer::GHAST_LOCATION = ResourceLocation(TN_MOB_GHAST);
ResourceLocation GhastRenderer::GHAST_SHOOTING_LOCATION = ResourceLocation(TN_MOB_GHAST_FIRE);

GhastRenderer::GhastRenderer() : MobRenderer(new GhastModel(), 0.5f)
{
}

void GhastRenderer::scale(shared_ptr<LivingEntity> mob, float a)
{
	shared_ptr<Ghast> ghast = dynamic_pointer_cast<Ghast>(mob);
        
	float ss = (ghast->oCharge+(ghast->charge-ghast->oCharge)*a)/20.0f;
	if (ss<0) ss = 0;
	ss = 1/(ss*ss*ss*ss*ss*2+1);
	float s = (8+ss)/2;
	float hs = (8+1/ss)/2;
	glScalef(hs, s, hs);
	glColor4f(1, 1, 1, 1);
}

ResourceLocation *GhastRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
	shared_ptr<Ghast> ghast = dynamic_pointer_cast<Ghast>(mob);

    if (ghast->isCharging())
	{
        return &GHAST_SHOOTING_LOCATION;
    }

    return &GHAST_LOCATION;
}