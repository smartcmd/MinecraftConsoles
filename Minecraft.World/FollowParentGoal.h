#pragma once

#include "Goal.h"

class Animal;

class FollowParentGoal : public Goal
{
private:
	Animal *animal; // Owner of this goal
	weak_ptr<Animal> parent;
	double speedModifier;
	int timeToRecalcPath;

public:
	FollowParentGoal(Animal *animal, double speedModifier);

	virtual bool canUse();
	virtual bool canContinueToUse();
	virtual void start();
	virtual void stop();
	virtual void tick();
};