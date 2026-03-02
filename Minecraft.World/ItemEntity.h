#pragma once
#include "Entity.h"
#include "SharedConstants.h"

class Player;

class ItemEntity : public Entity
{
public:
	eINSTANCEOF GetType()			{ return eTYPE_ITEMENTITY;}
	static Entity *create(Level *level) { return new ItemEntity(level); }

private:
	static const int DATA_ITEM = 10;

	static const int LIFETIME = 5 * 60 * SharedConstants::TICKS_PER_SECOND; // Five miniutes.

	wstring thrower;

	// 4J Added
	void _init();
	void _init(Level *level, double x, double y, double z);

public:
	int age;
	int throwTime;

private:
	int health;

public:
	float bobOffs;
	
	ItemEntity(Level *level, double x, double y, double z);
	ItemEntity(Level *level, double x, double y, double z, shared_ptr<ItemInstance> item);

protected:
	virtual bool makeStepSound();

public:
	ItemEntity(Level *level);

protected:
	virtual void defineSynchedData();

public:
	virtual void tick();

private:
	void mergeWithNeighbours();

public:
	bool merge(shared_ptr<ItemEntity> target);
	void setShortLifeTime();
	virtual bool updateInWaterState();

protected:
	virtual void burn(int dmg);

public:
	virtual bool hurt(DamageSource *source, int damage);
	virtual void addAdditonalSaveData(CompoundTag *entityTag);
	virtual void readAdditionalSaveData(CompoundTag *tag);
	virtual void playerTouch(shared_ptr<Player> player);

	virtual wstring getAName();

	shared_ptr<ItemInstance> getItem();
	void setItem(shared_ptr<ItemInstance> item);
	virtual bool isAttackable();

	void setThrower(const wstring &thrower);
	wstring getThrower();
};
