#pragma once

using namespace std;

#include "Animal.h"

class Player;
class Level;

class Cow : public Animal
{
public:
	eINSTANCEOF GetType() { return eTYPE_COW; }
	static Entity *create(Level *level) { return new Cow(level); }

public:
	Cow(Level *level);
	virtual bool useNewAi();
	virtual int getMaxHealth();

protected:
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
	virtual float getSoundVolume();
	virtual int getDeathLoot();
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);

public:
	virtual bool interact(shared_ptr<Player> player);
	virtual shared_ptr<AgableMob> getBreedOffspring(shared_ptr<AgableMob> target);
};
