#pragma once

#include "Monster.h"

class EnderMan : public Monster
{
public:
	eINSTANCEOF GetType() { return eTYPE_ENDERMAN; }
	static Entity *create(Level *level) { return new EnderMan(level); }
public:
	static void staticCtor();
private:
	static bool MAY_TAKE[256];

	static const int DATA_CARRY_ITEM_ID = 16;
	static const int DATA_CARRY_ITEM_DATA = 17;
	static const int DATA_CREEPY = 18;

private:
	int teleportTime;
	int aggroTime;

public:
	EnderMan(Level *level);

	virtual int getMaxHealth();

protected:
	virtual void defineSynchedData();

public:
	virtual void addAdditonalSaveData(CompoundTag *tag);
	virtual void readAdditionalSaveData(CompoundTag *tag);

protected:
	virtual shared_ptr<Entity> findAttackTarget();

private:
	bool isLookingAtMe(shared_ptr<Player> player);

public:
	virtual void aiStep();

protected:
	bool teleport();
	bool teleportTowards(shared_ptr<Entity> e);
	bool teleport(double xx, double yy, double zz);

	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
	virtual int getDeathLoot();
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);

public:
	// 4J Brought forward from 1.2.3 to help fix Enderman behaviour
	void setCarryingTile(int carryingTile);
	int getCarryingTile();
	void setCarryingData(int carryingData);
	int getCarryingData();
	virtual bool hurt(DamageSource *source, int damage);
	bool isCreepy();
	void setCreepy(bool creepy);
};