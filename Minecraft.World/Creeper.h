#pragma once
using namespace std;

#include "Monster.h"

class Level;
class DamageSource;

class Creeper : public Monster
{
public:
	eINSTANCEOF GetType() { return eTYPE_CREEPER; }
	static Entity *create(Level *level) { return new Creeper(level); }

private:
	static const int DATA_SWELL_DIR = 16;
	static const int DATA_IS_POWERED = 17;

	int oldSwell;
	int swell;
	int maxSwell;
	int explosionRadius;

	void _init();

public:
	Creeper(Level *level);

protected:
	void registerAttributes();

public:
	virtual bool useNewAi();

	virtual int getMaxFallDistance();

protected:
	virtual void causeFallDamage(float distance);
	virtual void defineSynchedData();

public:
	virtual void addAdditonalSaveData(CompoundTag *entityTag);
	virtual void readAdditionalSaveData(CompoundTag *tag);

protected:
	virtual void tick();

protected:
	virtual int getHurtSound();
	virtual int getDeathSound();

public:
	virtual void die(DamageSource *source);
	virtual bool doHurtTarget(shared_ptr<Entity> target);
	virtual bool isPowered();
	float getSwelling(float a);

protected:
	int getDeathLoot();

public:
	int getSwellDir();
	void setSwellDir(int dir);
	void thunderHit(const LightningBolt *lightningBolt) ;
};
