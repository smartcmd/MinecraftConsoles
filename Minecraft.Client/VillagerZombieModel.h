#pragma once

#include "HumanoidModel.h"

class VillagerZombieModel : public HumanoidModel
{
private:
	void _init(float g, float yOffset, bool isArmor);

public:
	VillagerZombieModel();
	VillagerZombieModel(float g, float yOffset, bool isArmor);

	int version();
	void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim=0);
};