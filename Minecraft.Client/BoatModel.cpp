#include "stdafx.h"
#include "BoatModel.h"

BoatModel::BoatModel() : Model()
{
	cubes[0] = new ModelPart(this, 0, 8);
	cubes[1] = new ModelPart(this, 0, 0);
	cubes[2] = new ModelPart(this, 0, 0);
	cubes[3] = new ModelPart(this, 0, 0);
	cubes[4] = new ModelPart(this, 0, 0);

	int w = 24;
	int d = 6;
	int h = 20;
	int yOff = 4;

	cubes[0]->addBox((float)(-w / 2), (float)(-h / 2 + 2), -3, w, h - 4, 4, 0);
	cubes[0]->setPos(0, (float)(0 + yOff), 0);
			
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

	// 4J added - compile now to avoid random performance hit first time cubes are rendered
	cubes[0]->compile(1.0f/16.0f);
	cubes[1]->compile(1.0f/16.0f);
	cubes[2]->compile(1.0f/16.0f);
	cubes[3]->compile(1.0f/16.0f);
	cubes[4]->compile(1.0f/16.0f);
}

void BoatModel::render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled)
{
	for (int i = 0; i < 5; i++)
	{
		cubes[i]->render(scale, usecompiled);
	}
}