#pragma once
using namespace std;

#include "Entity.h"
#include "ParticleTypes.h"

class HitResult;

class Fireball : public Entity
{
public:
	eINSTANCEOF GetType() { return 	eTYPE_FIREBALL; }

private:
	int xTile;
	int yTile;
	int zTile;
	int lastTile;

private:
	bool inGround;

public:
	shared_ptr<LivingEntity> owner;

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
	Fireball(Level *level, shared_ptr<LivingEntity> mob, double xa, double ya, double za);

public:
	virtual void tick();

protected:
	virtual float getInertia();
	virtual void onHit(HitResult *res) = 0;

public:
	virtual void addAdditonalSaveData(CompoundTag *tag);
	virtual void readAdditionalSaveData(CompoundTag *tag);
	virtual bool isPickable();
	virtual float getPickRadius();
	virtual bool hurt(DamageSource *source, float damage);
	virtual float getShadowHeightOffs();
	virtual float getBrightness(float a);
	virtual int getLightColor(float a);

protected:
// 4J Added TU9
	virtual ePARTICLE_TYPE getTrailParticleType();

	virtual bool shouldBurn();
};


