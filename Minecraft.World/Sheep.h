#pragma once

using namespace std;

class Random;
class EatTileGoal;
class CraftingContainer;

#include "Animal.h"
#include "SharedConstants.h"
#include "AbstractContainerMenu.h"

class Sheep : public Animal
{
private:
	class SheepContainer : public AbstractContainerMenu
	{
		bool stillValid(shared_ptr<Player> player) { return false; }
	};

	shared_ptr<CraftingContainer> container;
public:
	eINSTANCEOF GetType() { return eTYPE_SHEEP; }
	static Entity *create(Level *level) { return new Sheep(level); }

private:
	static const int EAT_ANIMATION_TICKS = SharedConstants::TICKS_PER_SECOND * 2;
	static const int DATA_WOOL_ID = 16;

	int eatAnimationTick;
	EatTileGoal *eatTileGoal;

public:
	static const float COLOR[][3];

public:
	Sheep(Level *level);

protected:
	virtual bool useNewAi();
	virtual void newServerAiStep();

public:
	void aiStep();
	virtual int getMaxHealth();

protected:
	virtual void defineSynchedData();

public:
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);
	virtual int getDeathLoot();

public:
	virtual void handleEntityEvent(byte id);

public:
	float getHeadEatPositionScale(float a);
	float getHeadEatAngleScale(float a);

	virtual bool interact(shared_ptr<Player> player);
	virtual void addAdditonalSaveData(CompoundTag *tag);
	virtual void readAdditionalSaveData(CompoundTag *tag);

protected:
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();

public:
	int getColor();
	void setColor(int color);
	bool isSheared();
	void setSheared(bool value);

	static int getSheepColor(Random *random);
	virtual shared_ptr<AgableMob> getBreedOffspring(shared_ptr<AgableMob> target);

	virtual void ate();

	void finalizeMobSpawn();

private:
	int getOffspringColor(shared_ptr<Animal> animal, shared_ptr<Animal> partner);
	int getDyeColor(shared_ptr<Animal> animal);
};
