#pragma once
using namespace std;

#include "Monster.h"
#include "SharedConstants.h"

class Zombie : public Monster
{
private:
	static const int VILLAGER_CONVERSION_WAIT_MIN = SharedConstants::TICKS_PER_SECOND * 60 * 3;
	static const int VILLAGER_CONVERSION_WAIT_MAX = SharedConstants::TICKS_PER_SECOND * 60 * 5;

	static const int DATA_BABY_ID = 12;
	static const int DATA_VILLAGER_ID = 13;
	static const int DATA_CONVERTING_ID = 14;

	int villagerConversionTime;

	float registeredBBWidth;
	float registeredBBHeight;

public:
	static const int MAX_SPECIAL_BLOCKS_COUNT = 14;
	static const int SPECIAL_BLOCK_RADIUS = 4;

public:
	eINSTANCEOF GetType() { return eTYPE_ZOMBIE; }
	static Entity *create(Level *level) { return new Zombie(level); }

	Zombie(Level *level);
	virtual float getWalkingSpeedModifier();

protected:
	virtual void defineSynchedData();

public:
	virtual int getTexture();
	virtual int getMaxHealth();
	int getArmorValue();

protected:
	virtual bool useNewAi();

public:
	bool isBaby();
	void setBaby(bool baby);
	bool isVillager();
	void setVillager(bool villager);
    virtual void aiStep();
    virtual void tick();
	
protected:
	virtual int getAmbientSound();
    virtual int getHurtSound();
    virtual int getDeathSound();
    virtual int getDeathLoot();

public:
	MobType getMobType();

protected:
	virtual void dropRareDeathLoot(int rareLootLevel);

public:
	virtual void addAdditonalSaveData(CompoundTag *tag);
	virtual void readAdditionalSaveData(CompoundTag *tag);
	void killed(shared_ptr<Mob> mob);
	virtual void finalizeMobSpawn();
	bool interact(shared_ptr<Player> player);

protected:
	void startConverting(int time);

public:
	void handleEntityEvent(byte id);
	bool isConverting();

protected:
	void finishConversion();
	int getConversionProgress();

public:
	virtual void updateSize(bool isBaby);

protected:
	virtual void setSize(float w, float h);
	void internalSetSize(float scale);
};
