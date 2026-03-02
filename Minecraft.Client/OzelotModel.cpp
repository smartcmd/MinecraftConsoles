#include "stdafx.h"
#include "ModelPart.h"
#include "..\Minecraft.World\net.minecraft.world.entity.animal.h"
#include "..\Minecraft.World\Mth.h"
#include "OzelotModel.h"

const float OzelotModel::xo = 0;
const float OzelotModel::yo = 16;
const float OzelotModel::zo = -9;

const float OzelotModel::headWalkY = -1 + yo;
const float OzelotModel::headWalkZ = 0 + zo;
const float OzelotModel::bodyWalkY = -4 + yo;
const float OzelotModel::bodyWalkZ = -1 + zo;
const float OzelotModel::tail1WalkY = -1 + yo;
const float OzelotModel::tail1WalkZ = 17 + zo;
const float OzelotModel::tail2WalkY = 4 + yo;
const float OzelotModel::tail2WalkZ = 23 + zo;
const float OzelotModel::backLegY = 2.f + yo;
const float OzelotModel::backLegZ = 14 + zo;
const float OzelotModel::frontLegY = -2.2f + yo;
const float OzelotModel::frontLegZ = 4.f + zo;

OzelotModel::OzelotModel()
{
	state = WALK_STATE;

	setMapTex(L"head.main", 0, 0);
	setMapTex(L"head.nose", 0, 24);
	setMapTex(L"head.ear1", 0, 10);
	setMapTex(L"head.ear2", 6, 10);

	head = new ModelPart(this, L"head");
	head->addBox(L"main", -2.5f, -2, -3, 5, 4, 5);
	head->addBox(L"nose", -1.5f, 0, -4, 3, 2, 2);
	head->addBox(L"ear1", -2, -3, 0, 1, 1, 2);
	head->addBox(L"ear2", 1, -3, 0, 1, 1, 2);
	head->setPos(0 + xo, headWalkY, headWalkZ);

	body = new ModelPart(this, 20, 0);
	body->addBox(-2, 3, -8, 4, 16, 6, 0);
	body->setPos(0 + xo, bodyWalkY, bodyWalkZ);

	tail1 = new ModelPart(this, 0, 15);
	tail1->addBox(-0.5f, 0, 0, 1, 8, 1);
	tail1->xRot = 0.9f;
	tail1->setPos(0 + xo, tail1WalkY, tail1WalkZ);

	tail2 = new ModelPart(this, 4, 15);
	tail2->addBox(-0.5f, 0, 0, 1, 8, 1);
	tail2->setPos(0 + xo, tail2WalkY, tail2WalkZ);

	backLegL = new ModelPart(this, 8, 13);
	backLegL->addBox(-1, 0, 1, 2, 6, 2);
	backLegL->setPos(1.1f + xo, backLegY, backLegZ);

	backLegR = new ModelPart(this, 8, 13);
	backLegR->addBox(-1, 0, 1, 2, 6, 2);
	backLegR->setPos(-1.1f + xo, backLegY, backLegZ);

	frontLegL = new ModelPart(this, 40, 0);
	frontLegL->addBox(-1, 0, 0, 2, 10, 2);
	frontLegL->setPos(1.2f + xo, frontLegY, frontLegZ);

	frontLegR = new ModelPart(this, 40, 0);
	frontLegR->addBox(-1, 0, 0, 2, 10, 2);
	frontLegR->setPos(-1.2f + xo, frontLegY, frontLegZ);


	// 4J added - compile now to avoid random performance hit first time cubes are rendered
	head->compile(1.0f/16.0f);
	body->compile(1.0f/16.0f);
	tail1->compile(1.0f/16.0f);
	tail2->compile(1.0f/16.0f);
	backLegL->compile(1.0f/16.0f);
	backLegR->compile(1.0f/16.0f);
	backLegL->compile(1.0f/16.0f);
	backLegR->compile(1.0f/16.0f);
}

void OzelotModel::render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled)
{
	setupAnim(time, r, bob, yRot, xRot, scale);
	if (young)
	{
		float ss = 2.0f;
		glPushMatrix();
		glScalef(1.5f / ss, 1.5f / ss, 1.5f / ss);
		glTranslatef(0, 10 * scale, 4 * scale);
		head->render(scale, usecompiled);
		glPopMatrix();
		glPushMatrix();
		glScalef(1 / ss, 1 / ss, 1 / ss);
		glTranslatef(0, 24 * scale, 0);
		body->render(scale, usecompiled);
		backLegL->render(scale, usecompiled);
		backLegR->render(scale, usecompiled);
		frontLegL->render(scale, usecompiled);
		frontLegR->render(scale, usecompiled);
		tail1->render(scale, usecompiled);
		tail2->render(scale, usecompiled);
		glPopMatrix();
	}
	else
	{
		head->render(scale, usecompiled);
		body->render(scale, usecompiled);
		tail1->render(scale, usecompiled);
		tail2->render(scale, usecompiled);
		backLegL->render(scale, usecompiled);
		backLegR->render(scale, usecompiled);
		frontLegL->render(scale, usecompiled);
		frontLegR->render(scale, usecompiled);
	}
}

void OzelotModel::render(OzelotModel *model, float scale, bool usecompiled)
{
	head->yRot = model->head->yRot;
	head->xRot = model->head->xRot;
	head->y = model->head->y;
	head->x = model->head->x;
	body->yRot = model->body->yRot;
	body->xRot = model->body->xRot;

	tail1->yRot = model->body->yRot;
	tail1->y = model->body->y;
	tail1->x = model->body->x;
	tail1->render(scale, usecompiled);

	tail2->yRot = model->body->yRot;
	tail2->y = model->body->y;
	tail2->x = model->body->x;
	tail2->render(scale, usecompiled);

	backLegL->xRot = model->backLegL->xRot;
	backLegR->xRot = model->backLegR->xRot;
	backLegL->render(scale, usecompiled);
	backLegR->render(scale, usecompiled);

	frontLegL->xRot = model->frontLegL->xRot;
	frontLegR->xRot = model->frontLegR->xRot;
	frontLegL->render(scale, usecompiled);
	frontLegR->render(scale, usecompiled);

	head->render(scale, usecompiled);
	body->render(scale, usecompiled);
}

void OzelotModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, unsigned int uiBitmaskOverrideAnim)
{
	head->xRot = xRot / (float) (180 / PI);
	head->yRot = yRot / (float) (180 / PI);

	if (state == SITTING_STATE)
	{

	}
	else
	{
		body->xRot = 90 / (float) (180 / PI);
		if (state == SPRINT_STATE)
		{
			backLegL->xRot = ((float) Mth::cos(time * 0.6662f) * 1.f) * r;
			backLegR->xRot = ((float) Mth::cos(time * 0.6662f + 0.3f) * 1.f) * r;
			frontLegL->xRot = ((float) Mth::cos(time * 0.6662f + PI + 0.3f) * 1.f) * r;
			frontLegR->xRot = ((float) Mth::cos(time * 0.6662f + PI) * 1.f) * r;
			tail2->xRot = 0.55f * PI + 0.1f * PI * Mth::cos(time) * r;
		}
		else
		{
			backLegL->xRot = ((float) Mth::cos(time * 0.6662f) * 1.f) * r;
			backLegR->xRot = ((float) Mth::cos(time * 0.6662f + PI) * 1.f) * r;
			frontLegL->xRot = ((float) Mth::cos(time * 0.6662f + PI) * 1.f) * r;
			frontLegR->xRot = ((float) Mth::cos(time * 0.6662f) * 1.f) * r;

			if (state == WALK_STATE) tail2->xRot = 0.55f * PI + 0.25f * PI * Mth::cos(time) * r;
			else tail2->xRot = 0.55f * PI + 0.15f * PI * Mth::cos(time) * r;
		}
	}
}

void OzelotModel::prepareMobModel(shared_ptr<Mob> mob, float time, float r, float a)
{
	shared_ptr<Ozelot> ozelot = dynamic_pointer_cast<Ozelot>(mob);

	body->y = bodyWalkY;
	body->z = bodyWalkZ;
	head->y = headWalkY;
	head->z = headWalkZ;
	tail1->y = tail1WalkY;
	tail1->z = tail1WalkZ;
	tail2->y = tail2WalkY;
	tail2->z = tail2WalkZ;
	frontLegL->y = frontLegR->y = frontLegY;
	frontLegL->z = frontLegR->z = frontLegZ;
	backLegL->y = backLegR->y = backLegY;
	backLegL->z = backLegR->z = backLegZ;
	tail1->xRot = 0.9f;

	if (ozelot->isSneaking())
	{
		body->y += 1;
		head->y += 2;
		tail1->y += 1;
		tail2->y += -4;
		tail2->z += 2;
		tail1->xRot = 0.5f * PI;
		tail2->xRot = 0.5f * PI;
		state = SNEAK_STATE;
	}
	else if (ozelot->isSprinting())
	{
		tail2->y = tail1->y;
		tail2->z += 2;
		tail1->xRot = 0.5f * PI;
		tail2->xRot = 0.5f * PI;
		state = SPRINT_STATE;
	}
	else if (ozelot->isSitting())
	{
		body->xRot = 45 / (float) (180 / PI);
		body->y += -4;
		body->z += 5;
		head->y += -3.3f;
		head->z += 1;

		tail1->y += 8;
		tail1->z += -2;
		tail2->y += 2;
		tail2->z += -0.8f;
		tail1->xRot = PI * 0.55f;
		tail2->xRot = PI * 0.85f;

		frontLegL->xRot = frontLegR->xRot = -PI * 0.05f;
		frontLegL->y = frontLegR->y = frontLegY + 2;
		frontLegL->z = frontLegR->z = -7;

		backLegL->xRot = backLegR->xRot = -PI * 0.5f;
		backLegL->y = backLegR->y = backLegY + 3;
		backLegL->z = backLegR->z = backLegZ - 4;
		state = SITTING_STATE;
	}
	else
	{
		state = WALK_STATE;
	}
}