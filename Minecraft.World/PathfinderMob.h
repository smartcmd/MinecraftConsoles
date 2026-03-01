#pragma once

#include "Mob.h"

class Level;
class Path;

class PathfinderMob : public Mob
{
public:
	static AttributeModifier *SPEED_MODIFIER_FLEEING;

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

private:
	Pos *restrictCenter;
	float restrictRadius;
	Goal *leashRestrictionGoal;
	bool addedLeashRestrictionGoal;

protected:
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
	virtual bool isPathFinding();
	virtual void setPath(Path *path);
	virtual shared_ptr<Entity> getAttackTarget();
	virtual void setAttackTarget(shared_ptr<Entity> attacker);

	// might move to navigation, might make area
	virtual bool isWithinRestriction();
	virtual bool isWithinRestriction(int x, int y, int z);
	virtual void restrictTo(int x, int y, int z, int radius);
	virtual Pos *getRestrictCenter();
	virtual float getRestrictRadius();
	virtual void clearRestriction();
	virtual bool hasRestriction();

protected:
	void tickLeash();
	void onLeashDistance(float distanceToLeashHolder);

	// 4J added
public:
	virtual bool couldWander();
};
