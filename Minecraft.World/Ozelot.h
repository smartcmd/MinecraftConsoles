#pragma once

#include "TamableAnimal.h"

class TemptGoal;

class Ozelot : public TamableAnimal
{
public:
	eINSTANCEOF GetType() { return eTYPE_OZELOT; }
	static Entity *create(Level *level) { return new Ozelot(level); }

public:
	static const float SNEAK_SPEED;
	static const float WALK_SPEED;
	static const float FOLLOW_SPEED;
	static const float SPRINT_SPEED;

private:
	static const int DATA_TYPE_ID;

	static const int TYPE_OZELOT;
	static const int TYPE_BLACK;
	static const int TYPE_RED;
	static const int TYPE_SIAMESE;

	TemptGoal *temptGoal;

public:
	Ozelot(Level *level);

protected:
	virtual void defineSynchedData();

public:
	virtual void serverAiMobStep();

protected:
	virtual bool removeWhenFarAway();

public:
	virtual int getTexture();
	virtual bool useNewAi();
	virtual int getMaxHealth();

protected:
	virtual void causeFallDamage(float distance);

public:
	virtual void addAdditonalSaveData(CompoundTag *tag);
	virtual void readAdditionalSaveData(CompoundTag *tag);

protected:
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
	virtual float getSoundVolume();
	virtual int getDeathLoot();

public:
	virtual bool doHurtTarget(shared_ptr<Entity> target);
	virtual bool hurt(DamageSource *source, int dmg);

protected:
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);

public:
	virtual bool interact(shared_ptr<Player> player);
	virtual shared_ptr<AgableMob> getBreedOffspring(shared_ptr<AgableMob> target);
	virtual bool isFood(shared_ptr<ItemInstance> itemInstance);
	virtual bool canMate(shared_ptr<Animal> animal);
	virtual int getCatType();
	virtual void setCatType(int type);
	virtual bool canSpawn();
	virtual wstring getAName();
};