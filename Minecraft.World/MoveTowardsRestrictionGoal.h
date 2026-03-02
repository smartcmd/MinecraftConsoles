#pragma once

#include "Goal.h"

class MoveTowardsRestrictionGoal : public Goal
{
private:
	PathfinderMob *mob;
	double wantedX, wantedY, wantedZ;
	double speedModifier;

public:
	MoveTowardsRestrictionGoal(PathfinderMob *mob, double speedModifier);

	bool canUse();
	bool canContinueToUse();
	void start();
};