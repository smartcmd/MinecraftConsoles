#include "stdafx.h"
#include "../Minecraft.World/net.minecraft.world.entity.ambient.h"
#include "BatRenderer.h"
#include "BatModel.h"

ResourceLocation BatRenderer::BAT_LOCATION = ResourceLocation(TN_MOB_BAT);

BatRenderer::BatRenderer() : MobRenderer(new BatModel(), 0.25f)
{
	modelVersion = static_cast<BatModel *>(model)->modelVersion();
}

void BatRenderer::render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a)
{
	int modelVersion = (dynamic_cast<BatModel*>(model))->modelVersion();
	if (modelVersion != this->modelVersion) {
		this->modelVersion = modelVersion;
		model = new BatModel();
	}
	MobRenderer::render(_mob, x, y, z, rot, a);
}

ResourceLocation *BatRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
	return &BAT_LOCATION;
}

void BatRenderer::scale(shared_ptr<LivingEntity> mob, float a)
{
	glScalef(.35f, .35f, .35f);
}

void BatRenderer::setupPosition(shared_ptr<LivingEntity> mob, double x, double y, double z)
{
	MobRenderer::setupPosition(mob, x, y, z);
}

void BatRenderer::setupRotations(shared_ptr<LivingEntity> _mob, float bob, float bodyRot, float a)
{
	shared_ptr<Bat> mob = dynamic_pointer_cast<Bat>(_mob);
	if (!mob->isResting())
	{
		glTranslatef(0, cos(bob * .3f) * .1f, 0);
	}
	else
	{
		glTranslatef(0, -.1f, 0);
	}
	MobRenderer::setupRotations(mob, bob, bodyRot, a);
}