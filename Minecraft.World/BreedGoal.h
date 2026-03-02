#pragma once

#include "Goal.h"

class Animal;
class Level;

class BreedGoal : public Goal
{
private:
	Animal *animal; // Owner of this goal
	Level *level;
	weak_ptr<Animal> partner;
	int loveTime;
	float speed;

public:
	BreedGoal(Animal *animal, float speed);

	virtual bool canUse();
	virtual bool canContinueToUse();
	virtual void stop();
	virtual void tick();

private:
	shared_ptr<Animal> getFreePartner();
	void breed();

public:
	// 4J Added override to update ai elements when loading entity from schematics
	virtual void setLevel(Level *level) { this->level = level; }
};