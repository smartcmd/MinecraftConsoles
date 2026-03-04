#include "stdafx.h"
#include "QuadrupedModel.h"
#include "..\Minecraft.World\Mth.h"
#include "ModelPart.h"

QuadrupedModel::QuadrupedModel(int legSize, float g) : Model()
{
	yHeadOffs = 8;
	zHeadOffs = 4;

	head = new ModelPart(this, 0, 0);
	head->addBox(-4, -4, -8, 8, 8, 8, g); // Head
	head->setPos(0, (float)(12 + 6 - legSize), -6);

	body = new ModelPart(this, 28, 8);
	body->addBox(-5, -10, -7, 10, 16, 8, g); // Body
	body->setPos(0, (float)(11 + 6 - legSize), 2);

	leg0 = new ModelPart(this, 0, 16);
	leg0->addBox(-2, 0, -2, 4, legSize, 4, g); // Leg0
	leg0->setPos(-3, (float)(18 + 6 - legSize), 7);

	leg1 = new ModelPart(this, 0, 16);
	leg1->addBox(-2, 0, -2, 4, legSize, 4, g); // Leg1
	leg1->setPos(3, (float)(18 + 6 - legSize), 7);

	leg2 = new ModelPart(this, 0, 16);
	leg2->addBox(-2, 0, -2, 4, legSize, 4, g); // Leg2
	leg2->setPos(-3, (float)(18 + 6 - legSize), -5);

	leg3 = new ModelPart(this, 0, 16);
	leg3->addBox(-2, 0, -2, 4, legSize, 4, g); // Leg3
	leg3->setPos(3, (float)(18 + 6 - legSize), -5);

	// 4J added - compile now to avoid random performance hit first time cubes are rendered
	head->compile(1.0f/16.0f);
    body->compile(1.0f/16.0f);
    leg0->compile(1.0f/16.0f);
    leg1->compile(1.0f/16.0f);
    leg2->compile(1.0f/16.0f);
    leg3->compile(1.0f/16.0f);
}

void QuadrupedModel::render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled)
{
    setupAnim(time, r, bob, yRot, xRot, scale, entity);

	if (young) 
	{
		float ss = 2.0f;
		glPushMatrix();
		glTranslatef(0, yHeadOffs * scale, zHeadOffs * scale);
		head->render(scale,usecompiled);
		glPopMatrix();
		glPushMatrix();
		glScalef(1 / ss, 1 / ss, 1 / ss);
		glTranslatef(0, 24 * scale, 0);
		body->render(scale, usecompiled);
		leg0->render(scale, usecompiled);
		leg1->render(scale, usecompiled);
		leg2->render(scale, usecompiled);
		leg3->render(scale, usecompiled);
		glPopMatrix();
	} 
	else 
	{
		head->render(scale, usecompiled);
		body->render(scale, usecompiled);
		leg0->render(scale, usecompiled);
		leg1->render(scale, usecompiled);
		leg2->render(scale, usecompiled);
		leg3->render(scale, usecompiled);
	}
}

void QuadrupedModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim)
{
	float rad = (float) (180 / PI);
    head->xRot = xRot / rad;
    head->yRot = yRot / rad;
    body->xRot = 90 / rad;

    leg0->xRot = (Mth::cos(time * 0.6662f) * 1.4f) * r;
    leg1->xRot = (Mth::cos(time * 0.6662f + PI) * 1.4f) * r;
    leg2->xRot = (Mth::cos(time * 0.6662f + PI) * 1.4f) * r;
    leg3->xRot = (Mth::cos(time * 0.6662f) * 1.4f) * r;
}

void QuadrupedModel::render(QuadrupedModel *model, float scale, bool usecompiled)
{
    head->yRot = model->head->yRot;
    head->xRot = model->head->xRot;

    head->y = model->head->y;
    head->x = model->head->x;

    body->yRot = model->body->yRot;
    body->xRot = model->body->xRot;

    leg0->xRot = model->leg0->xRot;
    leg1->xRot = model->leg1->xRot;
    leg2->xRot = model->leg2->xRot;
    leg3->xRot = model->leg3->xRot;

	if (young) 
	{
		float ss = 2.0f;
		glPushMatrix();
		glTranslatef(0, 8 * scale, 4 * scale);
		head->render(scale,usecompiled);
		glPopMatrix();
		glPushMatrix();
		glScalef(1 / ss, 1 / ss, 1 / ss);
		glTranslatef(0, 24 * scale, 0);
		body->render(scale, usecompiled);
		leg0->render(scale, usecompiled);
		leg1->render(scale, usecompiled);
		leg2->render(scale, usecompiled);
		leg3->render(scale, usecompiled);
		glPopMatrix();
	} 
	else 
	{
		head->render(scale, usecompiled);
		body->render(scale, usecompiled);
		leg0->render(scale, usecompiled);
		leg1->render(scale, usecompiled);
		leg2->render(scale, usecompiled);
		leg3->render(scale, usecompiled);
	}
}
