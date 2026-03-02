#pragma once

#include "TamableAnimal.h"

class TemptGoal;

class Ocelot : public TamableAnimal
{
	friend class OcelotSitOnTileGoal;

public:
	eINSTANCEOF GetType() { return eTYPE_OCELOT; }
	static Entity *create(Level *level) { return new Ocelot(level); }

public:
	static const double SNEAK_SPEED_MOD;
	static const double WALK_SPEED_MOD;
	static const double FOLLOW_SPEED_MOD;
	static const double SPRINT_SPEED_MOD;

private:
	static const int DATA_TYPE_ID;

public:
	enum
	{
		TYPE_OCELOT,
		TYPE_BLACK,
		TYPE_RED,
		TYPE_SIAMESE,
	};

private:
	TemptGoal *temptGoal;

public:
	Ocelot(Level *level);

protected:
	virtual void defineSynchedData();

public:
	virtual void serverAiMobStep();

protected:
	virtual bool removeWhenFarAway();

public:
	virtual bool useNewAi();

protected:
	virtual void registerAttributes();
	virtual void causeFallDamage(float distance);

public:
	virtual void addAdditonalSaveData(CompoundTag *tag);
	virtual void readAdditionalSaveData(CompoundTag *tag);

protected:
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
	virtual float getSoundVolume();
	virtual int getDeathLoot();

public:
	virtual bool doHurtTarget(shared_ptr<Entity> target);
	virtual bool hurt(DamageSource *source, float dmg);

protected:
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);

public:
	virtual bool mobInteract(shared_ptr<Player> player);
	virtual shared_ptr<AgableMob> getBreedOffspring(shared_ptr<AgableMob> target);
	virtual bool isFood(shared_ptr<ItemInstance> itemInstance);
	virtual bool canMate(shared_ptr<Animal> animal);
	virtual int getCatType();
	virtual void setCatType(int type);
	virtual bool canSpawn();
	virtual wstring getAName();
	virtual MobGroupData *finalizeMobSpawn(MobGroupData *groupData, int extraData = 0); // 4J Added extraData param


	// 4J-JEV: Added for tooltips, is cat annoying player by sitting on chest or furnace.
private:
	void setSittingOnTile(bool val);
public:
	bool isSittingOnTile();
};