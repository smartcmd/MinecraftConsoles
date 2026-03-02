#pragma once
#include "HumanoidModel.h"

class ZombieModel : public HumanoidModel
{
public:
	ZombieModel();

protected:
	ZombieModel(float g, float yOffset, int texWidth, int texHeight);

public:
	ZombieModel(float g, bool isArmor);

	virtual void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim=0);
};
