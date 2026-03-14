#include "stdafx.h"
#include "../Minecraft.World/Mth.h"
#include "LavaSlimeModel.h"
#include "ModelPart.h"
#include "../Minecraft.World/LavaSlime.h"


LavaSlimeModel::LavaSlimeModel() 
{
    for (int i = 0; i < BODYCUBESLENGTH; i++) 
	{
        int u = 0;
        int v = i;
        if (i == 2) 
		{
            u = 24;
            v = 10;
        } 
		else if (i == 3) 
		{
            u = 24;
            v = 19;
        }
        bodyCubes[i] = new ModelPart(this, u, v);
        bodyCubes[i]->addBox(-4.0f, 16.0f + static_cast<float>(i), -4.0f, 8, 1, 8);
    }

    insideCube = new ModelPart(this, 0, 16);
    insideCube->addBox(-2, 16 + 2, -2, 4, 4, 4);

	// 4J added - compile now to avoid random performance hit first time cubes are rendered
	insideCube->compile(1.0f/16.0f);
	for( int i = 0; i < BODYCUBESLENGTH; i++ )
	{
		bodyCubes[i]->compile(1.0f/16.0f);
	}
}

int LavaSlimeModel::getModelVersion() 
{
        return 5;
}

void LavaSlimeModel::prepareMobModel(shared_ptr<LivingEntity> mob, float time, float r, float a) 
{
	 shared_ptr<LavaSlime> lavaSlime = dynamic_pointer_cast<LavaSlime>(mob);

    float slimeSquish = (lavaSlime->oSquish + (lavaSlime->squish - lavaSlime->oSquish) * a);
    if (slimeSquish < 0) 
	{
        slimeSquish = 0.0f;
    }

    for (int i = 0; i < BODYCUBESLENGTH; i++) 
	{
        bodyCubes[i]->y = -(4 - i) * slimeSquish * 1.7f;
    }
}

void LavaSlimeModel::render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled) 
{
	setupAnim(time, r, bob, yRot, xRot, scale, entity);

	insideCube->render(scale, usecompiled);
	for (int i = 0; i < BODYCUBESLENGTH; i++) 
	{
		bodyCubes[i]->render(scale, usecompiled);
	}

}

