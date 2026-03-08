#include "stdafx.h"
#include "../Minecraft.World/Mth.h"
#include "CreeperModel.h"
#include "ModelPart.h"

// 4J - added
void CreeperModel::_init(float g)
{
    head = new ModelPart(this, 0, 0);
    head->addBox(-4, - 8, -4, 8, 8, 8, g); // Head
    head->setPos(0, 6, 0);

    hair = new ModelPart(this, 32, 0);
    hair->addBox(-4, -8, -4, 8, 8, 8, g + 0.5f); // Head
    hair->setPos(0, 6, 0);

    body = new ModelPart(this, 16, 16);
    body->addBox(-4, 0, -2, 8, 12, 4, g); // Body
    body->setPos(0, 6, 0);

    leg0 = new ModelPart(this, 0, 16);
    leg0->addBox(-2, 0, -2, 4, 6, 4, g); // Leg0
    leg0->setPos(-2, 18, 4);

    leg1 = new ModelPart(this, 0, 16);
    leg1->addBox(-2, 0, -2, 4, 6, 4, g); // Leg1
    leg1->setPos(2, 18, 4);

    leg2 = new ModelPart(this, 0, 16);
    leg2->addBox(-2, 0, -2, 4, 6, 4, g); // Leg2
    leg2->setPos(-2, 18, -4);

    leg3 = new ModelPart(this, 0, 16);
    leg3->addBox(-2, 0, -2, 4, 6, 4, g); // Leg3
    leg3->setPos(2, 18, -4);

	// 4J added - compile now to avoid random performance hit first time cubes are rendered
	head->compile(1.0f/16.0f);
	hair->compile(1.0f/16.0f);
	body->compile(1.0f/16.0f);
	leg0->compile(1.0f/16.0f);
	leg1->compile(1.0f/16.0f);
	leg2->compile(1.0f/16.0f);
	leg3->compile(1.0f/16.0f);
}

CreeperModel::CreeperModel() : Model()
{
	_init(0);
}

CreeperModel::CreeperModel(float g) : Model()
{
	_init(g);
}

void CreeperModel::render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled)
{
    setupAnim(time, r, bob, yRot, xRot, scale, entity);

    head->render(scale, usecompiled);
    body->render(scale, usecompiled);
    leg0->render(scale, usecompiled);
    leg1->render(scale, usecompiled);
    leg2->render(scale, usecompiled);
    leg3->render(scale, usecompiled);
}

void CreeperModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim)
{
    head->yRot = yRot / (float) (180 / PI);
    head->xRot = xRot / (float) (180 / PI);

    leg0->xRot = (Mth::cos(time * 0.6662f) * 1.4f) * r;
    leg1->xRot = (Mth::cos(time * 0.6662f + PI) * 1.4f) * r;
    leg2->xRot = (Mth::cos(time * 0.6662f + PI) * 1.4f) * r;
    leg3->xRot = (Mth::cos(time * 0.6662f) * 1.4f) * r;
}