#include "stdafx.h"
#include "..\Minecraft.World\net.minecraft.world.entity.animal.h"
#include "SheepFurModel.h"
#include "ModelPart.h"

SheepFurModel::SheepFurModel() : QuadrupedModel(12, 0)
{
	headXRot = 0.0f;

    head = new ModelPart(this, 0, 0);
    head->addBox(-3, -4, -4, 6, 6, 6, 0.6f); // Head
    head->setPos(0, 12 - 6, -8);

    body = new ModelPart(this, 28, 8);
    body->addBox(-4, -10, -7, 8, 16, 6, 1.75f); // Body
    body->setPos(0, 11 + 6 - 12, 2);

    float g = 0.5f;
    leg0 = new ModelPart(this, 0, 16);
    leg0->addBox(-2, 0, -2, 4, 6, 4, g); // Leg0
    leg0->setPos(-3, 18 + 6 - 12, 7);

    leg1 = new ModelPart(this, 0, 16);
    leg1->addBox(-2, 0, -2, 4, 6, 4, g); // Leg1
    leg1->setPos(3, 18 + 6 - 12, 7);

    leg2 = new ModelPart(this, 0, 16);
    leg2->addBox(-2, 0, -2, 4, 6, 4, g); // Leg2
    leg2->setPos(-3, 18 + 6 - 12, -5);

    leg3 = new ModelPart(this, 0, 16);
    leg3->addBox(-2, 0, -2, 4, 6, 4, g); // Leg3
    leg3->setPos(3, 18 + 6 - 12, -5);

	// 4J added - compile now to avoid random performance hit first time cubes are rendered
	head->compile(1.0f/16.0f);
    body->compile(1.0f/16.0f);
    leg0->compile(1.0f/16.0f);
    leg1->compile(1.0f/16.0f);
    leg2->compile(1.0f/16.0f);
    leg3->compile(1.0f/16.0f);
}

void SheepFurModel::prepareMobModel(shared_ptr<Mob> mob, float time, float r, float a)
{
	QuadrupedModel::prepareMobModel(mob, time, r, a);

	shared_ptr<Sheep> sheep = dynamic_pointer_cast<Sheep>(mob);
	head->y = 6 + sheep->getHeadEatPositionScale(a) * 9.0f;
	headXRot = sheep->getHeadEatAngleScale(a);
}

void SheepFurModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, unsigned int uiBitmaskOverrideAnim)
{
	QuadrupedModel::setupAnim(time, r, bob, yRot, xRot, scale);
	head->xRot = headXRot;
}