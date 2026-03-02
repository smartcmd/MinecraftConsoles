#pragma once
using namespace std;

#include "Monster.h"

class Skeleton : public Monster
{
public:
	eINSTANCEOF GetType() { return eTYPE_SKELETON; }
	static Entity *create(Level *level) { return new Skeleton(level); }

	Skeleton(Level *level);

	virtual bool useNewAi();
	virtual int getMaxHealth();

protected:
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();

public:
    virtual shared_ptr<ItemInstance> getCarriedItem();
	virtual MobType getMobType();
	virtual void aiStep();
	virtual void die(DamageSource *source);

protected:
	virtual int getDeathLoot();
    virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);
	virtual void dropRareDeathLoot(int rareLootLevel);

private:
	static shared_ptr<ItemInstance> bow;

public:

	static void staticCtor();
};
