#pragma once
#include "Model.h"

class LavaSlimeModel : public Model 
{
	static const int BODYCUBESLENGTH=8;
	ModelPart *bodyCubes[BODYCUBESLENGTH];
	ModelPart *insideCube;

public:
	LavaSlimeModel();
	int getModelVersion();
	virtual void prepareMobModel(shared_ptr<Mob> mob, float time, float r, float a); 
	virtual void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled);
};
