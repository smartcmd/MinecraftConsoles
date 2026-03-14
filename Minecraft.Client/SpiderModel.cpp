#include "stdafx.h"
#include "SpiderModel.h"
#include "../Minecraft.World/Mth.h"
#include "ModelPart.h"

SpiderModel::SpiderModel() : Model()
{
    float g = 0;
        
    int yo = 18+6-9;

    head = new ModelPart(this, 32, 4);
    head->addBox(-4, -4, -8, 8, 8, 8, g); // Head
    head->setPos(0, static_cast<float>(0 + yo), -3);

    body0 = new ModelPart(this, 0, 0);
    body0->addBox(-3, -3, -3, 6, 6, 6, g); // Body
    body0->setPos(0,static_cast<float>(yo), 0);

    body1 = new ModelPart(this, 0, 12);
    body1->addBox(-5, -4, -6, 10, 8, 12, g); // Body
    body1->setPos(0, static_cast<float>(0 + yo), 3 + 6);


    leg0 = new ModelPart(this, 18, 0);
    leg0->addBox(-15, -1, -1, 16, 2, 2, g); // Leg0
    leg0->setPos(-4, static_cast<float>(0 + yo), 2);

    leg1 = new ModelPart(this, 18, 0);
    leg1->addBox(-1, -1, -1, 16, 2, 2, g); // Leg1
    leg1->setPos(4, static_cast<float>(0 + yo), 2);

    leg2 = new ModelPart(this, 18, 0);
    leg2->addBox(-15, -1, -1, 16, 2, 2, g); // Leg2
    leg2->setPos(-4, static_cast<float>(0 + yo), 1);

    leg3 = new ModelPart(this, 18, 0);
    leg3->addBox(-1, -1, -1, 16, 2, 2, g); // Leg3
    leg3->setPos(4, static_cast<float>(0 + yo), 1);

    leg4 = new ModelPart(this, 18, 0);
    leg4->addBox(-15, -1, -1, 16, 2, 2, g); // Leg0
    leg4->setPos(-4, static_cast<float>(0 + yo), 0);

    leg5 = new ModelPart(this, 18, 0);
    leg5->addBox(-1, -1, -1, 16, 2, 2, g); // Leg1
    leg5->setPos(4, static_cast<float>(0 + yo), 0);

    leg6 = new ModelPart(this, 18, 0);
    leg6->addBox(-15, -1, -1, 16, 2, 2, g); // Leg2
    leg6->setPos(-4, static_cast<float>(0 + yo), -1);

    leg7 = new ModelPart(this, 18, 0);
    leg7->addBox(-1, -1, -1, 16, 2, 2, g); // Leg3
    leg7->setPos(4, static_cast<float>(0 + yo), -1);

	// 4J added - compile now to avoid random performance hit first time cubes are rendered
    head->compile(1.0f/16.0f);
    body0->compile(1.0f/16.0f);
    body1->compile(1.0f/16.0f);
    leg0->compile(1.0f/16.0f);
    leg1->compile(1.0f/16.0f);
    leg2->compile(1.0f/16.0f);
    leg3->compile(1.0f/16.0f);
    leg4->compile(1.0f/16.0f);
    leg5->compile(1.0f/16.0f);
    leg6->compile(1.0f/16.0f);
    leg7->compile(1.0f/16.0f);
}

void SpiderModel::render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled)
{
    setupAnim(time, r, bob, yRot, xRot, scale, entity);

    head->render(scale,usecompiled);
    body0->render(scale,usecompiled);
    body1->render(scale,usecompiled);
    leg0->render(scale,usecompiled);
    leg1->render(scale,usecompiled);
    leg2->render(scale,usecompiled);
    leg3->render(scale,usecompiled);
    leg4->render(scale,usecompiled);
    leg5->render(scale,usecompiled);
    leg6->render(scale,usecompiled);
    leg7->render(scale,usecompiled);
}

void SpiderModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim)
{
    head->yRot = yRot / (float) (180 / PI);
    head->xRot = xRot / (float) (180 / PI);

    float sr = (float) PI / 4.0f;
    leg0->zRot = -sr;
    leg1->zRot = sr;
		
    leg2->zRot = -sr * 0.74f;
    leg3->zRot = sr * 0.74f;
		
    leg4->zRot = -sr * 0.74f;
    leg5->zRot = sr * 0.74f;
		
    leg6->zRot = -sr;
    leg7->zRot = sr;

    float ro = -(float) PI / 2.0f * 0;
    float ur = (float) PI / 8.0f;
    leg0->yRot = +ur * 2.0f + ro;
    leg1->yRot = -ur * 2.0f - ro;
    leg2->yRot = +ur * 1.0f + ro;
    leg3->yRot = -ur * 1.0f - ro;
    leg4->yRot = -ur * 1.0f + ro;
    leg5->yRot = +ur * 1.0f - ro;
    leg6->yRot = -ur * 2.0f + ro;
    leg7->yRot = +ur * 2.0f - ro;


    float c0 = -((float) Mth::cos(time * 0.6662f * 2 + PI * 2 * 0 / 4.0f) * 0.4f) * r;
    float c1 = -((float) Mth::cos(time * 0.6662f * 2 + PI * 2 * 2 / 4.0f) * 0.4f) * r;
    float c2 = -((float) Mth::cos(time * 0.6662f * 2 + PI * 2 * 1 / 4.0f) * 0.4f) * r;
    float c3 = -((float) Mth::cos(time * 0.6662f * 2 + PI * 2 * 3 / 4.0f) * 0.4f) * r;

    float s0 = abs((float)Mth::sin(time * 0.6662f + PI * 2 * 0 / 4.0f) * 0.4f) * r;
    float s1 = abs((float)Mth::sin(time * 0.6662f + PI * 2 * 2 / 4.0f) * 0.4f) * r;
    float s2 = abs((float)Mth::sin(time * 0.6662f + PI * 2 * 1 / 4.0f) * 0.4f) * r;
    float s3 = abs((float)Mth::sin(time * 0.6662f + PI * 2 * 3 / 4.0f) * 0.4f) * r;

    leg0->yRot += +c0;
    leg1->yRot += -c0;
    leg2->yRot += +c1;
    leg3->yRot += -c1;
    leg4->yRot += +c2;
    leg5->yRot += -c2;
    leg6->yRot += +c3;
    leg7->yRot += -c3;

    leg0->zRot += +s0;
    leg1->zRot += -s0;
    leg2->zRot += +s1;
    leg3->zRot += -s1;
    leg4->zRot += +s2;
    leg5->zRot += -s2;
    leg6->zRot += +s3;
    leg7->zRot += -s3;
}