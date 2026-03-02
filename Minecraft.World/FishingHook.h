#pragma once
using namespace std;

#include "Entity.h"

class Player;

class FishingHook : public Entity
{
public:
	eINSTANCEOF GetType() { return eTYPE_FISHINGHOOK; }

private:
	int xTile;
	int yTile;
	int zTile;
	int lastTile;
	bool inGround;

public:
	int shakeTime;
	shared_ptr<Player> owner;

private:
	int life;
	int flightTime;
	int nibble;

public:
	shared_ptr<Entity> hookedIn;

private:
	void _init();

public:
	FishingHook(Level *level);
	FishingHook(Level *level, double x, double y, double z, shared_ptr<Player> owner);
	FishingHook(Level *level, shared_ptr<Player> mob);

protected:
	virtual void defineSynchedData();

public:
	bool shouldRenderAtSqrDistance(double distance);
	void shoot(double xd, double yd, double zd, float pow, float uncertainty);

private:
	int lSteps;
	double lx, ly, lz, lyr, lxr;
	double lxd, lyd, lzd;

public:
	virtual void lerpTo(double x, double y, double z, float yRot, float xRot, int steps);
	virtual void lerpMotion(double xd, double yd, double zd);
	virtual void tick();
	virtual void addAdditonalSaveData(CompoundTag *tag);
	virtual void readAdditionalSaveData(CompoundTag *tag);
	virtual float getShadowHeightOffs();
	int retrieve();

	// 4J Stu - Brought forward from 1.4
	virtual void remove();
};
