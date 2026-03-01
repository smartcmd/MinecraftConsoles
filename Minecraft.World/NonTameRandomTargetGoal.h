#pragma once

#include "NearestAttackableTargetGoal.h"

class TamableAnimal;

class NonTameRandomTargetGoal : public NearestAttackableTargetGoal
{
private:
	TamableAnimal *tamableMob; // Owner of this goal

public:
	NonTameRandomTargetGoal(TamableAnimal *mob, const type_info& targetType, int randomInterval, bool mustSee);

	bool canUse();
};