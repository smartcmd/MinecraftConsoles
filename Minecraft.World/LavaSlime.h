#pragma once

#include "Slime.h"

class LavaSlime : public Slime
{
public:
	eINSTANCEOF GetType() { return eTYPE_LAVASLIME; }
	static Entity *create(Level *level) { return new LavaSlime(level); }

public:
	LavaSlime(Level *level);

	virtual bool canSpawn();
	virtual int getArmorValue();

public:
	virtual int getLightColor(float a);
	virtual float getBrightness(float a);

protected:
	virtual ePARTICLE_TYPE getParticleName();
	virtual shared_ptr<Slime> createChild();
	virtual int getDeathLoot();
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);

public:
	bool isOnFire();

protected:
	int getJumpDelay();
	virtual void decreaseSquish();
	virtual void jumpFromGround();
	virtual void causeFallDamage(float distance);
	virtual bool isDealsDamage();
	virtual int getAttackDamage();
	virtual int getHurtSound();
	virtual int getDeathSound();
	virtual int getSquishSound();

public:
	virtual bool isInLava();

protected:
	virtual bool doPlayLandSound();
};