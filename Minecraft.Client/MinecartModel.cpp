#include "stdafx.h"
#include "MinecartModel.h"
#include "ModelPart.h"

MinecartModel::MinecartModel() : Model()
{
    cubes[0] = new ModelPart(this, 0, 10);
    cubes[1] = new ModelPart(this, 0, 0);
    cubes[2] = new ModelPart(this, 0, 0);
    cubes[3] = new ModelPart(this, 0, 0);
    cubes[4] = new ModelPart(this, 0, 0);
    cubes[5] = new ModelPart(this, 44, 10);

    int w = 20;
    int d = 8;
    int h = 16;
    int yOff = 4;

    cubes[0]->addBox((float)(-w / 2), (float)(-h / 2), -1, w, h, 2, 0);
    cubes[0]->setPos(0, (float)(0 + yOff), 0);
			
    cubes[5]->addBox((float)(-w / 2 + 1), (float)(-h / 2 + 1), -1, w - 2, h - 2, 1, 0);
    cubes[5]->setPos(0, (float)(0 + yOff), 0);
			
    cubes[1]->addBox((float)(-w / 2 + 2), (float)(-d - 1), -1, w - 4, d, 2, 0);
    cubes[1]->setPos((float)(-w / 2 + 1), (float)(0 + yOff), 0);
			
    cubes[2]->addBox((float)(-w / 2 + 2), (float)(-d - 1), -1, w - 4, d, 2, 0);
    cubes[2]->setPos((float)(+w / 2 - 1), (float)(0 + yOff), 0);
			
    cubes[3]->addBox((float)(-w / 2 + 2), (float)(-d - 1), -1, w - 4, d, 2, 0);
    cubes[3]->setPos(0, (float)(0 + yOff), (float)(-h / 2 + 1));
			
    cubes[4]->addBox((float)(-w / 2 + 2), (float)(-d - 1), -1, w - 4, d, 2, 0);
    cubes[4]->setPos(0, (float)(0 + yOff), (float)(+h / 2 - 1));
			
    cubes[0]->xRot = PI / 2;
    cubes[1]->yRot = PI / 2 * 3;
    cubes[2]->yRot = PI / 2 * 1;
    cubes[3]->yRot = PI / 2 * 2;
    cubes[5]->xRot = -PI / 2;

	// 4J added - compile now to avoid random performance hit first time cubes are rendered
    for (int i = 0; i < MINECART_LENGTH; i++)
	{
        cubes[i]->compile(1.0f/16.0f);
    }
}

void MinecartModel::render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled)
{
    cubes[5]->y = 4 - bob;
    for (int i = 0; i < MINECART_LENGTH; i++)
	{
        cubes[i]->render(scale, usecompiled);
    }
}
