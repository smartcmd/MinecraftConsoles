#pragma once

#include "Goal.h"

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
	Mob *mob; // Owner of this goal
	float within;
	bool mustSee;

private:
	bool mustReach;
	int reachCache;
	int reachCacheTime;
	int unseenTicks;

	void _init(Mob *mob, float within, bool mustSee, bool mustReach);

public:
	TargetGoal(Mob *mob, float within, bool mustSee);
	TargetGoal(Mob *mob, float within, bool mustSee, bool mustReach);
	virtual ~TargetGoal() {}

	virtual bool canContinueToUse();
	virtual void start();
	virtual void stop();

protected:
	virtual bool canAttack(shared_ptr<Mob> target, bool allowInvulnerable);

private:
	bool canReach(shared_ptr<Mob> target);
};