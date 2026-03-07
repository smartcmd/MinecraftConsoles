#include "stdafx.h"
#include "SilverfishModel.h"
#include "Cube.h"
#include "../Minecraft.World/Mth.h"
#include "ModelPart.h"

const int SilverfishModel::BODY_SIZES[BODY_COUNT][3] = {
	{
		3, 2, 2
	}, {
		4, 3, 2
	}, {
		6, 4, 3
	}, {
		3, 3, 3
	}, {
		2, 2, 3
	}, {
		2, 1, 2
	}, {
		1, 1, 2
	},
};

const int SilverfishModel::BODY_TEXS[BODY_COUNT][2] = {
	{
		0, 0
	}, {
		0, 4
	}, {
		0, 9
	}, {
		0, 16
	}, {
		0, 22
	}, {
		11, 0
	}, {
		13, 4
	},
};

SilverfishModel::SilverfishModel()
{
	bodyParts = ModelPartArray(BODY_COUNT);
	float placement = -3.5f;
	for (unsigned int i = 0; i < bodyParts.length; i++)
	{
		bodyParts[i] = new ModelPart(this, BODY_TEXS[i][0], BODY_TEXS[i][1]);
		bodyParts[i]->addBox(BODY_SIZES[i][0] * -0.5f, 0, BODY_SIZES[i][2] * -0.5f, BODY_SIZES[i][0], BODY_SIZES[i][1], BODY_SIZES[i][2]);
		bodyParts[i]->setPos(0.0f, 24.0f - (float)BODY_SIZES[i][1], placement);
		zPlacement[i] = placement;
		if (i < bodyParts.length - 1)
		{
			placement += (BODY_SIZES[i][2] + BODY_SIZES[i + 1][2]) * .5f;
		}
	}

	bodyLayers = ModelPartArray(3);
	bodyLayers[0] = new ModelPart(this, 20, 0);
	bodyLayers[0]->addBox(-5, 0, BODY_SIZES[2][2] * -0.5f, 10, 8, BODY_SIZES[2][2]);
	bodyLayers[0]->setPos(0, 24 - 8, zPlacement[2]);
	bodyLayers[1] = new ModelPart(this, 20, 11);
	bodyLayers[1]->addBox(-3, 0, BODY_SIZES[4][2] * -0.5f, 6, 4, BODY_SIZES[4][2]);
	bodyLayers[1]->setPos(0, 24 - 4, zPlacement[4]);
	bodyLayers[2] = new ModelPart(this, 20, 18);
	bodyLayers[2]->addBox(-3, 0, BODY_SIZES[4][2] * -0.5f, 6, 5, BODY_SIZES[1][2]);
	bodyLayers[2]->setPos(0, 24 - 5, zPlacement[1]);

	// 4J added - compile now to avoid random performance hit first time cubes are rendered
	for (unsigned int i = 0; i < bodyParts.length; i++)
	{
		bodyParts[i]->compile(1.0f/16.0f);
	}
	bodyLayers[0]->compile(1.0f/16.0f);
	bodyLayers[1]->compile(1.0f/16.0f);
	bodyLayers[2]->compile(1.0f/16.0f);
}

int SilverfishModel::modelVersion()
{
	return 38;
}

void SilverfishModel::render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled)
{
	setupAnim(time, r, bob, yRot, xRot, scale, entity);

	for (unsigned int i = 0; i < bodyParts.length; i++)
	{
		bodyParts[i]->render(scale, usecompiled);
	}
	for (unsigned int i = 0; i < bodyLayers.length; i++)
	{
		bodyLayers[i]->render(scale, usecompiled);
	}
}

void SilverfishModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim)
{
	for (unsigned int i = 0; i < bodyParts.length; i++)
	{
		bodyParts[i]->yRot = Mth::cos(bob * .9f + i * .15f * PI) * PI * .05f * (1 + abs((int)i - 2));
		bodyParts[i]->x = Mth::sin(bob * .9f + i * .15f * PI) * PI * .2f * abs((int)i - 2);
	}

	bodyLayers[0]->yRot = bodyParts[2]->yRot;
	bodyLayers[1]->yRot = bodyParts[4]->yRot;
	bodyLayers[1]->x = bodyParts[4]->x;
	bodyLayers[2]->yRot = bodyParts[1]->yRot;
	bodyLayers[2]->x = bodyParts[1]->x;
}