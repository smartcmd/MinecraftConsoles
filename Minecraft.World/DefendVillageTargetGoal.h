#pragma once

#include "TargetGoal.h"

class VillagerGolem;

class DefendVillageTargetGoal : public TargetGoal
{
private:
	VillagerGolem *golem; // Owner of this goal
	weak_ptr<LivingEntity> potentialTarget;

public:
	DefendVillageTargetGoal(VillagerGolem *golem);

	bool canUse();
	void start();
};