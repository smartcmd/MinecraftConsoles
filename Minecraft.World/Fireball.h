#pragma once
using namespace std;

#include "Entity.h"
#include "ParticleTypes.h"

class HitResult;

class Fireball : public Entity
{
public:
	eINSTANCEOF GetType() { return 	eTYPE_FIREBALL; }
	static Entity *create(Level *level) { return new Fireball(level); }

private:
	int xTile;
	int yTile;
	int zTile;
	int lastTile;

private:
	bool inGround;

public:
	shared_ptr<Mob> owner;

private:
	int life;
	int flightTime;

	// 4J - added common ctor code.
	void _init();

public:
	double xPower, yPower, zPower;

	Fireball(Level *level);

protected:
	virtual void defineSynchedData();

public:
	virtual bool shouldRenderAtSqrDistance(double distance);

	Fireball(Level *level, double x, double y, double z, double xa, double ya, double za);
	Fireball(Level *level, shared_ptr<Mob> mob, double xa, double ya, double za);

public:
	virtual void tick();

protected:
	virtual void onHit(HitResult *res);

public:
	virtual void addAdditonalSaveData(CompoundTag *tag);
	virtual void readAdditionalSaveData(CompoundTag *tag);
	virtual bool isPickable();
	virtual float getPickRadius();
	virtual bool hurt(DamageSource *source, int damage);
	virtual float getShadowHeightOffs();
	virtual float getBrightness(float a);
	virtual int getLightColor(float a);

	// 4J Added TU9
	virtual bool shouldBurn();
	virtual int getIcon();

protected:
// 4J Added TU9
	virtual ePARTICLE_TYPE getTrailParticleType();
};


