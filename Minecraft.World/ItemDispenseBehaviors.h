#pragma once
#include "DefaultDispenseItemBehavior.h"
#include "AbstractProjectileDispenseBehavior.h"

class ArrowDispenseBehavior : public AbstractProjectileDispenseBehavior
{
protected:
	virtual shared_ptr<Projectile> getProjectile(Level *world, Position *position);
};

class EggDispenseBehavior : public AbstractProjectileDispenseBehavior
{
protected:
	virtual shared_ptr<Projectile> getProjectile(Level *world, Position *position);
};

class SnowballDispenseBehavior : public AbstractProjectileDispenseBehavior
{
protected:
	virtual shared_ptr<Projectile> getProjectile(Level *world, Position *position);
};

class ExpBottleDispenseBehavior : public AbstractProjectileDispenseBehavior
{
protected:
	virtual shared_ptr<Projectile> getProjectile(Level *world, Position *position);
	virtual float getUncertainty();
	virtual float getPower();
};

class ThrownPotionDispenseBehavior : public AbstractProjectileDispenseBehavior
{
private:
	int m_potionValue;
public:
	ThrownPotionDispenseBehavior(int potionValue);
protected:
	virtual shared_ptr<Projectile> getProjectile(Level *world, Position *position);
	virtual float getUncertainty();
	virtual float getPower();
};

class PotionDispenseBehavior : public DefaultDispenseItemBehavior
{
public:
	virtual	shared_ptr<ItemInstance> dispense(BlockSource *source, shared_ptr<ItemInstance> dispensed);
};

class SpawnEggDispenseBehavior : public DefaultDispenseItemBehavior
{
public:
	virtual shared_ptr<ItemInstance> execute(BlockSource *source, shared_ptr<ItemInstance> dispensed, eOUTCOME &outcome);
};

class FireworksDispenseBehavior : public DefaultDispenseItemBehavior
{
public:
	virtual shared_ptr<ItemInstance> execute(BlockSource *source, shared_ptr<ItemInstance> dispensed, eOUTCOME &outcome);
protected:
	virtual void playSound(BlockSource *source, eOUTCOME outcome);
};

class FireballDispenseBehavior : public DefaultDispenseItemBehavior
{
public:
	virtual shared_ptr<ItemInstance> execute(BlockSource *source, shared_ptr<ItemInstance> dispensed, eOUTCOME &outcome);
protected:
	virtual void playSound(BlockSource *source, eOUTCOME outcome);
};

class BoatDispenseBehavior : public DefaultDispenseItemBehavior
{
public:
	BoatDispenseBehavior();
	virtual ~BoatDispenseBehavior();
	virtual shared_ptr<ItemInstance> execute(BlockSource *source, shared_ptr<ItemInstance> dispensed, eOUTCOME &outcome);
protected:
	virtual void playSound(BlockSource *source, eOUTCOME outcome);
private:
	DefaultDispenseItemBehavior *defaultDispenseItemBehavior;
};

class FilledBucketDispenseBehavior : public DefaultDispenseItemBehavior
{
public:
	virtual shared_ptr<ItemInstance> execute(BlockSource *source, shared_ptr<ItemInstance> dispensed, eOUTCOME &outcome);
};

class EmptyBucketDispenseBehavior : public DefaultDispenseItemBehavior
{
public:
	virtual shared_ptr<ItemInstance> execute(BlockSource *source, shared_ptr<ItemInstance> dispensed, eOUTCOME &outcome);
};

class FlintAndSteelDispenseBehavior : public DefaultDispenseItemBehavior
{
	// bool success; // 4J-JEV: Removed because we have something cleaner for this now.
public:
	shared_ptr<ItemInstance> execute(BlockSource *source, shared_ptr<ItemInstance> dispensed, eOUTCOME &outcome);
protected:
	virtual void playSound(BlockSource *source, eOUTCOME outcome);
};

class DyeDispenseBehavior : public DefaultDispenseItemBehavior
{
	// bool success; // 4J-JEV: Removed because we have something cleaner for this now.
public:
	virtual shared_ptr<ItemInstance> execute(BlockSource *source, shared_ptr<ItemInstance> dispensed, eOUTCOME &outcome);
protected:
	virtual void playSound(BlockSource *source, eOUTCOME outcome);
};

class TntDispenseBehavior : public DefaultDispenseItemBehavior
{
protected:
	virtual shared_ptr<ItemInstance> execute(BlockSource *source, shared_ptr<ItemInstance> dispensed, eOUTCOME &outcome);
};