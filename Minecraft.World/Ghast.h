#pragma once
using namespace std;

#include "FlyingMob.h"
#include "Enemy.h"

class GhastClass;
class Level;

class Ghast : public FlyingMob, public Enemy
{
public:
	eINSTANCEOF GetType() { return eTYPE_GHAST; }
	static Entity *create(Level *level) { return new Ghast(level); }

private:
	static const int DATA_IS_CHARGING = 16;

public:
	int floatDuration;
    double xTarget, yTarget, zTarget;

private: 
	shared_ptr<Entity> target;
    int retargetTime;

public:
	int oCharge;
    int charge;

private:
	void _init();

public:
	Ghast(Level *level);

	virtual bool hurt(DamageSource *source, int dmg);

protected:
	virtual void defineSynchedData();

public:
	int getMaxHealth();

public:
	virtual void tick();

protected:
	virtual void serverAiStep();

private:
	virtual bool canReach(double xt, double yt, double zt, double dist);

protected:
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
    virtual int getDeathLoot();
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);
    virtual float getSoundVolume();

public:
	virtual bool canSpawn();
    virtual int getMaxSpawnClusterSize();
};
