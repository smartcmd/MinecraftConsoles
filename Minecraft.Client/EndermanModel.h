#pragma once

#include "HumanoidModel.h"

class EndermanModel : public HumanoidModel
{
public:
	bool carrying;
	bool creepy;

	EndermanModel();
	virtual void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim=0);
};