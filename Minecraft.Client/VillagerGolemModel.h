#pragma once

#include "Model.h"

class ModelPart;

class VillagerGolemModel : public Model
{
public:
	ModelPart *head, *body, *arm0, *arm1, *leg0, *leg1;

	//VillagerGolemModel()
	//{
	//	this(0);
	//}

	//VillagerGolemModel(float g)
	//{
	//	this(g, -7);
	//}

	VillagerGolemModel(float g = 0.0f, float yOffset = -7.0f);

	void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled);
	void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim=0);
	void prepareMobModel(shared_ptr<LivingEntity> mob, float time, float r, float a);

private:
	float triangleWave(float bob, float period);
};