#pragma once

#include "Goal.h"

class FleeSunGoal : public Goal
{
private:
	PathfinderMob *mob; // Owner of this goal
	double wantedX, wantedY, wantedZ;
	double speedModifier;
	Level *level;

public:
	FleeSunGoal(PathfinderMob *mob, double speedModifier);

	virtual bool canUse();
	virtual bool canContinueToUse();
	virtual void start();

private:
	Vec3 *getHidePos();

public:
	// 4J Added override to update ai elements when loading entity from schematics
	virtual void setLevel(Level *level) { this->level = level; }
};