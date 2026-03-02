#include "stdafx.h"
#include "VillagerModel.h"
#include "..\Minecraft.World\Mth.h"
#include "ModelPart.h"


void VillagerModel::_init(float g, float yOffset)
{
	int xTexSize = 64;
	int yTexSize = 64;

	head = (new ModelPart(this))->setTexSize(xTexSize, yTexSize);
	head->setPos(0, 0 + yOffset, 0);
	head->texOffs(0, 0)->addBox(-4, -10, -4, 8, 10, 8, g);
	//        head.texOffs(32, 0).addBox(-4, -10, -4, 8, 12, 8, g + 0.5f);
	head->texOffs(24, 0)->addBox(-1, -3, -6, 2, 4, 2, g);

	body = (new ModelPart(this))->setTexSize(xTexSize, yTexSize);
	body->setPos(0, 0 + yOffset, 0);
	body->texOffs(16, 20)->addBox(-4, 0, -3, 8, 12, 6, g);
	body->texOffs(0, 38)->addBox(-4, 0, -3, 8, 18, 6, g + 0.5f);

	arms = (new ModelPart(this))->setTexSize(xTexSize, yTexSize);
	arms->setPos(0, 0 + yOffset + 2, 0);
	arms->texOffs(44, 22)->addBox(-6 - 2, -2, -2, 4, 8, 4, g);
	arms->texOffs(44, 22)->addBox(6 - 2, -2, -2, 4, 8, 4, g);
	arms->texOffs(40, 38)->addBox(-4, 2, -2, 8, 4, 4, g);

	leg0 = (new ModelPart(this, 0, 22))->setTexSize(xTexSize, yTexSize);
	leg0->setPos(-2, 12 + yOffset, 0);
	leg0->addBox(-2, 0, -2, 4, 12, 4, g); // Leg0

	leg1 = (new ModelPart(this, 0, 22))->setTexSize(xTexSize, yTexSize);
	leg1->bMirror = true;
	leg1->setPos(2, 12 + yOffset, 0);
	leg1->addBox(-2, 0, -2, 4, 12, 4, g); // Leg1

	// 4J added - compile now to avoid random performance hit first time cubes are rendered
	// 4J Stu - Not just performance, but alpha+depth tests don't work right unless we compile here

	head->compile(1.0f/16.0f);
	body->compile(1.0f/16.0f);
	arms->compile(1.0f/16.0f);
	leg0->compile(1.0f/16.0f);
	leg1->compile(1.0f/16.0f);
}

VillagerModel::VillagerModel(float g) : Model()
{
	_init(g, 0);
}

VillagerModel::VillagerModel(float g, float yOffset) : Model()
{
	_init(g,yOffset);
}

void VillagerModel::render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled) 
{
    setupAnim(time, r, bob, yRot, xRot, scale);

	head->render(scale,usecompiled);
	body->render(scale,usecompiled);
	leg0->render(scale,usecompiled);
	leg1->render(scale,usecompiled);
	arms->render(scale,usecompiled);
}

void VillagerModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, unsigned int uiBitmaskOverrideAnim)
{
	head->yRot = yRot / (float) (180 / PI);
	head->xRot = xRot / (float) (180 / PI);
		
    arms->y = 3;
    arms->z = -1;
    arms->xRot = -0.75f;
		
    leg0->xRot = ((float) Mth::cos(time * 0.6662f) * 1.4f) * r * 0.5f;
    leg1->xRot = ((float) Mth::cos(time * 0.6662f + PI) * 1.4f) * r * 0.5f;
    leg0->yRot = 0;
    leg1->yRot = 0;
}

