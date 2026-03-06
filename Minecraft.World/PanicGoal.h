#pragma once

#include "Goal.h"
#include <random>

class PathfinderMob;

class PanicGoal : public Goal
{
private:
	PathfinderMob *mob;
	double speedModifier;
	double posX, posY, posZ;

public:
	PanicGoal(PathfinderMob *mob, double speedModifier);

	virtual bool canUse();
	virtual void start();
	virtual bool canContinueToUse();
};