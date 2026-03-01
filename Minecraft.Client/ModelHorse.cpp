#include "stdafx.h"
#include "..\Minecraft.World\Mth.h"
#include "..\Minecraft.World\net.minecraft.world.entity.animal.h"
#include "ModelHorse.h"
#include "ModelPart.h"

ModelHorse::ModelHorse()
{
	texWidth = 128;
	texHeight = 128;

	// TODO: All rotation magic numbers in this method
	Body = new ModelPart(this, 0, 34);
	Body->addBox(-5.f, -8.f, -19.f, 10, 10, 24);
	Body->setPos(0.f, 11.f, 9.f);

	TailA = new ModelPart(this, 44, 0);
	TailA->addBox(-1.f, -1.f, 0.f, 2, 2, 3);
	TailA->setPos(0.f, 3.f, 14.f);
	setRotation(TailA, -1.134464f, 0.f, 0.f);

	TailB = new ModelPart(this, 38, 7);
	TailB->addBox(-1.5f, -2.f, 3.f, 3, 4, 7);
	TailB->setPos(0.f, 3.f, 14.f);
	setRotation(TailB, -1.134464f, 0.f, 0.f);

	TailC = new ModelPart(this, 24, 3);
	TailC->addBox(-1.5f, -4.5f, 9.f, 3, 4, 7);
	TailC->setPos(0.f, 3.f, 14.f);
	setRotation(TailC, -1.40215f, 0.f, 0.f);

	Leg1A = new ModelPart(this, 78, 29);
	Leg1A->addBox(-2.5f, -2.f, -2.5f, 4, 9, 5);
	Leg1A->setPos(4.f, 9.f, 11.f);

	Leg1B = new ModelPart(this, 78, 43);
	Leg1B->addBox(-2.f, 0.f, -1.5f, 3, 5, 3);
	Leg1B->setPos(4.f, 16.f, 11.f);

	Leg1C = new ModelPart(this, 78, 51);
	Leg1C->addBox(-2.5f, 5.1f, -2.f, 4, 3, 4);
	Leg1C->setPos(4.f, 16.f, 11.f);

	Leg2A = new ModelPart(this, 96, 29);
	Leg2A->addBox(-1.5f, -2.f, -2.5f, 4, 9, 5);
	Leg2A->setPos(-4.f, 9.f, 11.f);

	Leg2B = new ModelPart(this, 96, 43);
	Leg2B->addBox(-1.f, 0.f, -1.5f, 3, 5, 3);
	Leg2B->setPos(-4.f, 16.f, 11.f);

	Leg2C = new ModelPart(this, 96, 51);
	Leg2C->addBox(-1.5f, 5.1f, -2.f, 4, 3, 4);
	Leg2C->setPos(-4.f, 16.f, 11.f);

	Leg3A = new ModelPart(this, 44, 29);
	Leg3A->addBox(-1.9f, -1.f, -2.1f, 3, 8, 4);
	Leg3A->setPos(4.f, 9.f, -8.f);

	Leg3B = new ModelPart(this, 44, 41);
	Leg3B->addBox(-1.9f, 0.f, -1.6f, 3, 5, 3);
	Leg3B->setPos(4.f, 16.f, -8.f);

	Leg3C = new ModelPart(this, 44, 51);
	Leg3C->addBox(-2.4f, 5.1f, -2.1f, 4, 3, 4);
	Leg3C->setPos(4.f, 16.f, -8.f);

	Leg4A = new ModelPart(this, 60, 29);
	Leg4A->addBox(-1.1f, -1.f, -2.1f, 3, 8, 4);
	Leg4A->setPos(-4.f, 9.f, -8.f);

	Leg4B = new ModelPart(this, 60, 41);
	Leg4B->addBox(-1.1f, 0.f, -1.6f, 3, 5, 3);
	Leg4B->setPos(-4.f, 16.f, -8.f);

	Leg4C = new ModelPart(this, 60, 51);
	Leg4C->addBox(-1.6f, 5.1f, -2.1f, 4, 3, 4);
	Leg4C->setPos(-4.f, 16.f, -8.f);

	Head = new ModelPart(this, 0, 0);
	Head->addBox(-2.5f, -10.f, -1.5f, 5, 5, 7);
	Head->setPos(0.f, 4.f, -10.f);
	setRotation(Head, 0.5235988f, 0.f, 0.f);

	UMouth = new ModelPart(this, 24, 18);
	UMouth->addBox(-2.f, -10.f, -7.f, 4, 3, 6);
	UMouth->setPos(0.f, 3.95f, -10.f);
	setRotation(UMouth, 0.5235988f, 0.f, 0.f);

	LMouth = new ModelPart(this, 24, 27);
	LMouth->addBox(-2.f, -7.f, -6.5f, 4, 2, 5);
	LMouth->setPos(0.f, 4.f, -10.f);
	setRotation(LMouth, 0.5235988f, 0.f, 0.f);

	Head->addChild(UMouth);
	Head->addChild(LMouth);

	Ear1 = new ModelPart(this, 0, 0);
	Ear1->addBox(0.45f, -12.f, 4.f, 2, 3, 1);
	Ear1->setPos(0.f, 4.f, -10.f);
	setRotation(Ear1, 0.5235988f, 0.f, 0.f);

	Ear2 = new ModelPart(this, 0, 0);
	Ear2->addBox(-2.45f, -12.f, 4.f, 2, 3, 1);
	Ear2->setPos(0.f, 4.f, -10.f);
	setRotation(Ear2, 0.5235988f, 0.f, 0.f);

	MuleEarL = new ModelPart(this, 0, 12);
	MuleEarL->addBox(-2.f, -16.f, 4.f, 2, 7, 1);
	MuleEarL->setPos(0.f, 4.f, -10.f);
	setRotation(MuleEarL, 0.5235988f, 0.f, 0.2617994f);

	MuleEarR = new ModelPart(this, 0, 12);
	MuleEarR->addBox(0.f, -16.f, 4.f, 2, 7, 1);
	MuleEarR->setPos(0.f, 4.f, -10.f);
	setRotation(MuleEarR, 0.5235988f, 0.f, -0.2617994f);

	Neck = new ModelPart(this, 0, 12);
	Neck->addBox(-2.05f, -9.8f, -2.f, 4, 14, 8);
	Neck->setPos(0.f, 4.f, -10.f);
	setRotation(Neck, 0.5235988f, 0.f, 0.f);

	Bag1 = new ModelPart(this, 0, 34);
	Bag1->addBox(-3.f, 0.f, 0.f, 8, 8, 3);
	Bag1->setPos(-7.5f, 3.f, 10.f);
	setRotation(Bag1, 0.f, 1.570796f, 0.f);

	Bag2 = new ModelPart(this, 0, 47);
	Bag2->addBox(-3.f, 0.f, 0.f, 8, 8, 3);
	Bag2->setPos(4.5f, 3.f, 10.f);
	setRotation(Bag2, 0.f, 1.570796f, 0.f);

	Saddle = new ModelPart(this, 80, 0);
	Saddle->addBox(-5.f, 0.f, -3.f, 10, 1, 8);
	Saddle->setPos(0.f, 2.f, 2.f);

	SaddleB = new ModelPart(this, 106, 9);
	SaddleB->addBox(-1.5f, -1.f, -3.f, 3, 1, 2);
	SaddleB->setPos(0.f, 2.f, 2.f);

	SaddleC = new ModelPart(this, 80, 9);
	SaddleC->addBox(-4.f, -1.f, 3.f, 8, 1, 2);
	SaddleC->setPos(0.f, 2.f, 2.f);

	SaddleL2 = new ModelPart(this, 74, 0);
	SaddleL2->addBox(-0.5f, 6.f, -1.f, 1, 2, 2);
	SaddleL2->setPos(5.f, 3.f, 2.f);

	SaddleL = new ModelPart(this, 70, 0);
	SaddleL->addBox(-0.5f, 0.f, -0.5f, 1, 6, 1);
	SaddleL->setPos(5.f, 3.f, 2.f);

	SaddleR2 = new ModelPart(this, 74, 4);
	SaddleR2->addBox(-0.5f, 6.f, -1.f, 1, 2, 2);
	SaddleR2->setPos(-5.f, 3.f, 2.f);

	SaddleR = new ModelPart(this, 80, 0);
	SaddleR->addBox(-0.5f, 0.f, -0.5f, 1, 6, 1);
	SaddleR->setPos(-5.f, 3.f, 2.f);

	SaddleMouthL = new ModelPart(this, 74, 13);
	SaddleMouthL->addBox(1.5f, -8.f, -4.f, 1, 2, 2);
	SaddleMouthL->setPos(0.f, 4.f, -10.f);
	setRotation(SaddleMouthL, 0.5235988f, 0.f, 0.f);

	SaddleMouthR = new ModelPart(this, 74, 13);
	SaddleMouthR->addBox(-2.5f, -8.f, -4.f, 1, 2, 2);
	SaddleMouthR->setPos(0.f, 4.f, -10.f);
	setRotation(SaddleMouthR, 0.5235988f, 0.f, 0.f);

	SaddleMouthLine = new ModelPart(this, 44, 10);
	SaddleMouthLine->addBox(2.6f, -6.f, -6.f, 0, 3, 16);
	SaddleMouthLine->setPos(0.f, 4.f, -10.f);

	SaddleMouthLineR = new ModelPart(this, 44, 5);
	SaddleMouthLineR->addBox(-2.6f, -6.f, -6.f, 0, 3, 16);
	SaddleMouthLineR->setPos(0.f, 4.f, -10.f);

	Mane = new ModelPart(this, 58, 0);
	Mane->addBox(-1.f, -11.5f, 5.f, 2, 16, 4);
	Mane->setPos(0.f, 4.f, -10.f);
	setRotation(Mane, 0.5235988f, 0.f, 0.f);

	HeadSaddle = new ModelPart(this, 80, 12);
	HeadSaddle->addBox(-2.5f, -10.1f, -7.f, 5, 5, 12, 0.2f);
	HeadSaddle->setPos(0.f, 4.f, -10.f);
	setRotation(HeadSaddle, 0.5235988f, 0.f, 0.f);

	// 4J added - compile now to avoid random performance hit first time cubes are rendered
	Head->compile(1.0f/16.0f);;
	UMouth->compile(1.0f/16.0f);;
	LMouth->compile(1.0f/16.0f);;
	Ear1->compile(1.0f/16.0f);;
	Ear2->compile(1.0f/16.0f);;
	MuleEarL->compile(1.0f/16.0f);;
	MuleEarR->compile(1.0f/16.0f);;
	Neck->compile(1.0f/16.0f);;
	HeadSaddle->compile(1.0f/16.0f);;
	Mane->compile(1.0f/16.0f);;

	Body->compile(1.0f/16.0f);;
	TailA->compile(1.0f/16.0f);;
	TailB->compile(1.0f/16.0f);;
	TailC->compile(1.0f/16.0f);;

	Leg1A->compile(1.0f/16.0f);;
	Leg1B->compile(1.0f/16.0f);;
	Leg1C->compile(1.0f/16.0f);;

	Leg2A->compile(1.0f/16.0f);;
	Leg2B->compile(1.0f/16.0f);;
	Leg2C->compile(1.0f/16.0f);;

	Leg3A->compile(1.0f/16.0f);;
	Leg3B->compile(1.0f/16.0f);;
	Leg3C->compile(1.0f/16.0f);;

	Leg4A->compile(1.0f/16.0f);;
	Leg4B->compile(1.0f/16.0f);;
	Leg4C->compile(1.0f/16.0f);;

	Bag1->compile(1.0f/16.0f);;
	Bag2->compile(1.0f/16.0f);;

	Saddle->compile(1.0f/16.0f);;
	SaddleB->compile(1.0f/16.0f);;
	SaddleC->compile(1.0f/16.0f);;

	SaddleL->compile(1.0f/16.0f);;
	SaddleL2->compile(1.0f/16.0f);;

	SaddleR->compile(1.0f/16.0f);;
	SaddleR2->compile(1.0f/16.0f);;

	SaddleMouthL->compile(1.0f/16.0f);;
	SaddleMouthR->compile(1.0f/16.0f);;

	SaddleMouthLine->compile(1.0f/16.0f);;
	SaddleMouthLineR->compile(1.0f/16.0f);;
}


void ModelHorse::render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled)
{
	shared_ptr<EntityHorse> entityhorse = dynamic_pointer_cast<EntityHorse>(entity);

	int type = entityhorse->getType();
	float eating = entityhorse->getEatAnim(0);
	bool adult = (entityhorse->isAdult());
	bool saddled = adult && entityhorse->isSaddled();
	bool chested = adult && entityhorse->isChestedHorse();
	bool largeEars = type == EntityHorse::TYPE_DONKEY || type == EntityHorse::TYPE_MULE;
	float sizeFactor = entityhorse->getFoalScale();

	bool rider = (entityhorse->rider.lock() != NULL);

	if (saddled)
	{
		HeadSaddle->render(scale, usecompiled);
		Saddle->render(scale, usecompiled);
		SaddleB->render(scale, usecompiled);
		SaddleC->render(scale, usecompiled);
		SaddleL->render(scale, usecompiled);
		SaddleL2->render(scale, usecompiled);
		SaddleR->render(scale, usecompiled);
		SaddleR2->render(scale, usecompiled);
		SaddleMouthL->render(scale, usecompiled);
		SaddleMouthR->render(scale, usecompiled);

		if (rider)
		{
			SaddleMouthLine->render(scale, usecompiled);
			SaddleMouthLineR->render(scale, usecompiled);
		}
	}

	// render legs
	if (!adult)
	{
		glPushMatrix();
		glScalef(sizeFactor, .5f + sizeFactor * .5f, sizeFactor);
		glTranslatef(0, .95f * (1.0f - sizeFactor), 0);
	}
	Leg1A->render(scale, usecompiled);
	Leg1B->render(scale, usecompiled);
	Leg1C->render(scale, usecompiled);

	Leg2A->render(scale, usecompiled);
	Leg2B->render(scale, usecompiled);
	Leg2C->render(scale, usecompiled);

	Leg3A->render(scale, usecompiled);
	Leg3B->render(scale, usecompiled);
	Leg3C->render(scale, usecompiled);

	Leg4A->render(scale, usecompiled);
	Leg4B->render(scale, usecompiled);
	Leg4C->render(scale, usecompiled);
	if (!adult)
	{
		glPopMatrix();

		glPushMatrix();
		glScalef(sizeFactor, sizeFactor, sizeFactor);
		glTranslatef(0, 1.35f * (1.0f - sizeFactor), 0);
	}
	// render body
	Body->render(scale, usecompiled);
	TailA->render(scale, usecompiled);
	TailB->render(scale, usecompiled);
	TailC->render(scale, usecompiled);
	Neck->render(scale, usecompiled);
	Mane->render(scale, usecompiled);
	if (!adult)
	{
		glPopMatrix();

		glPushMatrix();
		float headScale = .5f + (sizeFactor * sizeFactor) * .5f;
		glScalef(headScale, headScale, headScale);
		if (eating <= 0)
		{
			glTranslatef(0, 1.35f * (1.0f - sizeFactor), 0);
		}
		else
		{
			glTranslatef(0, .9f * (1.0f - sizeFactor) * eating + (1.35f * (1.0f - sizeFactor)) * (1.0f - eating), .15f * (1.0f - sizeFactor) * eating);
		}
	}
	// render head
	if (largeEars)
	{
		MuleEarL->render(scale, usecompiled);
		MuleEarR->render(scale, usecompiled);
	}
	else
	{
		Ear1->render(scale, usecompiled);
		Ear2->render(scale, usecompiled);
	}
	Head->render(scale, usecompiled);
	if (!adult)
	{
		glPopMatrix();
	}
	if (chested)
	{
		Bag1->render(scale, usecompiled);
		Bag2->render(scale, usecompiled);
	}
}

void ModelHorse::setRotation(ModelPart *model, float x, float y, float z)
{
	model->xRot = x;
	model->yRot = y;
	model->zRot = z;
}

float ModelHorse::rotlerp(float from, float to, float a)
{
	float diff = to - from;
	while (diff < -180)
		diff += 360;
	while (diff >= 180)
		diff -= 360;
	return from + a * diff;
}

void ModelHorse::prepareMobModel(shared_ptr<LivingEntity> mob, float wp, float ws, float a)
{
	Model::prepareMobModel(mob, wp, ws, a);

	float bodyRot = rotlerp(mob->yBodyRotO, mob->yBodyRot, a);
	float headRot = rotlerp(mob->yHeadRotO, mob->yHeadRot, a);
	float headRotx = (mob->xRotO + (mob->xRot - mob->xRotO) * a);
	float headRotMinusBodyRot = headRot - bodyRot;

	// TODO: Magic numbers
	float HeadXRot = (headRotx / 57.29578f);
	if (headRotMinusBodyRot > 20.f) {
		headRotMinusBodyRot = 20.f;
	}
	if (headRotMinusBodyRot < -20.f) {
		headRotMinusBodyRot = -20.f;
	}

	/**
	* f = distance walked f1 = speed 0 - 1 f2 = timer
	*/
	if (ws > 0.2f)
	{
		HeadXRot = HeadXRot + (cos(wp * 0.4f) * 0.15f * ws);
	}

	shared_ptr<EntityHorse> entityhorse = dynamic_pointer_cast<EntityHorse>(mob);

	float eating = entityhorse->getEatAnim(a);
	float standing = entityhorse->getStandAnim(a);
	float iStanding = 1.0f - standing;
	float openMouth = entityhorse->getMouthAnim(a);
	bool tail = entityhorse->tailCounter != 0;
	bool saddled = entityhorse->isSaddled();
	bool rider = entityhorse->rider.lock() != NULL;
	float bob = mob->tickCount + a;

	float legAnim1 = cos((wp * 0.6662f) + 3.141593f);
	float legXRotAnim = legAnim1 * 0.8f * ws;

	Head->y = 4.0f;
	Head->z = -10.f;
	TailA->y = 3.f;
	TailB->z = 14.f;
	Bag2->y = 3.f;
	Bag2->z = 10.f;
	Body->xRot = 0.f;

	// TODO: Fix these magical numbers
	Head->xRot = 0.5235988f + (HeadXRot);
	Head->yRot = (headRotMinusBodyRot / 57.29578f);// fixes SMP bug

	// interpolate positions and rotations based on current eating and standing animations
	{
		// TODO: Magic numbers
		Head->xRot = standing * ((15 * Mth::DEGRAD) + (HeadXRot)) + eating * 2.18166f + (1.0f - max(standing, eating)) * Head->xRot;
		Head->yRot = standing * (headRotMinusBodyRot / 57.29578f) + (1.0f - max(standing, eating)) * Head->yRot;

		Head->y = standing * -6.f + eating * 11.0f + (1.0f - max(standing, eating)) * Head->y;
		Head->z = standing * -1.f + eating * -10.f + (1.0f - max(standing, eating)) * Head->z;

		TailA->y = standing * 9.f + iStanding * TailA->y;
		TailB->z = standing * 18.f + iStanding * TailB->z;
		Bag2->y = standing * 5.5f + iStanding * Bag2->y;
		Bag2->z = standing * 15.f + iStanding * Bag2->z;
		Body->xRot = standing * (-45 / 57.29578f) + iStanding * Body->xRot;
	}

	Ear1->y = Head->y;
	Ear2->y = Head->y;
	MuleEarL->y = Head->y;
	MuleEarR->y = Head->y;
	Neck->y = Head->y;
	UMouth->y = 0 + .02f;
	LMouth->y = 0;
	Mane->y = Head->y;

	Ear1->z = Head->z;
	Ear2->z = Head->z;
	MuleEarL->z = Head->z;
	MuleEarR->z = Head->z;
	Neck->z = Head->z;
	UMouth->z = 0 + .02f - openMouth * 1;
	LMouth->z = 0 + openMouth * 1;
	Mane->z = Head->z;

	Ear1->xRot = Head->xRot;
	Ear2->xRot = Head->xRot;
	MuleEarL->xRot = Head->xRot;
	MuleEarR->xRot = Head->xRot;
	Neck->xRot = Head->xRot;
	UMouth->xRot = 0 - (PI * .03f) * openMouth;
	LMouth->xRot = 0 + (PI * .05f) * openMouth;

	Mane->xRot = Head->xRot;

	Ear1->yRot = Head->yRot;
	Ear2->yRot = Head->yRot;
	MuleEarL->yRot = Head->yRot;
	MuleEarR->yRot = Head->yRot;
	Neck->yRot = Head->yRot;
	UMouth->yRot = 0;
	LMouth->yRot = 0;
	Mane->yRot = Head->yRot;

	// (if chested)
	Bag1->xRot = legXRotAnim / 5.f;
	Bag2->xRot = -legXRotAnim / 5.f;

	/**
	* knee joints Leg1 and Leg4 use LLegXRot Leg2 and Leg3 use RLegXRot
	*/
	{
		float r90 = PI * .5f;
		float r270 = PI * 1.5f;
		float r300 = -60 * Mth::DEGRAD;
		float standAngle = 15 * Mth::DEGRAD * standing;
		float bobValue = Mth::cos((bob * 0.6f) + 3.141593f);

		Leg3A->y = -2.f * standing + 9.f * iStanding;
		Leg3A->z = -2.f * standing + -8.f * iStanding;
		Leg4A->y = Leg3A->y;
		Leg4A->z = Leg3A->z;

		Leg1B->y = Leg1A->y + (Mth::sin(r90 + standAngle + iStanding * (-legAnim1 * 0.5f * ws)) * 7.f);
		Leg1B->z = Leg1A->z + (Mth::cos(r270 + standAngle + iStanding * (-legAnim1 * 0.5f * ws)) * 7.f);

		Leg2B->y = Leg2A->y + (Mth::sin(r90 + standAngle + iStanding * (legAnim1 * 0.5f * ws)) * 7.f);
		Leg2B->z = Leg2A->z + (Mth::cos(r270 + standAngle + iStanding * (legAnim1 * 0.5f * ws)) * 7.f);

		float rlegRot = (r300 + bobValue) * standing + legXRotAnim * iStanding;
		float llegRot = (r300 + -bobValue) * standing + -legXRotAnim * iStanding;
		Leg3B->y = Leg3A->y + (Mth::sin(r90 + rlegRot) * 7.f);
		Leg3B->z = Leg3A->z + (Mth::cos(r270 + rlegRot) * 7.f);

		Leg4B->y = Leg4A->y + (Mth::sin(r90 + llegRot) * 7.f);
		Leg4B->z = Leg4A->z + (Mth::cos(r270 + llegRot) * 7.f);

		Leg1A->xRot = standAngle + (-legAnim1 * 0.5f * ws) * iStanding;
		Leg1B->xRot = (-5 * Mth::DEGRAD) * standing + ((-legAnim1 * 0.5f * ws) - max(0.0f, legAnim1 * .5f * ws)) * iStanding;
		Leg1C->xRot = Leg1B->xRot;

		Leg2A->xRot = standAngle + (legAnim1 * 0.5f * ws) * iStanding;
		Leg2B->xRot = (-5 * Mth::DEGRAD) * standing + ((legAnim1 * 0.5f * ws) - max(0.0f, -legAnim1 * .5f * ws)) * iStanding;
		Leg2C->xRot = Leg2B->xRot;

		Leg3A->xRot = rlegRot;
		Leg3B->xRot = (Leg3A->xRot + PI * max(0.0f, (.2f + bobValue * .2f))) * standing + (legXRotAnim + max(0.0f, legAnim1 * 0.5f * ws)) * iStanding;
		Leg3C->xRot = Leg3B->xRot;

		Leg4A->xRot = llegRot;
		Leg4B->xRot = (Leg4A->xRot + PI * max(0.0f, (.2f - bobValue * .2f))) * standing + (-legXRotAnim + max(0.0f, -legAnim1 * 0.5f * ws)) * iStanding;
		Leg4C->xRot = Leg4B->xRot;
	}

	Leg1C->y = Leg1B->y;
	Leg1C->z = Leg1B->z;
	Leg2C->y = Leg2B->y;
	Leg2C->z = Leg2B->z;
	Leg3C->y = Leg3B->y;
	Leg3C->z = Leg3B->z;
	Leg4C->y = Leg4B->y;
	Leg4C->z = Leg4B->z;

	if (saddled)
	{

		Saddle->y = standing * .5f + iStanding * 2.f;
		Saddle->z = standing * 11.f + iStanding * 2.f;

		SaddleB->y = Saddle->y;
		SaddleC->y = Saddle->y;
		SaddleL->y = Saddle->y;
		SaddleR->y = Saddle->y;
		SaddleL2->y = Saddle->y;
		SaddleR2->y = Saddle->y;
		Bag1->y = Bag2->y;

		SaddleB->z = Saddle->z;
		SaddleC->z = Saddle->z;
		SaddleL->z = Saddle->z;
		SaddleR->z = Saddle->z;
		SaddleL2->z = Saddle->z;
		SaddleR2->z = Saddle->z;
		Bag1->z = Bag2->z;

		Saddle->xRot = Body->xRot;
		SaddleB->xRot = Body->xRot;
		SaddleC->xRot = Body->xRot;

		SaddleMouthLine->y = Head->y;
		SaddleMouthLineR->y = Head->y;
		HeadSaddle->y = Head->y;
		SaddleMouthL->y = Head->y;
		SaddleMouthR->y = Head->y;

		SaddleMouthLine->z = Head->z;
		SaddleMouthLineR->z = Head->z;
		HeadSaddle->z = Head->z;
		SaddleMouthL->z = Head->z;
		SaddleMouthR->z = Head->z;

		SaddleMouthLine->xRot = HeadXRot;
		SaddleMouthLineR->xRot = HeadXRot;
		HeadSaddle->xRot = Head->xRot;
		SaddleMouthL->xRot = Head->xRot;
		SaddleMouthR->xRot = Head->xRot;
		HeadSaddle->yRot = Head->yRot;
		SaddleMouthL->yRot = Head->yRot;
		SaddleMouthLine->yRot = Head->yRot;
		SaddleMouthR->yRot = Head->yRot;
		SaddleMouthLineR->yRot = Head->yRot;

		if (rider) {
			// TODO: Magic number (smells like radians :D)
			SaddleL->xRot = -60 / 57.29578f;
			SaddleL2->xRot = -60 / 57.29578f;
			SaddleR->xRot = -60 / 57.29578f;
			SaddleR2->xRot = -60 / 57.29578f;

			SaddleL->zRot = 0.f;
			SaddleL2->zRot = 0.f;
			SaddleR->zRot = 0.f;
			SaddleR2->zRot = 0.f;
		} else {
			SaddleL->xRot = legXRotAnim / 3.f;
			SaddleL2->xRot = legXRotAnim / 3.f;
			SaddleR->xRot = legXRotAnim / 3.f;
			SaddleR2->xRot = legXRotAnim / 3.f;

			SaddleL->zRot = legXRotAnim / 5.f;
			SaddleL2->zRot = legXRotAnim / 5.f;
			SaddleR->zRot = -legXRotAnim / 5.f;
			SaddleR2->zRot = -legXRotAnim / 5.f;
		}
	}

	// TODO: Magic number
	float tailMov = -1.3089f + (ws * 1.5f);
	if (tailMov > 0)
	{
		tailMov = 0;
	}

	if (tail)
	{
		TailA->yRot = Mth::cos(bob * 0.7f);
		tailMov = 0;
	}
	else
	{
		TailA->yRot = 0.f;
	}
	TailB->yRot = TailA->yRot;
	TailC->yRot = TailA->yRot;

	TailB->y = TailA->y;
	TailC->y = TailA->y;
	TailB->z = TailA->z;
	TailC->z = TailA->z;

	// TODO: Magic number
	TailA->xRot = tailMov;
	TailB->xRot = tailMov;
	TailC->xRot = -0.2618f + tailMov;
}