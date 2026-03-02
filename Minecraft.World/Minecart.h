#pragma once
#include "Entity.h"
#include "Container.h"

class DamageSource;

class Minecart : public Entity, public Container
{
public:
	eINSTANCEOF GetType() { return eTYPE_MINECART; };
	static Entity *create(Level *level) { return new Minecart(level); }

public:
	static const int RIDEABLE = 0;
	static const int CHEST = 1;
	static const int FURNACE = 2;

private:
	ItemInstanceArray *items; // Array

public:
	static const int serialVersionUID = 0;

private:
	static const int DATA_ID_FUEL = 16;
	static const int DATA_ID_HURT = 17;
	static const int DATA_ID_HURTDIR = 18;
	static const int DATA_ID_DAMAGE = 19;

	int fuel;

private:
	bool flipped;

protected:
	// 4J Added
	bool m_bHasPushedCartThisTick;

public:
	int type;
	double xPush, zPush;

	void _init();

	Minecart(Level *level);

protected:
	virtual bool makeStepSound();
	virtual void defineSynchedData();

public:
	virtual AABB *getCollideAgainstBox(shared_ptr<Entity> entity);
	virtual AABB *getCollideBox();
	virtual bool isPushable();

	Minecart(Level *level, double x, double y, double z, int type);

	virtual double getRideHeight();
	virtual bool hurt(DamageSource *source, int damage);
	virtual void animateHurt();
	virtual bool isPickable();
	virtual void remove();

private:
	static const int EXITS[][2][3];

public:
	virtual void tick();
	virtual Vec3 *getPosOffs(double x, double y, double z, double offs);
	virtual Vec3 *getPos(double x, double y, double z);

protected:
	virtual void addAdditonalSaveData(CompoundTag *base);
	virtual void readAdditionalSaveData(CompoundTag *base);

public:
	virtual float getShadowHeightOffs();
	virtual void push(shared_ptr<Entity> e);
	virtual unsigned int getContainerSize();
	virtual shared_ptr<ItemInstance> getItem(unsigned int slot);
	virtual shared_ptr<ItemInstance> removeItem(unsigned int slot, int count);
	virtual shared_ptr<ItemInstance> removeItemNoUpdate(int slot);
	virtual void setItem(unsigned int slot, shared_ptr<ItemInstance> item);
	int getName();
	virtual int getMaxStackSize();
	virtual void setChanged();
	virtual bool interact(shared_ptr<Player> player);
	virtual float getLootContent();

private:
	int lSteps;
	double lx, ly, lz, lyr, lxr;
	double lxd, lyd, lzd;

public:
	virtual void lerpTo(double x, double y, double z, float yRot, float xRot, int steps);
	virtual void lerpMotion(double xd, double yd, double zd);
	virtual bool stillValid(shared_ptr<Player> player);

protected:
	bool hasFuel();
	void setHasFuel(bool fuel);

public:
	virtual void startOpen();
	virtual void stopOpen();

	void setDamage(int damage);
	int getDamage();
	void setHurtTime(int hurtTime);
	int getHurtTime();
	void setHurtDir(int hurtDir);
	int getHurtDir();
};
