#pragma once

#include "Entity.h"

class ExperienceOrb :  public Entity
{
public:
	virtual eINSTANCEOF GetType() { return eTYPE_EXPERIENCEORB; }
	static Entity *create(Level *level) { return new ExperienceOrb(level); }

private:
	static const int LIFETIME;

public:
	int tickCount;
	int age;

	int throwTime;

private:
	int health;
	int value;
	shared_ptr<Player> followingPlayer;
	int followingTime;

	void _init();

public:
	ExperienceOrb(Level *level, double x, double y, double z, int count);

protected:
	virtual bool makeStepSound();

public:
	ExperienceOrb(Level *level);

protected:
	virtual void defineSynchedData();

public:
	virtual int getLightColor(float a);
	virtual void tick();
	virtual bool updateInWaterState();

protected:
	virtual void burn(int dmg);

public:
	virtual bool hurt(DamageSource *source, int damage);
	virtual void addAdditonalSaveData(CompoundTag *entityTag);
	virtual void readAdditionalSaveData(CompoundTag *tag);
	virtual void playerTouch(shared_ptr<Player> player);
	int getValue();
	int getIcon();

	static int getExperienceValue(int maxValue);
	virtual bool isAttackable();

	virtual bool shouldRender(Vec3 *c);	// 4J added
};