#pragma once

#include "Goal.h"
#include "EntitySelector.h"

class PathNavigation;
class PathfinderMob;
class Path;
class AvoidPlayerGoal;

class AvoidPlayerGoalEntitySelector : public EntitySelector
{
private:
	AvoidPlayerGoal *m_parent;

public:
	AvoidPlayerGoalEntitySelector(AvoidPlayerGoal *parent);
	bool matches(shared_ptr<Entity> entity) const;
};

class AvoidPlayerGoal : public Goal
{
	friend class AvoidPlayerGoalEntitySelector;
private:
	PathfinderMob *mob; // Owner of this goal
	double walkSpeedModifier, sprintSpeedModifier;
	weak_ptr<Entity> toAvoid;
	float maxDist;
	Path *path;
	PathNavigation *pathNav;
	const type_info& avoidType;
	EntitySelector *entitySelector;

public:
	AvoidPlayerGoal(PathfinderMob *mob, const type_info& avoidType, float maxDist, double walkSpeedModifier, double sprintSpeedModifier);
	~AvoidPlayerGoal();

	virtual bool canUse();
	virtual bool canContinueToUse();
	virtual void start();
	virtual void stop();
	virtual void tick();
};