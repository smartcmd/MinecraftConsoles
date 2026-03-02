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
	static AttributeModifier *SPEED_MODIFIER_ATTACKING;

	int angerTime;
	int playAngrySoundIn;
	shared_ptr<Entity> lastAttackTarget;

	void _init();

public:
	PigZombie(Level *level);

protected:
	virtual void registerAttributes();
	virtual bool useNewAi();

public:
	virtual void tick();
	virtual bool canSpawn();
	virtual void addAdditonalSaveData(CompoundTag *tag);
	virtual void readAdditionalSaveData(CompoundTag *tag);

protected:
	virtual shared_ptr<Entity> findAttackTarget();

public:
	virtual bool hurt(DamageSource *source, float dmg);

private:
	void alert(shared_ptr<Entity> target);

protected:
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);

public:
	virtual bool mobInteract(shared_ptr<Player> player);

protected:
	virtual void dropRareDeathLoot(int rareLootLevel);
	virtual int getDeathLoot();
	virtual void populateDefaultEquipmentSlots();


public:
	virtual MobGroupData *finalizeMobSpawn(MobGroupData *groupData, int extraData = 0); // 4J Added extraData param
};
