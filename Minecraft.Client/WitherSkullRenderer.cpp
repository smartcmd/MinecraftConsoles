#include "stdafx.h"
#include "WitherSkullRenderer.h"
#include "SkeletonHeadModel.h"
#include "../Minecraft.World/WitherSkull.h"

ResourceLocation WitherSkullRenderer::WITHER_ARMOR_LOCATION(TN_MOB_WITHER_INVULNERABLE);
ResourceLocation WitherSkullRenderer::WITHER_LOCATION(TN_MOB_WITHER);

WitherSkullRenderer::WitherSkullRenderer()
{
	model = new SkeletonHeadModel();
}

void WitherSkullRenderer::render(shared_ptr<Entity> entity, double x, double y, double z, float rot, float a)
{
	glPushMatrix();
	glDisable(GL_CULL_FACE);

	float headRot = rotlerp(entity->yRotO, entity->yRot, a);
	float headRotx = entity->xRotO + (entity->xRot - entity->xRotO) * a;

	glTranslatef((float) x, (float) y, (float) z);

	float scale = 1 / 16.0f;
	glEnable(GL_RESCALE_NORMAL);
	glScalef(-1, -1, 1);

	glEnable(GL_ALPHA_TEST);

	bindTexture(entity);

	model->render(entity, 0, 0, 0, headRot, headRotx, scale, true);

	glPopMatrix();
}

ResourceLocation *WitherSkullRenderer::getTextureLocation(shared_ptr<Entity> entity)
{
	shared_ptr<WitherSkull> mob = dynamic_pointer_cast<WitherSkull>(entity);

	return mob->isDangerous() ? &WITHER_ARMOR_LOCATION : &WITHER_LOCATION;
}

float WitherSkullRenderer::rotlerp(float from, float to, float a)
{
	float diff = to - from;
	while (diff < -180)
		diff += 360;
	while (diff >= 180)
		diff -= 360;
	return from + a * diff;
}