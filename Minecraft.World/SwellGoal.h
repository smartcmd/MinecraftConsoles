#pragma once

#include "Goal.h"

class Creeper;

class SwellGoal : public Goal
{
private:
	Creeper *creeper;
	weak_ptr<LivingEntity> target;

public:
	SwellGoal(Creeper *creeper);

	bool canUse();
	void start();
	void stop();
	void tick();
};