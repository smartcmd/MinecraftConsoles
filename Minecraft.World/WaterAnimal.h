#pragma once
#include "PathfinderMob.h"
#include "Creature.h"

class Player;

class WaterAnimal : public PathfinderMob, public Creature
{
public:
	WaterAnimal(Level *level);
	virtual bool isWaterMob();
	virtual bool canSpawn();
	virtual int getAmbientSoundInterval();

protected:
	virtual bool removeWhenFarAway();
	virtual int getExperienceReward(shared_ptr<Player> killedBy);
};
