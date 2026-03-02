#pragma once

#include "Mob.h"

class Level;
class Path;

class PathfinderMob : public Mob
{
private:
	static const int MAX_TURN = 30;

public:
	PathfinderMob(Level *level);
	virtual ~PathfinderMob();

private:
	Path *path;

protected:
	shared_ptr<Entity> attackTarget;
	bool holdGround;
	int fleeTime;

	virtual bool shouldHoldGround();
	virtual void serverAiStep();
	virtual void findRandomStrollLocation(int quadrant = -1);
	virtual void checkHurtTarget(shared_ptr<Entity> target, float d);

public:
	virtual float getWalkTargetValue(int x, int y, int z);

protected:
	virtual shared_ptr<Entity> findAttackTarget();

public:
	virtual bool canSpawn();
	bool isPathFinding();
	void setPath(Path *path);
	shared_ptr<Entity> getAttackTarget();
	void setAttackTarget(shared_ptr<Entity> attacker);

protected:
	float getWalkingSpeedModifier();

	// 4J added
public:
	virtual bool couldWander();
};
