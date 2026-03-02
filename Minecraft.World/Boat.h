#pragma once
using namespace std;

#include "Entity.h"

class Player;
class Level;
class CompoundTag;
class DamageSource;

class Boat : public Entity
{
public:
	eINSTANCEOF GetType() { return eTYPE_BOAT; };
	static Entity *create(Level *level) { return new Boat(level); }

private:
	// 4J - added for common ctor code
	void _init();
public:
	static const int serialVersionUID = 0;

private:
	static const int DATA_ID_HURT = 17;
	static const int DATA_ID_HURTDIR = 18;
	static const int DATA_ID_DAMAGE = 19;
	static const double MAX_SPEED;
	static const double MAX_COLLISION_SPEED;
	static const double MIN_ACCELERATION;
	static const double MAX_ACCELERATION;

	bool doLerp;
	double acceleration;

public:
	Boat(Level *level);

protected:
	virtual bool makeStepSound();
	virtual void defineSynchedData();

public:
	virtual AABB *getCollideAgainstBox(shared_ptr<Entity> entity);
	virtual AABB *getCollideBox();
	virtual bool isPushable();

	Boat(Level *level, double x, double y, double z);

	virtual double getRideHeight();
	virtual bool hurt(DamageSource *source, int damage);
	virtual void animateHurt();
	virtual bool isPickable();

private:
	int lSteps;
	double lx, ly, lz, lyr, lxr;
	double lxd, lyd, lzd;

public:
	virtual void lerpTo(double x, double y, double z, float yRot, float xRot, int steps);
	virtual void lerpMotion(double xd, double yd, double zd);
	virtual void tick();
	virtual void positionRider();

protected:
	virtual void addAdditonalSaveData(CompoundTag *base);
	virtual void readAdditionalSaveData(CompoundTag *base);

public:
	virtual float getShadowHeightOffs();
	wstring getName();
	virtual bool interact(shared_ptr<Player> player);

	virtual void setDamage(int damage);
	virtual int getDamage();
	virtual void setHurtTime(int hurtTime);
	virtual int getHurtTime();
	virtual void setHurtDir(int hurtDir);
	virtual int getHurtDir();

	bool getDoLerp();
	void setDoLerp(bool doLerp);
};
