#include "stdafx.h"
#include "SnowmanModel.h"
#include "..\Minecraft.World\Mth.h"
#include "ModelPart.h"

SnowManModel::SnowManModel() : Model()
{
    float yOffset = 4;
    float g = 0;

    head = (new ModelPart(this, 0, 0))->setTexSize(64, 64);
    head->addBox(-4, -8, -4, 8, 8, 8, g - 0.5f); // head
    head->setPos(0, 0 + yOffset, 0);

    arm1 = (new ModelPart(this, 32, 0))->setTexSize(64, 64);
    arm1->addBox(-1, 0, -1, 12, 2, 2, g - 0.5f); // arm
    arm1->setPos(0, 0 + yOffset + 9 - 7, 0);

    arm2 = (new ModelPart(this, 32, 0))->setTexSize(64, 64);
    arm2->addBox(-1, 0, -1, 12, 2, 2, g - 0.5f); // arm
    arm2->setPos(0, 0 + yOffset + 9 - 7, 0);

    piece1 = (new ModelPart(this, 0, 16))->setTexSize(64, 64);
    piece1->addBox(-5, -10, -5, 10, 10, 10, g - 0.5f); // upper body
    piece1->setPos(0, 0 + yOffset + 9, 0);

    piece2 = (new ModelPart(this, 0, 36))->setTexSize(64, 64);
    piece2->addBox(-6, -12, -6, 12, 12, 12, g - 0.5f); // lower body
    piece2->setPos(0, 0 + yOffset + 20, 0);  

	// 4J added - compile now to avoid random performance hit first time cubes are rendered
	head->compile(1.0f/16.0f);
	arm1->compile(1.0f/16.0f);
	arm2->compile(1.0f/16.0f);
	piece1->compile(1.0f/16.0f);
	piece2->compile(1.0f/16.0f);
}

void SnowManModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, unsigned int uiBitmaskOverrideAnim)
{
    Model::setupAnim(time, r, bob, yRot, xRot, scale);
	head->yRot = yRot / (float) (180 / PI);
	head->xRot = xRot / (float) (180 / PI);
    piece1->yRot = (yRot / (float) (180 / PI)) * 0.25f;

	float s = Mth::sin(piece1->yRot);
	float c = Mth::cos(piece1->yRot);

	arm1->zRot = 1;
	arm2->zRot = -1;
	arm1->yRot = 0 + piece1->yRot;
	arm2->yRot = PI + piece1->yRot;

	arm1->x = (c) * 5;
	arm1->z = (-s) * 5;
	
	arm2->x = (-c) * 5;
	arm2->z = (s) * 5;
}

void SnowManModel::render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled) 
{
	setupAnim(time, r, bob, yRot, xRot, scale);

	piece1->render(scale,usecompiled);
	piece2->render(scale,usecompiled);
	head->render(scale,usecompiled);
	arm1->render(scale,usecompiled);
	arm2->render(scale,usecompiled);
}


