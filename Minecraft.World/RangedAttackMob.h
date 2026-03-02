#pragma once

class RangedAttackMob
{
public:
	virtual void performRangedAttack(shared_ptr<LivingEntity> target, float power) = 0;
};