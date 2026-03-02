#pragma once
#include "Model.h"
#include "..\Minecraft.World\Mob.h"

class WolfModel : public Model
{
public:
	ModelPart *head, *body, *leg0, *leg1, *leg2, *leg3;
//     Cube *ear1;
//     Cube *ear2;
//     Cube *mouth;
    ModelPart *tail;
    ModelPart *upperBody;
private:
	static const int legSize = 8;
public:
	WolfModel();
    virtual void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled);
    void prepareMobModel(shared_ptr<Mob> mob, float time, float r, float a);
    virtual void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, unsigned int uiBitmaskOverrideAnim=0);
};