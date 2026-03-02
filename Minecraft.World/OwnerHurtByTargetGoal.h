#pragma once

#include "TargetGoal.h"

class TamableAnimal;

class OwnerHurtByTargetGoal : public TargetGoal
{
private:
	TamableAnimal *tameAnimal; // Owner of this goal
	weak_ptr<LivingEntity> ownerLastHurtBy;
	int timestamp;

public:
	OwnerHurtByTargetGoal(TamableAnimal *tameAnimal);

	bool canUse();
	void start();
};