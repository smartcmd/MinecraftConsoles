#pragma once

#include "Goal.h"

class PathfinderMob;
class Path;
class DoorInfo;

class MoveThroughVillageGoal : public Goal
{
private:
	PathfinderMob *mob;
	double speedModifier;
	Path *path;
	weak_ptr<DoorInfo> doorInfo;
	bool onlyAtNight;
	vector< weak_ptr<DoorInfo> > visited;

public:
	MoveThroughVillageGoal(PathfinderMob *mob, double speedModifier, bool onlyAtNight);
	~MoveThroughVillageGoal();

	virtual bool canUse();
	virtual bool canContinueToUse();
	virtual void start();
	virtual void stop();

private:
	shared_ptr<DoorInfo> getNextDoorInfo(shared_ptr<Village> village);
	bool hasVisited(shared_ptr<DoorInfo> di);
	void updateVisited();
};