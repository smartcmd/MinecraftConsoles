#include "stdafx.h"
#include "EnderCrystalModel.h"
#include "..\Minecraft.World\net.minecraft.world.entity.boss.enderdragon.h"
#include "EnderCrystalRenderer.h"

ResourceLocation EnderCrystalRenderer::ENDER_CRYSTAL_LOCATION = ResourceLocation(TN_MOB_ENDERDRAGON_ENDERCRYSTAL);

EnderCrystalRenderer::EnderCrystalRenderer()
{
	currentModel = -1;
	this->shadowRadius = 0.5f;
}

void EnderCrystalRenderer::render(shared_ptr<Entity> _crystal, double x, double y, double z, float rot, float a)
{
	// 4J - original version used generics and thus had an input parameter of type EnderCrystal rather than shared_ptr<Entity>  we have here - 
	// do some casting around instead
	shared_ptr<EnderCrystal> crystal = dynamic_pointer_cast<EnderCrystal>(_crystal);
	if (currentModel != EnderCrystalModel::MODEL_ID)
	{
		model = new EnderCrystalModel(0);
		currentModel = EnderCrystalModel::MODEL_ID;
	}


	float tt = crystal->time + a;
	glPushMatrix();
	glTranslatef(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
	bindTexture(&ENDER_CRYSTAL_LOCATION);
	float hh = sin(tt * 0.2f) / 2 + 0.5f;
	hh = hh * hh + hh;
	model->render(crystal, 0, tt * 3, hh * 0.2f, 0, 0, 1 / 16.0f, true);

	glPopMatrix();
}

ResourceLocation *EnderCrystalRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
    return &ENDER_CRYSTAL_LOCATION;
}