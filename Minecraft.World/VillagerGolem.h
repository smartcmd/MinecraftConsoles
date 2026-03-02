#pragma once

#include "Golem.h"

class Village;
class Level;

class VillagerGolem : public Golem
{
public:
	eINSTANCEOF GetType() { return eTYPE_VILLAGERGOLEM; }
	static Entity *create(Level *level) { return new VillagerGolem(level); }

protected:
	static const int DATA_FLAGS_ID = 16;

private:
	int villageUpdateInterval;
	weak_ptr<Village> village;
	int attackAnimationTick;
	int offerFlowerTick;

public:
	VillagerGolem(Level *level);

protected:
	virtual void defineSynchedData();

public:
	virtual bool useNewAi();

protected:
	virtual void serverAiMobStep();

public:
	virtual int getMaxHealth();

protected:
	virtual int decreaseAirSupply(int currentSupply);

public:
	virtual void aiStep();
	virtual bool canAttackType(eINSTANCEOF targetType);
	virtual void addAdditonalSaveData(CompoundTag *tag);
	virtual void readAdditionalSaveData(CompoundTag *tag);
	virtual bool doHurtTarget(shared_ptr<Entity> target);
	virtual void handleEntityEvent(byte id);
	virtual shared_ptr<Village> getVillage();
	virtual int getAttackAnimationTick();
	virtual void offerFlower(bool offer);

protected:
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
	virtual void playStepSound(int xt, int yt, int zt, int t);
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);

public:
	virtual int getOfferFlowerTick();
	virtual bool isPlayerCreated();
	virtual void setPlayerCreated(bool value);
	virtual void die(DamageSource *source);
};