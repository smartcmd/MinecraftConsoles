#include "stdafx.h"
#include "..\Minecraft.World\net.minecraft.world.entity.animal.h"
#include "SheepModel.h"
#include "ModelPart.h"

SheepModel::SheepModel() : QuadrupedModel(12, 0)
{
	headXRot = 0.0f;

    head = new ModelPart(this, 0, 0);
    head->addBox(-3, -4, -6, 6, 6, 8, 0); // Head
    head->setPos(0, 12-6, -8);
        
    body = new ModelPart(this, 28, 8);
    body->addBox(-4, -10, -7, 8, 16, 6, 0); // Body
    body->setPos(0, 11+6-12, 2);

	// 4J added - compile now to avoid random performance hit first time cubes are rendered
	head->compile(1.0f/16.0f);
    body->compile(1.0f/16.0f);
}

void SheepModel::prepareMobModel(shared_ptr<Mob> mob, float time, float r, float a)
{
	QuadrupedModel::prepareMobModel(mob, time, r, a);

	shared_ptr<Sheep> sheep = dynamic_pointer_cast<Sheep>(mob);
	head->y = 6 + sheep->getHeadEatPositionScale(a) * 9.0f;
	headXRot = sheep->getHeadEatAngleScale(a);
}

void SheepModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, unsigned int uiBitmaskOverrideAnim)
{
	QuadrupedModel::setupAnim(time, r, bob, yRot, xRot, scale);

	head->xRot = headXRot;
}