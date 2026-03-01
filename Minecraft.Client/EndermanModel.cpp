#include "stdafx.h"
#include "EndermanModel.h"
#include "ModelPart.h"

EndermanModel::EndermanModel() : HumanoidModel(0, -14, 64, 32)
{
	carrying = false;
	creepy = false;

	float yOffset = -14.0f;
	float g = 0;

	delete hair;
	hair = new ModelPart(this, 0, 16);
	hair->addBox(-4.0f, -8.0f, -4.0f, 8, 8, 8, g - 0.5f); // Head
	hair->setPos(0.0f, 0.0f + yOffset, 0.0f);

	delete body;
	body = new ModelPart(this, 32, 16);
	body->addBox(-4.0f, 0.0f, -2.0f, 8, 12, 4, g); // Body
	body->setPos(0.0f, 0.0f + yOffset, 0.0f);


	delete arm0;
	arm0 = new ModelPart(this, 56, 0);
	arm0->addBox(-1.0f, -2.0f, -1.0f, 2, 30, 2, g); // Arm0
	arm0->setPos(-3.0f, 2.0f + yOffset, 0.0f);


	delete arm1;
	arm1 = new ModelPart(this, 56, 0);
	arm1->bMirror = true;
	arm1->addBox(-1.0f, -2.0f, -1.0f, 2, 30, 2, g); // Arm1
	arm1->setPos(5.0f, 2.0f + yOffset, 0.0f);

	delete leg0;
	leg0 = new ModelPart(this, 56, 0);
	leg0->addBox(-1.0f, 0.0f, -1.0f, 2, 30, 2, g); // Leg0
	leg0->setPos(-2.0f, 12.0f + yOffset, 0.0f);

	delete leg1;
	leg1 = new ModelPart(this, 56, 0);
	leg1->bMirror = true;
	leg1->addBox(-1.0f, 0.0f, -1.0f, 2, 30, 2, g); // Leg1
	leg1->setPos(2.0f, 12.0f + yOffset, 0.0f);

	// 4J added - compile now to avoid random performance hit first time cubes are rendered
	body->compile(1.0f/16.0f);
	arm0->compile(1.0f/16.0f);
	arm1->compile(1.0f/16.0f);
	leg0->compile(1.0f/16.0f);
	leg1->compile(1.0f/16.0f);
	hair->compile(1.0f/16.0f);
}

void EndermanModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim)
{
	HumanoidModel::setupAnim(time, r, bob, yRot, xRot, scale, entity, uiBitmaskOverrideAnim);

	head->visible = true;

	float yOffs = -14.0f;
	body->xRot = 0.0f;
	body->y = yOffs;
	body->z = -0.0f;

	leg0->xRot -= 0.0f;
	leg1->xRot -= 0.0f;

	arm0->xRot *= 0.5f;
	arm1->xRot *= 0.5f;
	leg0->xRot *= 0.5f;
	leg1->xRot *= 0.5f;

	float max = 0.4f;
	if (arm0->xRot > +max) arm0->xRot = +max;
	if (arm1->xRot > +max) arm1->xRot = +max;
	if (arm0->xRot < -max) arm0->xRot = -max;
	if (arm1->xRot < -max) arm1->xRot = -max;
	if (leg0->xRot > +max) leg0->xRot = +max;
	if (leg1->xRot > +max) leg1->xRot = +max;
	if (leg0->xRot < -max) leg0->xRot = -max;
	if (leg1->xRot < -max) leg1->xRot = -max;


	if (carrying)
	{
		arm0->xRot = -0.5f;
		arm1->xRot = -0.5f;
		arm0->zRot = 0.05f;
		arm1->zRot = -0.05f;
	}

	arm0->z = -0.0f;
	arm1->z = -0.0f;
	leg0->z = -0.0f;
	leg1->z = -0.0f;

	arm0->y = 2.0f + yOffs;
	arm1->y = 2.0f + yOffs;

	leg0->y = +9.0f + yOffs;
	leg1->y = +9.0f + yOffs;

	head->z = -0.0f;
	head->y = +yOffs + 1;

	hair->x = head->x;
	hair->y = head->y;
	hair->z = head->z;
	hair->xRot = head->xRot;
	hair->yRot = head->yRot;
	hair->zRot = head->zRot;

	if (creepy)
	{
		float amt = 1;
		head->y -= (float) (amt * 5);
	}
}