#pragma once
#include "Model.h"

 class SpiderModel : public Model
{
public:
	ModelPart *head, *body0, *body1, *leg0, *leg1, *leg2, *leg3, *leg4, *leg5, *leg6, *leg7;

    SpiderModel();
    virtual void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled);
    virtual void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, unsigned int uiBitmaskOverrideAnim=0);
};