#pragma once
using namespace std;

#include "Zombie.h"

class DamageSource;

// SKIN BY XaPhobia Chris Beidler  
class PigZombie : public Zombie
{
public:
	eINSTANCEOF GetType() { return eTYPE_PIGZOMBIE; }
	static Entity *create(Level *level) { return new PigZombie(level); }

private:
	int angerTime;
    int playAngrySoundIn;

	void _init();

public:
	PigZombie(Level *level);

protected:
	bool useNewAi();

public:
	virtual int getTexture();
    virtual void tick();
    virtual bool canSpawn();
    virtual void addAdditonalSaveData(CompoundTag *tag);
    virtual void readAdditionalSaveData(CompoundTag *tag);

protected:
	virtual shared_ptr<Entity> findAttackTarget();

public:
    virtual bool hurt(DamageSource *source, int dmg);

private:
	void alert(shared_ptr<Entity> target);

protected:
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);
	virtual void dropRareDeathLoot(int rareLootLevel);
    virtual int getDeathLoot();

private:
	static shared_ptr<ItemInstance> sword;

public:
	virtual void finalizeMobSpawn();

	shared_ptr<ItemInstance> getCarriedItem();

	static void staticCtor();
};
