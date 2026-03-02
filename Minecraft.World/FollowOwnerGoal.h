#pragma once

#include "Goal.h"

class PathNavigation;
class TamableAnimal;

class FollowOwnerGoal : public Goal
{
public:
	static const int TeleportDistance = 12;

private:
	TamableAnimal *tamable; // Owner of this goal
	weak_ptr<LivingEntity> owner;
	Level *level;
	double speedModifier;
	PathNavigation *navigation;
	int timeToRecalcPath;
	float stopDistance, startDistance;
	bool oldAvoidWater;

public:
	FollowOwnerGoal(TamableAnimal *tamable, double speedModifier, float startDistance, float stopDistance);

	virtual bool canUse();
	virtual bool canContinueToUse();
	virtual void start();
	virtual void stop();
	virtual void tick();
	
	// 4J Added override to update ai elements when loading entity from schematics
	virtual void setLevel(Level *level) { this->level = level; }
};