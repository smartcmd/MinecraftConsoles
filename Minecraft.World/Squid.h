#pragma once

using namespace std;

#include "WaterAnimal.h"

class Player;

class Squid : public WaterAnimal
{
public:
	eINSTANCEOF GetType() { return eTYPE_SQUID; }
	static Entity *create(Level *level) { return new Squid(level); }

	void _init();

	float xBodyRot, xBodyRotO;
	float zBodyRot, zBodyRotO;

	float tentacleMovement, oldTentacleMovement;
	float tentacleAngle, oldTentacleAngle;

private:
	float speed;
	float tentacleSpeed;
	float rotateSpeed;

	float tx, ty, tz;

public:
	Squid(Level *level);

protected:
	virtual void registerAttributes();
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
	virtual float getSoundVolume();
	virtual int getDeathLoot();
	virtual bool makeStepSound();
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);

public:
	virtual bool isInWater();
	virtual void aiStep();
	virtual void travel(float xa, float ya);

protected:
	virtual void serverAiStep();

public:
	virtual bool canSpawn();
};
