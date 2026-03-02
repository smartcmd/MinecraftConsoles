#pragma once

#include "TargetGoal.h"

class HurtByTargetGoal : public TargetGoal
{
private:
	bool alertSameType;
	shared_ptr<Mob> oldHurtByMob;

public:
	HurtByTargetGoal(Mob *mob, bool alertSameType);

	bool canUse();
	void start();
	void tick();
};
