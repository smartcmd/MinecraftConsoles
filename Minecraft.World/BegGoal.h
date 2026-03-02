#pragma once

#include "Goal.h"

class Wolf;

class BegGoal : public Goal
{
private:
	Wolf *wolf; // Owner of this goal
	weak_ptr<Player> player;
	Level *level;
	float lookDistance;
	int lookTime;

public:
	BegGoal(Wolf *wolf, float lookDistance);

	virtual bool canUse();
	virtual bool canContinueToUse();
	virtual void start();
	virtual void stop();
	virtual void tick();

private:
	bool playerHoldingInteresting(shared_ptr<Player> player);

public:
	// 4J Added override to update ai elements when loading entity from schematics
	virtual void setLevel(Level *level) { this->level = level; }
};