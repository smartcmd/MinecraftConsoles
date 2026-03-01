#pragma once

#include "Goal.h"

class RangedAttackMob;

class RangedAttackGoal : public Goal
{
private:
	Mob *mob; // Owner
	RangedAttackMob *rangedAttackMob; // owner
	weak_ptr<LivingEntity> target;
	int attackTime;
	double speedModifier;
	int seeTime;
	int attackIntervalMin;
	int attackIntervalMax;
	float attackRadius;
	float attackRadiusSqr;

	void _init(RangedAttackMob *rangedMob, Mob *mob, double speedModifier, int attackIntervalMin, int attackIntervalMax, float attackRadius);

public:
	// 4J Added extra Mob param to avoid weird type conversion problems
	RangedAttackGoal(RangedAttackMob *rangedMob, Mob *mob, double speedModifier, int attackInterval, float attackRadius);
	RangedAttackGoal(RangedAttackMob *rangedMob, Mob *mob, double speedModifier, int attackIntervalMin, int attackIntervalMax, float attackRadius);

	bool canUse();
	bool canContinueToUse();
	void stop();
	void tick();
};