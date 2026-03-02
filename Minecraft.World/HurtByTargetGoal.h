#pragma once

#include "TargetGoal.h"

class HurtByTargetGoal : public TargetGoal
{
private:
	bool alertSameType;
	int timestamp;

public:
	HurtByTargetGoal(PathfinderMob *mob, bool alertSameType);

	bool canUse();
	void start();
};
