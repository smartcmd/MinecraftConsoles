#pragma once

#include "AmbientCreature.h"

class Bat : public AmbientCreature
{
public:
	eINSTANCEOF GetType() { return eTYPE_BAT; }
	static Entity *create(Level *level) { return new Bat(level); }

private:
	static const int DATA_ID_FLAGS = 16;
	static const int FLAG_RESTING = 1;

	Pos *targetPosition;

public:
	Bat(Level *level);

protected:
	virtual void defineSynchedData(); 
	virtual float getSoundVolume();
	virtual float getVoicePitch();
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();

public:
	virtual bool isPushable();

protected:
	virtual void doPush(shared_ptr<Entity> e);
	virtual void pushEntities();
	virtual void registerAttributes();

public:
	virtual bool isResting();
	virtual void setResting(bool value);

protected:
	virtual bool useNewAi();

public:
	virtual void tick();

protected:
	virtual void newServerAiStep();
	virtual bool makeStepSound();
	virtual void causeFallDamage(float distance);
	virtual void checkFallDamage(double ya, bool onGround);
	virtual bool isIgnoringTileTriggers();

public:
	virtual bool hurt(DamageSource *source, float dmg);
	virtual void readAdditionalSaveData(CompoundTag *tag);
	virtual void addAdditonalSaveData(CompoundTag *entityTag);
	virtual bool canSpawn();
};