#pragma once

#include "Monster.h"

class Blaze : public Monster
{
public:
	eINSTANCEOF GetType() { return eTYPE_BLAZE; }
	static Entity *create(Level *level) { return new Blaze(level); }

	//    private int nextBurnNeighborsTick;
private:
	float allowedHeightOffset;
	int nextHeightOffsetChangeTick;
	int attackCounter;

	static const int DATA_FLAGS_ID = 16;

public:
	Blaze(Level *level);

protected:
	virtual void registerAttributes();
	virtual void defineSynchedData();
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();

public:
	virtual int getLightColor(float a);
	virtual float getBrightness(float a);
	virtual void aiStep();

protected:
	virtual void checkHurtTarget(shared_ptr<Entity> target, float d);
	virtual void causeFallDamage(float distance);
	virtual int getDeathLoot();

public:
	virtual bool isOnFire();

protected:
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);

public:
	bool isCharged();
	void setCharged(bool value);

protected:
	bool isDarkEnoughToSpawn();
};