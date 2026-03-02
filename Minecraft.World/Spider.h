#pragma once
using namespace std;

#include "Monster.h"

class Spider : public Monster
{
public:
	eINSTANCEOF GetType() { return eTYPE_SPIDER; }
	static Entity *create(Level *level) { return new Spider(level); }

private:
	static const int DATA_FLAGS_ID = 16;

public:
    Spider(Level *level);

protected:
	virtual void defineSynchedData();

public:
	virtual void tick();
	virtual int getMaxHealth();
    virtual double getRideHeight();

protected:
	virtual bool makeStepSound();
	virtual shared_ptr<Entity> findAttackTarget();
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
    virtual void checkHurtTarget(shared_ptr<Entity> target, float d);
	virtual int getDeathLoot();
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);

public:
	virtual bool onLadder();
	
	virtual void makeStuckInWeb();
	virtual float getModelScale();
	virtual MobType getMobType();
	virtual bool canBeAffected(MobEffectInstance *newEffect);
	virtual bool isClimbing();
	virtual void setClimbing(bool value);
};
