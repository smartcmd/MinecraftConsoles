#pragma once
using namespace std;

#include "Mob.h"
#include "Enemy.h"
#include "ParticleTypes.h"

class Slime : public Mob, public Enemy
{
public:
	eINSTANCEOF GetType() { return eTYPE_SLIME; }
	static Entity *create(Level *level) { return new Slime(level); }

private:
	static const int ID_SIZE = 16;

public:
	float targetSquish;
	float squish;
	float oSquish;

private:
	int jumpDelay;

	void _init();

public:
	Slime(Level *level);

protected: 
	virtual void defineSynchedData();

public:
	using Entity::setSize;

	virtual void setSize(int size);
	virtual int getMaxHealth();
	virtual int getSize();
	virtual void addAdditonalSaveData(CompoundTag *tag);
	virtual void readAdditionalSaveData(CompoundTag *tag);

protected:
	virtual ePARTICLE_TYPE getParticleName();
	virtual int getSquishSound();

public:
	virtual void tick();

protected:
	virtual void serverAiStep();
	virtual void decreaseSquish();
	virtual int getJumpDelay();
	virtual shared_ptr<Slime> createChild();

public:
	virtual void remove();
	virtual void playerTouch(shared_ptr<Player> player);

protected:
	virtual bool isDealsDamage();
	virtual int getAttackDamage();
	virtual int getHurtSound();
	virtual int getDeathSound();
	virtual int getDeathLoot();

public:
	virtual bool canSpawn();

protected:
	virtual float getSoundVolume();

public:
	virtual int getMaxHeadXRot();

protected:
	virtual bool doPlayJumpSound();
	virtual bool doPlayLandSound();
};
