#pragma once

#include "Monster.h"
#include "RangedAttackMob.h"

class Witch : public Monster, public RangedAttackMob
{
public:
	eINSTANCEOF GetType() { return eTYPE_WITCH; }
	static Entity *create(Level *level) { return new Witch(level); }

private:
	static AttributeModifier *SPEED_MODIFIER_DRINKING;

	static const int DATA_USING_ITEM = 21;
	static const int DEATH_LOOT_COUNT = 8;
	static const int DEATH_LOOT[DEATH_LOOT_COUNT];

	int usingTime;

public:
	Witch(Level *level);

protected:
	virtual void defineSynchedData();
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();

public:
	virtual void setUsingItem(bool isUsing);
	virtual bool isUsingItem();

protected:
	virtual void registerAttributes();

public:
	virtual bool useNewAi();
	virtual void aiStep();
	virtual void handleEntityEvent(byte id);

protected:
	virtual float getDamageAfterMagicAbsorb(DamageSource *damageSource, float damage);
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);

public:
	virtual void performRangedAttack(shared_ptr<LivingEntity> target, float power);
};