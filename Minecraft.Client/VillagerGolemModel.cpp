#include "stdafx.h"
#include "..\Minecraft.World\net.minecraft.world.entity.animal.h"
#include "ModelPart.h"
#include "VillagerGolemModel.h"

VillagerGolemModel::VillagerGolemModel(float g, float yOffset)
{
	int xTexSize = 128;
	int yTexSize = 128;

	head = (new ModelPart(this))->setTexSize(xTexSize, yTexSize);
	head->setPos(0, 0 + yOffset, -2);
	head->texOffs(0, 0)->addBox(-4, -12, -5.5f, 8, 10, 8, g);
	head->texOffs(24, 0)->addBox(-1, -5.0f, -7.5f, 2, 4, 2, g);

	body = (new ModelPart(this))->setTexSize(xTexSize, yTexSize);
	body->setPos(0, 0 + yOffset, 0);
	body->texOffs(0, 40)->addBox(-9, -2, -6, 18, 12, 11, g);
	body->texOffs(0, 70)->addBox(-4.5f, 10, -3, 9, 5, 6, g + 0.5f);

	arm0 = (new ModelPart(this))->setTexSize(xTexSize, yTexSize);
	arm0->setPos(0, -7, 0);
	arm0->texOffs(60, 21)->addBox(-13, -2.5f, -3, 4, 30, 6, g);

	arm1 = (new ModelPart(this))->setTexSize(xTexSize, yTexSize);
	arm1->setPos(0, -7, 0);
	arm1->texOffs(60, 58)->addBox(9, -2.5f, -3, 4, 30, 6, g);

	leg0 = (new ModelPart(this, 0, 22))->setTexSize(xTexSize, yTexSize);
	leg0->setPos(-4.0f, 18 + yOffset, 0);
	leg0->texOffs(37, 0)->addBox(-3.5f, -3, -3, 6, 16, 5, g);

	leg1 = (new ModelPart(this, 0, 22))->setTexSize(xTexSize, yTexSize);
	leg1->bMirror = true;
	leg1->texOffs(60, 0)->setPos(5.0f, 18 + yOffset, 0);
	leg1->addBox(-3.5f, -3, -3, 6, 16, 5, g);
}

void VillagerGolemModel::render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled)
{
	setupAnim(time, r, bob, yRot, xRot, scale);

	head->render(scale, usecompiled);
	body->render(scale, usecompiled);
	leg0->render(scale, usecompiled);
	leg1->render(scale, usecompiled);
	arm0->render(scale, usecompiled);
	arm1->render(scale, usecompiled);
}

void VillagerGolemModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, unsigned int uiBitmaskOverrideAnim)
{
	head->yRot = yRot / (float) (180 / PI);
	head->xRot = xRot / (float) (180 / PI);

	leg0->xRot = (-1.5f * triangleWave(time, 13)) * r;
	leg1->xRot = (1.5f * triangleWave(time, 13)) * r;
	leg0->yRot = 0;
	leg1->yRot = 0;
}

void VillagerGolemModel::prepareMobModel(shared_ptr<Mob> mob, float time, float r, float a)
{
	shared_ptr<VillagerGolem> vg = dynamic_pointer_cast<VillagerGolem>(mob);
	int attackTick = vg->getAttackAnimationTick();
	if (attackTick > 0)
	{
		arm0->xRot = (-2.0f + 1.5f * triangleWave(attackTick - a, 10));
		arm1->xRot = (-2.0f + 1.5f * triangleWave(attackTick - a, 10));
	}
	else
	{
		int offerFlowerTick = vg->getOfferFlowerTick();
		if (offerFlowerTick > 0)
		{
			arm0->xRot = -0.8f + 0.025f * triangleWave(offerFlowerTick, 70);
			arm1->xRot = 0;
		}
		else
		{
			arm0->xRot = (-0.2f + 1.5f * triangleWave(time, 13)) * r;
			arm1->xRot = (-0.2f - 1.5f * triangleWave(time, 13)) * r;
		}
	}
}

float VillagerGolemModel::triangleWave(float bob, float period)
{
	return (abs(fmod(bob, period) - period * 0.5f) - period * 0.25f) / (period * 0.25f);
}