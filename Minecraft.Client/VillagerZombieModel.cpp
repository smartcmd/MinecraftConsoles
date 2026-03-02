#include "stdafx.h"
#include "ModelPart.h"
#include "VillagerZombieModel.h"
#include "..\Minecraft.World\Mth.h"

void VillagerZombieModel::_init(float g, float yOffset, bool isArmor)
{
	delete head;
	if (isArmor)
	{
		head = new ModelPart(this, 0, 0);
		head->addBox(-4, -10, -4, 8, 6, 8, g); // Head
		head->setPos(0, 0 + yOffset, 0);
	}
	else
	{
		head = new ModelPart(this);
		head->setPos(0, 0 + yOffset, 0);
		head->texOffs(0, 32)->addBox(-4, -10, -4, 8, 10, 8, g);
		head->texOffs(24, 32)->addBox(-1, -3, -6, 2, 4, 2, g);
	}


	// 4J added - compile now to avoid random performance hit first time cubes are rendered
	head->compile(1.0f/16.0f);
}

VillagerZombieModel::VillagerZombieModel() : HumanoidModel(0, 0, 64, 64)
{
	_init(0, 0, false);
}

VillagerZombieModel::VillagerZombieModel(float g, float yOffset, bool isArmor) : HumanoidModel(g, 0, 64, isArmor ? 32 : 64)
{
	_init(g, yOffset, isArmor);
}

int VillagerZombieModel::version()
{
	return 10;
}

void VillagerZombieModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim)
{
	HumanoidModel::setupAnim(time, r, bob, yRot, xRot, scale, entity, uiBitmaskOverrideAnim);

	float attack2 = Mth::sin(attackTime * PI);
	float attack = Mth::sin((1 - (1 - attackTime) * (1 - attackTime)) * PI);
	arm0->zRot = 0;
	arm1->zRot = 0;
	arm0->yRot = -(0.1f - attack2 * 0.6f);
	arm1->yRot = +(0.1f - attack2 * 0.6f);
	arm0->xRot = -PI / 2.0f;
	arm1->xRot = -PI / 2.0f;
	arm0->xRot -= attack2 * 1.2f - attack * 0.4f;
	arm1->xRot -= attack2 * 1.2f - attack * 0.4f;

	arm0->zRot += (Mth::cos(bob * 0.09f) * 0.05f + 0.05f);
	arm1->zRot -= (Mth::cos(bob * 0.09f) * 0.05f + 0.05f);
	arm0->xRot += (Mth::sin(bob * 0.067f) * 0.05f);
	arm1->xRot -= (Mth::sin(bob * 0.067f) * 0.05f);
}