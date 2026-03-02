#pragma once
using namespace std;

#include "PathfinderMob.h"
#include "Enemy.h"

class Level;
class CompoundTag;
class DamageSource;

class Monster : public PathfinderMob, public Enemy
{
public:
	eINSTANCEOF GetType() { return eTYPE_MONSTER; }
	static Entity *create(Level *level) { return NULL; }

protected:
	int attackDamage;

private:
	void _init();

public:
	Monster(Level *level);

    virtual void aiStep();
    virtual void tick();

protected:
	virtual shared_ptr<Entity> findAttackTarget();

public:
	virtual bool hurt(DamageSource *source, int dmg);
	virtual bool doHurtTarget(shared_ptr<Entity> target);

protected:
	virtual void checkHurtTarget(shared_ptr<Entity> target, float distance);

public:
    virtual float getWalkTargetValue(int x, int y, int z);

protected:
    virtual bool isDarkEnoughToSpawn();

public:
	virtual bool canSpawn();
};

