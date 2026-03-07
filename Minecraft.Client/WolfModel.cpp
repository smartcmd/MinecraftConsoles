#include "stdafx.h"
#include "WolfModel.h"
#include "../Minecraft.World/Wolf.h"
#include "../Minecraft.World/Mth.h"
#include "ModelPart.h"

WolfModel::WolfModel()
{
    float g = 0;

    float headHeight = 12 + 9.5f - legSize;

    head = new ModelPart(this, 0, 0);
    head->addBox(-3, -3, -2, 6, 6, 4, g); // Head
    head->setPos(-1, headHeight, -7);

    body = new ModelPart(this, 18, 14);
    body->addBox(-4, -2, -3, 6, 9, 6, g); // Body
    body->setPos(0, 11 + 11 - legSize, 2);

    upperBody = new ModelPart(this, 21, 0);
    upperBody->addBox(-4, -3, -3, 8, 6, 7, g); // Body
    upperBody->setPos(-1.0f, 11 + 11.0f - legSize, 2);

    leg0 = new ModelPart(this, 0, 18);
    leg0->addBox(-1, 0, -1, 2, legSize, 2, g); // Leg0
    leg0->setPos(-2.5f, 18 + 6 - legSize, 7);

    leg1 = new ModelPart(this, 0, 18);
    leg1->addBox(-1, 0, -1, 2, legSize, 2, g); // Leg1
    leg1->setPos(.5f, 18 + 6 - legSize, 7);

    leg2 = new ModelPart(this, 0, 18);
    leg2->addBox(-1, 0, -1, 2, legSize, 2, g); // Leg2
    leg2->setPos(-2.5f, 18 + 6 - legSize, -4);

    leg3 = new ModelPart(this, 0, 18);
    leg3->addBox(-1, 0, -1, 2, legSize, 2, g); // Leg3
    leg3->setPos(.5f, 18 + 6 - legSize, -4);

    tail = new ModelPart(this, 9, 18);
    tail->addBox(-1, 0, -1, 2, 8, 2, g);
    tail->setPos(-1, 2 + 18 - legSize, 8);

	head->texOffs(16, 14)->addBox(-3, -5, 0, 2, 2, 1, g);
	head->texOffs(16, 14)->addBox(1, -5, 0, 2, 2, 1, g);
	head->texOffs(0, 10)->addBox(-1.5f, 0, -5, 3, 3, 4, g);

	// 4J added - compile now to avoid random performance hit first time cubes are rendered
    head->compile(1.0f/16.0f);
    body->compile(1.0f/16.0f);
    upperBody->compile(1.0f/16.0f);
    leg0->compile(1.0f/16.0f);
    leg1->compile(1.0f/16.0f);
    leg2->compile(1.0f/16.0f);
    leg3->compile(1.0f/16.0f);
    tail->compile(1.0f/16.0f);
}

void WolfModel::render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled)
{
    Model::render(entity, time, r, bob, yRot, xRot, scale, usecompiled);
    setupAnim(time, r, bob, yRot, xRot, scale, entity);

	if (young) 
	{
		float ss = 2;
		glPushMatrix();
		glTranslatef(0, 5 * scale, 2 * scale);
		head->renderRollable(scale, usecompiled);
		glPopMatrix();
		glPushMatrix();
		glScalef(1 / ss, 1 / ss, 1 / ss);
		glTranslatef(0, 24 * scale, 0);
		body->render(scale, usecompiled);
		leg0->render(scale, usecompiled);
		leg1->render(scale, usecompiled);
		leg2->render(scale, usecompiled);
		leg3->render(scale, usecompiled);
		tail->renderRollable(scale, usecompiled);
		upperBody->render(scale, usecompiled);
		glPopMatrix();
	} 
	else 
	{
		head->renderRollable(scale, usecompiled);
		body->render(scale, usecompiled);
		leg0->render(scale, usecompiled);
		leg1->render(scale, usecompiled);
		leg2->render(scale, usecompiled);
		leg3->render(scale, usecompiled);
		tail->renderRollable(scale, usecompiled);
		upperBody->render(scale, usecompiled);
	}
}

void WolfModel::prepareMobModel(shared_ptr<LivingEntity> mob, float time, float r, float a)
{
    shared_ptr<Wolf> wolf = dynamic_pointer_cast<Wolf>(mob);

    if (wolf->isAngry())
	{
        tail->yRot = 0;
    }
	else
	{
        tail->yRot = (Mth::cos(time * 0.6662f) * 1.4f) * r;
    }

    if (wolf->isSitting())
	{
        upperBody->setPos(-1.0f, 11 + 13.0f - 8, -3);
        upperBody->xRot = .40f * PI;
        upperBody->yRot = .0f * PI;

        body->setPos(0, 11 + 15 - legSize, 0);
        body->xRot = .25f * PI;

        tail->setPos(-1, 11 + 18 - legSize, 6);

        leg0->setPos(-2.5f, 18 + 12 - legSize, 2);
        leg0->xRot = 1.5f * PI;
        leg1->setPos(.5f, 18 + 12 - legSize, 2);
        leg1->xRot = 1.5f * PI;

        leg2->xRot = 1.85f * PI;
        leg2->setPos(-2.49f, 18 + 7.0f - legSize, -4);
        leg3->xRot = 1.85f * PI;
        leg3->setPos(.51f, 18 + 7.0f - legSize, -4);
    }
	else
	{
        body->setPos(0, 11 + 11 - legSize, 2);
        body->xRot = 90 / (float) (180 / PI);

        upperBody->setPos(-1.0f, 11 + 11.0f - legSize, -3);
        upperBody->xRot = body->xRot;

        tail->setPos(-1, 2 + 18 - legSize, 8);

        leg0->setPos(-2.5f, 18 + 6 - legSize, 7);
        leg1->setPos(.5f, 18 + 6 - legSize, 7);
        leg2->setPos(-2.5f, 18 + 6 - legSize, -4);
        leg3->setPos(.5f, 18 + 6 - legSize, -4);


        leg0->xRot = (Mth::cos(time * 0.6662f) * 1.4f) * r;
        leg1->xRot = (Mth::cos(time * 0.6662f + PI) * 1.4f) * r;
        leg2->xRot = (Mth::cos(time * 0.6662f + PI) * 1.4f) * r;
        leg3->xRot = (Mth::cos(time * 0.6662f) * 1.4f) * r;
    }

    float angle = wolf->getHeadRollAngle(a) + wolf->getBodyRollAngle(a, 0);
    head->zRot = angle;

    upperBody->zRot = wolf->getBodyRollAngle(a, -.08f);
    body->zRot = wolf->getBodyRollAngle(a, -.16f);
    tail->zRot = wolf->getBodyRollAngle(a, -.2f);
}

void WolfModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim)
{
    Model::setupAnim(time, r, bob, yRot, xRot, scale, entity);
    head->xRot = xRot / (float) (180 / PI);
    head->yRot = yRot / (float) (180 / PI);
    tail->xRot = bob;
}