#pragma once
#include "Model.h"

class BatModel : public Model
{
private:
    ModelPart *head;
    ModelPart *body;
    ModelPart *rightWing;
    ModelPart *leftWing;
    ModelPart *rightWingTip;
    ModelPart *leftWingTip;

public:    
	BatModel();

    int modelVersion();

    virtual void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled);
};