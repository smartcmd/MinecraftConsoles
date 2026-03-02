#pragma once
#include "QuadrupedModel.h"

class SheepModel : public QuadrupedModel
{
private:
	float headXRot;
public:
	SheepModel();

	virtual void prepareMobModel(shared_ptr<LivingEntity> mob, float time, float r, float a);
	virtual void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim=0);
};