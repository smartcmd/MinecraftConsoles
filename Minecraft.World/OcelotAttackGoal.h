#pragma once

#include "Goal.h"

class OcelotAttackGoal : public Goal
{
private:
	Level *level;
	Mob *mob;
	weak_ptr<LivingEntity> target;
	int attackTime;
	float speed;
	bool trackTarget;

public:
	OcelotAttackGoal(Mob *mob);

	virtual bool canUse();
	virtual bool canContinueToUse();
	virtual void stop();
	virtual void tick();
	
	// 4J Added override to update ai elements when loading entity from schematics
	virtual void setLevel(Level *level) { this->level = level; }
};