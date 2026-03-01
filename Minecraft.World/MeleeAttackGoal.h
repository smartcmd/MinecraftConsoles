#pragma once

#include "Goal.h"

class Level;
class PathfinderMob;
class Path;

class MeleeAttackGoal : public Goal
{
private:
	Level *level;
	PathfinderMob *mob; // Owner of this goal
	int attackTime;
	double speedModifier;
	bool trackTarget;
	Path *path;
	eINSTANCEOF attackType;
	int timeToRecalcPath;

	void _init(PathfinderMob *mob, double speedModifier, bool trackTarget);

public:
	MeleeAttackGoal(PathfinderMob *mob, eINSTANCEOF attackType, double speedModifier, bool trackTarget);
	MeleeAttackGoal(PathfinderMob *mob, double speedModifier, bool trackTarget);
	~MeleeAttackGoal();

	virtual bool canUse();
	virtual bool canContinueToUse();
	virtual void start();
	virtual void stop();
	virtual void tick();
	
	// 4J Added override to update ai elements when loading entity from schematics
	virtual void setLevel(Level *level) { this->level = level; }
};