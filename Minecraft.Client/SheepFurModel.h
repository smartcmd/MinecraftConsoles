#pragma once
#include "QuadrupedModel.h"

class SheepFurModel : public QuadrupedModel
{
private:
	float headXRot;
public:
	SheepFurModel();

	virtual void prepareMobModel(shared_ptr<Mob> mob, float time, float r, float a);
	virtual void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, unsigned int uiBitmaskOverrideAnim=0);
};