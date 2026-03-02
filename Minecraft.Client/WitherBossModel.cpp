#include "stdafx.h"
#include "WitherBossModel.h"
#include "..\Minecraft.World\WitherBoss.h"
#include "ModelPart.h"

WitherBossModel::WitherBossModel()
{
	texWidth = 64;
	texHeight = 64;

	upperBodyParts = ModelPartArray(3);

	upperBodyParts[0] = new ModelPart(this, 0, 16);
	upperBodyParts[0]->addBox(-10, 3.9f, -.5f, 20, 3, 3);

	upperBodyParts[1] = new ModelPart(this);
	upperBodyParts[1]->setTexSize(texWidth, texHeight);
	upperBodyParts[1]->setPos(-2, 6.9f, -.5f);
	upperBodyParts[1]->texOffs(0, 22)->addBox(0, 0, 0, 3, 10, 3);
	upperBodyParts[1]->texOffs(24, 22)->addBox(-4.f, 1.5f, .5f, 11, 2, 2);
	upperBodyParts[1]->texOffs(24, 22)->addBox(-4.f, 4, .5f, 11, 2, 2);
	upperBodyParts[1]->texOffs(24, 22)->addBox(-4.f, 6.5f, .5f, 11, 2, 2);

	upperBodyParts[2] = new ModelPart(this, 12, 22);
	upperBodyParts[2]->addBox(0, 0, 0, 3, 6, 3);

	heads = ModelPartArray(3);
	heads[0] = new ModelPart(this, 0, 0);
	heads[0]->addBox(-4, -4, -4, 8, 8, 8);
	heads[1] = new ModelPart(this, 32, 0);
	heads[1]->addBox(-4, -4, -4, 6, 6, 6);
	heads[1]->x = -8;
	heads[1]->y = 4;
	heads[2] = new ModelPart(this, 32, 0);
	heads[2]->addBox(-4, -4, -4, 6, 6, 6);
	heads[2]->x = 10;
	heads[2]->y = 4;
}

int WitherBossModel::modelVersion()
{
	return 32;
}

void WitherBossModel::render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled)
{
	setupAnim(time, r, bob, yRot, xRot, scale, entity);

	for (int i = 0; i < heads.length; i++)
	{
		heads[i]->render(scale, usecompiled);
	}
	for (int i = 0; i < upperBodyParts.length; i++)
	{
		upperBodyParts[i]->render(scale, usecompiled);
	}
}

void WitherBossModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim)
{
	float anim = cos(bob * .1f);
	upperBodyParts[1]->xRot = (.065f + .05f * anim) * PI;

	upperBodyParts[2]->setPos(-2.f, 6.9f + cos(upperBodyParts[1]->xRot) * 10.f, -.5f + sin(upperBodyParts[1]->xRot) * 10.f);
	upperBodyParts[2]->xRot = (.265f + .1f * anim) * PI;

	heads[0]->yRot = yRot / (180 / PI);
	heads[0]->xRot = xRot / (180 / PI);
}

void WitherBossModel::prepareMobModel(shared_ptr<LivingEntity> mob, float time, float r, float a)
{
	shared_ptr<WitherBoss> boss = dynamic_pointer_cast<WitherBoss>(mob);

	for (int i = 1; i < 3; i++)
	{
		heads[i]->yRot = (boss->getHeadYRot(i - 1) - mob->yBodyRot) / (180 / PI);
		heads[i]->xRot = boss->getHeadXRot(i - 1) / (180 / PI);
	}
}