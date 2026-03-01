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

protected:
	virtual void registerAttributes();
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
	virtual float getSoundVolume();
	virtual int getDeathLoot();
	virtual void playStepSound(int xt, int yt, int zt, int t);
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);

public:
	virtual bool mobInteract(shared_ptr<Player> player);
	virtual shared_ptr<AgableMob> getBreedOffspring(shared_ptr<AgableMob> target);
};
