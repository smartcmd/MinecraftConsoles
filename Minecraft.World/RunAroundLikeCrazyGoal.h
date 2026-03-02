#pragma once

#include "Goal.h"

class EntityHorse;

class RunAroundLikeCrazyGoal : public Goal
{
private:
	EntityHorse *horse; // Owner
	double speedModifier;
	double posX, posY, posZ;

public:
	RunAroundLikeCrazyGoal(EntityHorse *mob, double speedModifier);

	bool canUse();
	void start();
	bool canContinueToUse();
	void tick();
};