#include "stdafx.h"
#include "SkeletonModel.h"
#include "..\Minecraft.World\Mth.h"
#include "..\Minecraft.World\net.minecraft.world.entity.monster.h"
#include "ModelPart.h"

void SkeletonModel::_init(float g)
{
	arm0 = new ModelPart(this, 24 + 16, 16);
	arm0->addBox(-1, -2, -1, 2, 12, 2, g); // Arm0
	arm0->setPos(-5, 2, 0);

	arm1 = new ModelPart(this, 24 + 16, 16);
	arm1->bMirror = true;
	arm1->addBox(-1, -2, -1, 2, 12, 2, g); // Arm1
	arm1->setPos(5, 2, 0);

	leg0 = new ModelPart(this, 0, 16);
	leg0->addBox(-1, 0, -1, 2, 12, 2, g); // Leg0
	leg0->setPos(-2, 12, 0);

	leg1 = new ModelPart(this, 0, 16);
	leg1->bMirror = true;
	leg1->addBox(-1, 0, -1, 2, 12, 2, g); // Leg1
	leg1->setPos(2, 12, 0);

	// 4J added - compile now to avoid random performance hit first time cubes are rendered
	arm0->compile(1.0f/16.0f);
	arm1->compile(1.0f/16.0f);
	leg0->compile(1.0f/16.0f);
	leg1->compile(1.0f/16.0f);
}

SkeletonModel::SkeletonModel() : ZombieModel(0, 0, 64, 32)
{
	_init(0);
}

SkeletonModel::SkeletonModel(float g) : ZombieModel(g, 0, 64, 32)
{
	_init(g);
}

void SkeletonModel::prepareMobModel(shared_ptr<LivingEntity> mob, float time, float r, float a)
{
	ZombieModel::prepareMobModel(mob, time, r, a);

	bowAndArrow = dynamic_pointer_cast<Skeleton>(mob)->getSkeletonType() == Skeleton::TYPE_WITHER;
}

void SkeletonModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim)
{
	bowAndArrow=true;
	ZombieModel::setupAnim(time, r, bob, yRot, xRot, scale, entity, uiBitmaskOverrideAnim);
}