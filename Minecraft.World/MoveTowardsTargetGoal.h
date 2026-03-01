#pragma once

#include "Goal.h"

class MoveTowardsTargetGoal : public Goal
{
private:
	PathfinderMob *mob;
	weak_ptr<LivingEntity> target;
	double wantedX, wantedY, wantedZ;
	double speedModifier;
	float within;

public:
	MoveTowardsTargetGoal(PathfinderMob *mob, double speedModifier, float within);

	bool canUse();
	bool canContinueToUse();
	void stop();
	void start();
};