#pragma once

#include "Goal.h"

class PathfinderMob;

class TargetGoal : public Goal
{

public:
	static const int TargetFlag = 1;

private:
	static const int EmptyReachCache = 0;
	static const int CanReachCache = 1;
	static const int CantReachCache = 2;
	static const int UnseenMemoryTicks = 60;

protected:
	PathfinderMob *mob; // Owner of this goal
	bool mustSee;

private:
	bool mustReach;
	int reachCache;
	int reachCacheTime;
	int unseenTicks;

	void _init(PathfinderMob *mob, bool mustSee, bool mustReach);

public:
	TargetGoal(PathfinderMob *mob, bool mustSee);
	TargetGoal(PathfinderMob *mob, bool mustSee, bool mustReach);
	virtual ~TargetGoal() {}

	virtual bool canContinueToUse();

protected:
	virtual double getFollowDistance();

public:
	virtual void start();
	virtual void stop();

protected:
	virtual bool canAttack(shared_ptr<LivingEntity> target, bool allowInvulnerable);

private:
	bool canReach(shared_ptr<LivingEntity> target);
};