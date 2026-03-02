#pragma once

#include "Goal.h"

class PlayGoal : public Goal
{
private:
	Villager *mob;
	weak_ptr<LivingEntity> followFriend;
	double speedModifier;
	double wantedX, wantedY, wantedZ;
	int playTime;

public:
	PlayGoal(Villager *mob, double speedModifier);

	virtual bool canUse();
	virtual bool canContinueToUse();
	virtual void start();
	virtual void stop();
	virtual void tick();
};