#include "stdafx.h"
#include "SquidModel.h"
#include "..\Minecraft.World\Mth.h"
#include "ModelPart.h"

SquidModel::SquidModel() : Model()
{
    int yoffs = -16;
    body = new ModelPart(this, 0, 0);
    body->addBox(-6, -8, -6, 12, 16, 12);
    body->y += (8 + 16) + yoffs;

    for (int i = 0; i < TENTACLES_LENGTH; i++)			// 4J - 8 was tentacles.length
	{
        tentacles[i] = new ModelPart(this, 48, 0);

        double angle = i * PI * 2.0 / (double) TENTACLES_LENGTH;	// 4J - 8 was tentacles.length
        float xo = Mth::cos((float)angle) * 5;
        float yo = Mth::sin((float)angle) * 5;
        tentacles[i]->addBox(-1, 0, -1, 2, 18, 2);

        tentacles[i]->x = xo;
        tentacles[i]->z = yo;
        tentacles[i]->y = (float)(31 + yoffs);

        angle = i * PI * -2.0 / (double) TENTACLES_LENGTH + PI * .5;  // 4J - 8 was tentacles.length
        tentacles[i]->yRot = (float) angle;

		// 4J added - compile now to avoid random performance hit first time cubes are rendered
		tentacles[i]->compile(1.0f/16.0f);
	}
	body->compile(1.0f/16.0f);

}

void SquidModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, unsigned int uiBitmaskOverrideAnim)
{
    for (int i = 0; i < TENTACLES_LENGTH; i++)	 // 4J - 8 was tentacles.length
	{

        // tentacle angle is calculated in SquidRenderer
        tentacles[i]->xRot = bob;
    }
}

void SquidModel::render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled)
{
    setupAnim(time, r, bob, yRot, xRot, scale);

    body->render(scale, usecompiled);
    for (int i = 0; i < TENTACLES_LENGTH; i++) // 4J - 8 was tentacles.length // 4J Stu - Was 9 but I made it 8 as the array is [0,8)
	{
        tentacles[i]->render(scale, usecompiled);
    }
}