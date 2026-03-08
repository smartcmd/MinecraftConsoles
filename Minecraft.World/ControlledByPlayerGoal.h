#pragma once

#include "Goal.h"
#include "../Minecraft.World/SharedConstants.h"

class Mob;

class ControlledByPlayerGoal : public Goal
{
private:
	static const int MIN_BOOST_TIME = SharedConstants::TICKS_PER_SECOND * 7;
	static const int MAX_BOOST_TIME = SharedConstants::TICKS_PER_SECOND * 35;

	Mob *mob; // Owner of this goal
	float maxSpeed;
	float walkSpeed;
	float speed;
	bool boosting;
	int boostTime;
	int boostTimeTotal;

public:
	ControlledByPlayerGoal(Mob *mob, float maxSpeed, float walkSpeed); // 4J Added walkSpeed param

	void start();
	void stop();
	bool canUse();
	void tick();

private:
	bool isNoJumpTile(int tile);

public:
	bool isBoosting();
	void boost();
	bool canBoost();
};