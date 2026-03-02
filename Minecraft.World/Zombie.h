#pragma once
using namespace std;

#include "Monster.h"
#include "SharedConstants.h"
#include "MobGroupData.h"

class Zombie : public Monster
{
private:
	static const int VILLAGER_CONVERSION_WAIT_MIN = SharedConstants::TICKS_PER_SECOND * 60 * 3;
	static const int VILLAGER_CONVERSION_WAIT_MAX = SharedConstants::TICKS_PER_SECOND * 60 * 5;

protected:
	static Attribute *SPAWN_REINFORCEMENTS_CHANCE;

private:
	static AttributeModifier *SPEED_MODIFIER_BABY;


	static const int DATA_BABY_ID = 12;
	static const int DATA_VILLAGER_ID = 13;
	static const int DATA_CONVERTING_ID = 14;

public:
	static const float ZOMBIE_LEADER_CHANCE;
	static const int REINFORCEMENT_ATTEMPTS = 50;
	static const int REINFORCEMENT_RANGE_MAX = 40;
	static const int REINFORCEMENT_RANGE_MIN = 7;

private:
	int villagerConversionTime;

public:
	static const int MAX_SPECIAL_BLOCKS_COUNT = 14;
	static const int SPECIAL_BLOCK_RADIUS = 4;

public:
	eINSTANCEOF GetType() { return eTYPE_ZOMBIE; }
	static Entity *create(Level *level) { return new Zombie(level); }

	Zombie(Level *level);

protected:
	virtual void registerAttributes();
	virtual void defineSynchedData();

public:
	virtual int getArmorValue();

protected:
	virtual bool useNewAi();

public:
	virtual bool isBaby();
	virtual void setBaby(bool baby);
	virtual bool isVillager();
	virtual void setVillager(bool villager);
	virtual void aiStep();
	virtual bool hurt(DamageSource *source, float dmg);
	virtual void tick();
	virtual bool doHurtTarget(shared_ptr<Entity> target);

protected:
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
	virtual int getDeathLoot();
	virtual void playStepSound(int xt, int yt, int zt, int t);

public:
	virtual MobType getMobType();

protected:
	virtual void dropRareDeathLoot(int rareLootLevel);
	virtual void populateDefaultEquipmentSlots();

public:
	virtual void addAdditonalSaveData(CompoundTag *tag);
	virtual void readAdditionalSaveData(CompoundTag *tag);
	virtual void killed(shared_ptr<LivingEntity> mob);
	virtual MobGroupData *finalizeMobSpawn(MobGroupData *groupData, int extraData = 0); // 4J Added extraData param
	virtual bool mobInteract(shared_ptr<Player> player);

protected:
	virtual void startConverting(int time);

public:
	virtual void handleEntityEvent(byte id);

protected:
	virtual bool removeWhenFarAway();

public:
	virtual bool isConverting();

protected:
	virtual void finishConversion();
	virtual int getConversionProgress();

private:
	class ZombieGroupData : public MobGroupData
	{
	public:
		bool isBaby;
		bool isVillager;

		ZombieGroupData(bool baby, bool villager);
	};
};
