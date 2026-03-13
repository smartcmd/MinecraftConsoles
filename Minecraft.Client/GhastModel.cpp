#include "stdafx.h"
#include "../Minecraft.World/Random.h"
#include "../Minecraft.World/Mth.h"
#include "GhastModel.h"
#include "ModelPart.h"

GhastModel::GhastModel() : Model()
{
    int yoffs = -16;
    body = new ModelPart(this, 0, 0);
    body->addBox(-8, -8, -8, 16, 16, 16);
    body->y += (8 + 16) + yoffs;

    Random *random = new Random(1660);
    for (int i = 0; i < TENTACLESLENGTH; i++)		// 4J - 9 was  tentacles.length
	{
        tentacles[i] = new ModelPart(this, 0, 0);

        float xo = (((i % 3 - (i / 3 % 2) * 0.5f + 0.25f) / 2.0f * 2 - 1) * 5);
        float yo = (((i / 3) / 2.0f * 2 - 1) * 5);
        int len = random->nextInt(7) + 8;
        tentacles[i]->addBox(-1, 0, -1, 2, len, 2);

        tentacles[i]->x = xo;
        tentacles[i]->z = yo;
        tentacles[i]->y = static_cast<float>(31 + yoffs);
    }

	// 4J added - compile now to avoid random performance hit first time cubes are rendered
	body->compile(1.0f/16.0f);
	for( int i = 0; i < TENTACLESLENGTH; i++ )
	{
		tentacles[i]->compile(1.0f/16.0f);
	}
}

void GhastModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim)
{
    for (int i = 0; i < TENTACLESLENGTH; i++) // 4J - 9 was  tentacles.length
	{
        tentacles[i]->xRot = 0.2f * Mth::sin(bob * 0.3f + i) + 0.4f;
    }
}

void GhastModel::render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled)
{
    setupAnim(time, r, bob, yRot, xRot, scale, entity);

	glPushMatrix();
	glTranslatef(0, .6f, 0);

    body->render(scale, usecompiled);
    for (int i = 0; i < TENTACLESLENGTH; i++) // 4J - 9 was  tentacles.length
	{
        tentacles[i]->render(scale, usecompiled);
    }

	glPopMatrix();
}