#pragma once

#include "Monster.h"
#include "RangedAttackMob.h"
#include "BossMob.h"

class LivingEntitySelector : public EntitySelector
{
public:
	virtual bool matches(shared_ptr<Entity> entity) const;
};

class WitherBoss : public Monster, public RangedAttackMob, public BossMob
{
public:
	eINSTANCEOF GetType() { return eTYPE_WITHERBOSS; };
	static Entity *create(Level *level) { return new WitherBoss(level); }

private:
	static const int DATA_TARGET_A = 17;
	static const int DATA_TARGET_B = 18;
	static const int DATA_TARGET_C = 19;
	static const int DATA_ID_INV = 20;

private:
	static const int IDLE_HEAD_UPDATES_SIZE = 2;
	float xRotHeads[2];
	float yRotHeads[2];
	float xRotOHeads[2];
	float yRotOHeads[2];
	int nextHeadUpdate[2];
	int idleHeadUpdates[IDLE_HEAD_UPDATES_SIZE];
	int destroyBlocksTick;

	static EntitySelector *livingEntitySelector; 

public:
	WitherBoss(Level *level);

protected:
	virtual void defineSynchedData();

public:
	virtual void addAdditonalSaveData(CompoundTag *entityTag);
	virtual void readAdditionalSaveData(CompoundTag *tag);
	virtual float getShadowHeightOffs();

protected:
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();

public:
	virtual void aiStep();

protected:
	virtual void newServerAiStep();

public:
	virtual void makeInvulnerable();
	virtual void makeStuckInWeb();
	virtual int getArmorValue();

private:
	virtual double getHeadX(int index);
	virtual double getHeadY(int index);
	virtual double getHeadZ(int index);
	virtual float rotlerp(float a, float b, float max);
	virtual void performRangedAttack(int head, shared_ptr<LivingEntity> target);
	virtual void performRangedAttack(int head, double tx, double ty, double tz, bool dangerous);

public:
	virtual void performRangedAttack(shared_ptr<LivingEntity> target, float power);
	virtual bool hurt(DamageSource *source, float dmg);

protected:
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);
	virtual void checkDespawn();

public:
	virtual int getLightColor(float a);
	virtual bool isPickable();

protected:
	virtual void causeFallDamage(float distance);

public:
	virtual void addEffect(MobEffectInstance *newEffect);

protected:
	virtual bool useNewAi();
	virtual void registerAttributes();

public:
	virtual float getHeadYRot(int i);
	virtual float getHeadXRot(int i);
	virtual int getInvulnerableTicks();
	virtual void setInvulnerableTicks(int invulnerableTicks);
	virtual int getAlternativeTarget(int headIndex);
	virtual void setAlternativeTarget(int headIndex, int entityId);
	virtual bool isPowered();
	virtual MobType getMobType();
	virtual void ride(shared_ptr<Entity> e);

	// 4J Stu - These are required for the BossMob interface
	virtual float getMaxHealth() { return Monster::getMaxHealth(); };
	virtual float getHealth() { return Monster::getHealth(); };
	virtual wstring getAName() { return app.GetString(IDS_WITHER); };
};