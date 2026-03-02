#pragma once
using namespace std;

#include "LivingEntity.h"
#include "MobType.h"
#include "GoalSelector.h"

class HitResult;
class Level;
class CompoundTag;
class MobEffectInstance;
class DamageSource;
class MobEffect;
class LookControl;
class MoveControl;
class JumpControl;
class BodyControl;
class PathNavigation;
class Sensing;
class Icon;
class Pos;
class MobGroupData;

class Mob : public LivingEntity
{
	friend class MobSpawner;
public:
	// 4J-PB - added to replace (e instanceof Type), avoiding dynamic casts
	eINSTANCEOF GetType()						{ return eTYPE_MOB;}
	static Entity *create(Level *level) { return NULL; }

public:
	static const float MAX_WEARING_ARMOR_CHANCE;
	static const float MAX_PICKUP_LOOT_CHANCE;
	static const float MAX_ENCHANTED_ARMOR_CHANCE;
	static const float MAX_ENCHANTED_WEAPON_CHANCE;

private:
	static const int DATA_CUSTOM_NAME = 10;
	static const int DATA_CUSTOM_NAME_VISIBLE = 11;

public:
	int ambientSoundTime;

protected:
	int xpReward;

private:
	LookControl *lookControl;
	MoveControl *moveControl;
	JumpControl *jumpControl;
	BodyControl *bodyControl;
	PathNavigation *navigation;

protected:
	GoalSelector goalSelector;
	GoalSelector targetSelector;

private:
	shared_ptr<LivingEntity> target;
	Sensing *sensing;

	ItemInstanceArray equipment;

protected:
	floatArray dropChances;

private:
	bool _canPickUpLoot;
	bool persistenceRequired;

protected:
	// 4J - added for common ctor code
	void _init();

public:
	Mob(Level* level);
	virtual ~Mob();

protected:
	void registerAttributes();

public:
	virtual LookControl *getLookControl();
	virtual MoveControl *getMoveControl();
	virtual JumpControl *getJumpControl();
	virtual PathNavigation *getNavigation();
	virtual Sensing *getSensing();
	shared_ptr<LivingEntity> getTarget();
	virtual void setTarget(shared_ptr<LivingEntity> target);
	virtual bool canAttackType(eINSTANCEOF targetType);
	virtual void ate();

protected:
	virtual void defineSynchedData();

public:
	virtual int getAmbientSoundInterval();
	void playAmbientSound();
	virtual void baseTick();

protected:
	virtual int getExperienceReward(shared_ptr<Player> killedBy);

public:
	virtual void spawnAnim();
	virtual void tick();

protected:
	virtual float tickHeadTurn(float yBodyRotT, float walkSpeed);
	virtual int getAmbientSound();
	virtual int getDeathLoot();
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);

public:
	virtual void addAdditonalSaveData(CompoundTag *entityTag);
	virtual void readAdditionalSaveData(CompoundTag *tag);

protected:
	float defaultLookAngle;

public:
	virtual void setYya(float yya);
	virtual void setSpeed(float speed);
	virtual void aiStep();

protected:
	virtual bool useNewAi();
	virtual bool removeWhenFarAway();

private:
	shared_ptr<Entity> lookingAt;

protected:
	int lookTime;

	virtual void checkDespawn();
	virtual void newServerAiStep();
	virtual void serverAiStep();

public:
	virtual int getMaxHeadXRot();

protected:
	void lookAt(shared_ptr<Entity> e, float yMax, float xMax);
	bool isLookingAtAnEntity();
	shared_ptr<Entity> getLookingAt();

private:
	float rotlerp(float a, float b, float max);

public:
	virtual bool canSpawn();
	virtual float getSizeScale();
	virtual float getHeadSizeScale();
	virtual int getMaxSpawnClusterSize();
	virtual int getMaxFallDistance();
	virtual shared_ptr<ItemInstance> getCarriedItem();
	virtual shared_ptr<ItemInstance> getCarried(int slot);
	virtual shared_ptr<ItemInstance> getArmor(int pos);
	virtual void setEquippedSlot(int slot, shared_ptr<ItemInstance> item);
	virtual ItemInstanceArray getEquipmentSlots();

protected:
	virtual void dropEquipment(bool byPlayer, int playerBonusLevel);
	virtual void populateDefaultEquipmentSlots();

public:
	static int getEquipmentSlotForItem(shared_ptr<ItemInstance> item);
	static Item *getEquipmentForSlot(int slot, int type);

protected:
	virtual void populateDefaultEquipmentEnchantments();

public:
	/**
	* Added this method so mobs can handle their own spawn settings instead of
	* hacking MobSpawner.java
	* 
	* @param groupData
	*            TODO
	* @return TODO
	*/
	virtual MobGroupData *finalizeMobSpawn(MobGroupData *groupData, int extraData = 0); // 4J Added extraData param
	virtual void finalizeSpawnEggSpawn(int extraData); // 4J Added
	virtual bool canBeControlledByRider();
	virtual wstring getAName();
	virtual void setPersistenceRequired();
	virtual void setCustomName(const wstring &name);
	virtual wstring getCustomName();
	virtual bool hasCustomName();
	virtual void setCustomNameVisible(bool visible);
	virtual bool isCustomNameVisible();
	virtual bool shouldShowName();
	virtual void setDropChance(int slot, float pct);
	virtual bool canPickUpLoot();
	virtual void setCanPickUpLoot(bool canPickUpLoot);
	virtual bool isPersistenceRequired();
	virtual bool interact(shared_ptr<Player> player);

protected:
	virtual bool mobInteract(shared_ptr<Player> player);

	// roper / leash methods

private:
	bool _isLeashed;
	shared_ptr<Entity> leashHolder;
	CompoundTag *leashInfoTag;

protected:
	virtual void tickLeash();

public:
	virtual void dropLeash(bool synch, bool createItemDrop);
	virtual bool canBeLeashed();
	virtual bool isLeashed();
	virtual shared_ptr<Entity> getLeashHolder();
	virtual void setLeashedTo(shared_ptr<Entity> holder, bool synch);

private:
	virtual void restoreLeashFromSave();
	virtual bool shouldRender(Vec3 *c);


public:

	// 4J Added override to update ai elements when loading entity from schematics
	virtual void setLevel(Level *level);
};
