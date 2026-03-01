#pragma once

#include "Goal.h"

class LeapAtTargetGoal : public Goal
{
private:
    Mob *mob; // Owner of this goal
    weak_ptr<LivingEntity> target;
    float yd;

public:
	LeapAtTargetGoal(Mob *mob, float yd);

    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
};