#pragma once
#include "Model.h"
#include "ModelPart.h"

class EnderCrystalModel : public Model
{
public:
	static const int MODEL_ID = 1;

private:
	ModelPart *cube;
	ModelPart *glass;
	ModelPart *base;

public:
	EnderCrystalModel(float g);
	virtual void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled);
};