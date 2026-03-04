#include "stdafx.h"
#include "ModelPart.h"
#include "WitchModel.h"
#include "VillagerModel.h"
#include "../Minecraft.World/Mth.h"

WitchModel::WitchModel(float g) : VillagerModel(g, 0, 64, 128)
{
	mole = (new ModelPart(this))->setTexSize(64, 128);
	mole->setPos(0, -2, 0);
	mole->texOffs(0, 0)->addBox(0, 3, -6.75f, 1, 1, 1, -0.25f);
	nose->addChild(mole);

	hat = (new ModelPart(this))->setTexSize(64, 128);
	hat->setPos(-5, -10 - (0.5f / 16.f), -5);
	hat->texOffs(0, 64)->addBox(0, 0, 0, 10, 2, 10);
	head->addChild(hat);

	ModelPart *hat2 = new ModelPart(this);
	hat2->setTexSize(64, 128);
	hat2->setPos(1.75f, -4, 2.f);
	hat2->texOffs(0, 76)->addBox(0, 0, 0, 7, 4, 7);
	hat2->xRot = -3.f * Mth::RAD_TO_GRAD;
	hat2->zRot = 1.5f * Mth::RAD_TO_GRAD;
	hat->addChild(hat2);

	ModelPart *hat3 = new ModelPart(this);
	hat3->setTexSize(64, 128);
	hat3->setPos(1.75f, -4, 2.f);
	hat3->texOffs(0, 87)->addBox(0, 0, 0, 4, 4, 4);
	hat3->xRot = -6.f * Mth::RAD_TO_GRAD;
	hat3->zRot = 3.f * Mth::RAD_TO_GRAD;
	hat2->addChild(hat3);

	ModelPart *hat4 = new ModelPart(this);
	hat4->setTexSize(64, 128);
	hat4->setPos(1.75f, -2, 2.f);
	hat4->texOffs(0, 95)->addBox(0, 0, 0, 1, 2, 1, 0.25f);
	hat4->xRot = -12.f * Mth::RAD_TO_GRAD;
	hat4->zRot = 6.f * Mth::RAD_TO_GRAD;
	hat3->addChild(hat4);

	// 4J added - compile now to avoid random performance hit first time cubes are rendered
	mole->compile(1.0f/16.0f);
	hat->compile(1.0f/16.0f);
	hat2->compile(1.0f/16.0f);
	hat3->compile(1.0f/16.0f);
	hat4->compile(1.0f/16.0f);
}

void WitchModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim)
{
	VillagerModel::setupAnim(time, r, bob, yRot, xRot, scale, entity);

	nose->translateX = nose->translateY = nose->translateZ = 0;

	float speed = 0.01f * (entity->entityId % 10);
	nose->xRot = (sin(entity->tickCount * speed) * 4.5f) * PI / 180;
	nose->yRot = 0;
	nose->zRot = (cos(entity->tickCount * speed) * 2.5f) * PI / 180;

	if (holdingItem)
	{
		nose->xRot = -0.9f;
		nose->translateZ = -1.5f / 16.f;
		nose->translateY = 3 / 16.f;
	}
}

int WitchModel::getModelVersion()
{
	return 0;
}