#pragma once

#include "Goal.h"

class TemptGoal : public Goal
{
private:
	PathfinderMob *mob;
	double speedModifier;
	double px, py, pz, pRotX, pRotY;
	weak_ptr<Player> player;
	int calmDown ;
	bool _isRunning;
	int itemId;
	bool canScare;
	bool oldAvoidWater;

public:
	TemptGoal(PathfinderMob *mob, double speedModifier, int itemId, bool canScare);

	bool canUse();
	bool canContinueToUse();
	void start();
	void stop();
	void tick();
	bool isRunning();
};