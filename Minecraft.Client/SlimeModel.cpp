#include "stdafx.h"
#include "SlimeModel.h"
#include "ModelPart.h"

SlimeModel::SlimeModel(int vOffs)
{
    cube = new ModelPart(this, 0, vOffs);
    cube->addBox(-4, 16, -4, 8, 8, 8);
    if (vOffs > 0)
	{
        cube = new ModelPart(this, 0, vOffs);
        cube->addBox(-3, 16+1, -3, 6, 6, 6);

        eye0 = new ModelPart(this, 32, 0);
        eye0->addBox(-3-0.25f, 16+2, -3.5f, 2, 2, 2);

        eye1 = new ModelPart(this, 32, 4);
        eye1->addBox(+1+0.25f, 16+2, -3.5f, 2, 2, 2);

        mouth = new ModelPart(this, 32, 8);
        mouth->addBox(0, 16+5, -3.5f, 1, 1, 1);

		// 4J added - compile now to avoid random performance hit first time cubes are rendered
		eye0->compile(1.0f/16.0f);
		eye1->compile(1.0f/16.0f);
		mouth->compile(1.0f/16.0f);
    }
	else
	{
		eye0 = NULL;
		eye1 = NULL;
		mouth = NULL;
	}
	cube->compile(1.0f/16.0f);
}

void SlimeModel::render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled)
{
    setupAnim(time, r, bob, yRot, xRot, scale, entity);

    cube->render(scale,usecompiled);
    if (eye0!=NULL)
	{
        eye0->render(scale,usecompiled);
        eye1->render(scale,usecompiled);
        mouth->render(scale,usecompiled);
    }
}