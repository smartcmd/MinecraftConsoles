#pragma once
#include "Model.h"

class SquidModel : public Model
{
public:
	static const int TENTACLES_LENGTH=8;
    ModelPart *body;
    ModelPart *tentacles[TENTACLES_LENGTH];

    SquidModel();
    virtual void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim = 0);
    virtual void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled);
};