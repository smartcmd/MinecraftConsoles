#pragma once
#include "Model.h"

class QuadrupedModel : public Model
{
public:
	ModelPart *head, *body, *leg0, *leg1, *leg2, *leg3;

	QuadrupedModel(int legSize, float g);
	virtual void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled);
	virtual void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, unsigned int uiBitmaskOverrideAnim=0);
	void render(QuadrupedModel *model, float scale, bool usecompiled);
};