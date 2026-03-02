#pragma once
using namespace std;

#include "Monster.h"
#include "RangedAttackMob.h"

class RangedAttackGoal;
class MeleeAttackGoal;

class Skeleton : public Monster, public RangedAttackMob
{
public:
	eINSTANCEOF GetType() { return eTYPE_SKELETON; }
	static Entity *create(Level *level) { return new Skeleton(level); }

private:
	static const int DATA_TYPE_ID = 13;

public:
	static const int TYPE_DEFAULT = 0;
	static const int TYPE_WITHER = 1;

private:
	RangedAttackGoal *bowGoal;
	MeleeAttackGoal *meleeGoal;

public:
	Skeleton(Level *level);
	virtual ~Skeleton();

protected:
	virtual void registerAttributes();
	virtual void defineSynchedData();

public:
	virtual bool useNewAi();

protected:
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
	virtual void playStepSound(int xt, int yt, int zt, int t);

public:
	virtual bool doHurtTarget(shared_ptr<Entity> target);

public:
	virtual MobType getMobType();
	virtual void aiStep();
	virtual void rideTick();
	virtual void die(DamageSource *source);

protected:
	virtual int getDeathLoot();
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);
	virtual void dropRareDeathLoot(int rareLootLevel);
	virtual void populateDefaultEquipmentSlots();

public:
	virtual MobGroupData *finalizeMobSpawn(MobGroupData *groupData, int extraData = 0); // 4J Added extraData param
	virtual void reassessWeaponGoal();
	virtual void performRangedAttack(shared_ptr<LivingEntity> target, float power);
	virtual int getSkeletonType();
	virtual void setSkeletonType(int type);
	virtual void readAdditionalSaveData(CompoundTag *tag);
	virtual void addAdditonalSaveData(CompoundTag *entityTag);
	virtual void setEquippedSlot(int slot, shared_ptr<ItemInstance> item);
	virtual double getRidingHeight();
};
