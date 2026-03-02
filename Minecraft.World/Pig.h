#pragma once

using namespace std;

#include "Animal.h"

class Player;
class LightningBolt;
class ControlledByPlayerGoal;

class Pig : public Animal
{
public:
	eINSTANCEOF GetType() { return eTYPE_PIG; }
	static Entity *create(Level *level) { return new Pig(level); }
private:
	static const int DATA_SADDLE_ID = 16;
	ControlledByPlayerGoal *controlGoal;

public:
	Pig(Level *level);

	virtual bool useNewAi();
	virtual int getMaxHealth();
	virtual bool canBeControlledByRider();

protected:
	virtual void defineSynchedData();

public:
	virtual void addAdditonalSaveData(CompoundTag *tag);
	virtual void readAdditionalSaveData(CompoundTag *tag);

protected:
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();

public:
	virtual bool interact(shared_ptr<Player> player);

protected:
	virtual int getDeathLoot();
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);

public:
	bool hasSaddle();
	void setSaddle(bool value);
	virtual void thunderHit(const LightningBolt *lightningBolt);

protected:
	virtual void causeFallDamage(float distance);

public:
	virtual shared_ptr<AgableMob> getBreedOffspring(shared_ptr<AgableMob> target);
	bool isFood(shared_ptr<ItemInstance> itemInstance);
	ControlledByPlayerGoal *getControlGoal();
};
