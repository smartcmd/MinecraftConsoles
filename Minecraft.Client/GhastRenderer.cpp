#include "stdafx.h"
#include "GhastRenderer.h"
#include "GhastModel.h"
#include "..\Minecraft.World\net.minecraft.world.entity.monster.h"

GhastRenderer::GhastRenderer() : MobRenderer(new GhastModel(), 0.5f)
{
}

void GhastRenderer::scale(shared_ptr<Mob> mob, float a)
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