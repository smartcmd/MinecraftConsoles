#pragma once
using namespace std;

#include "Animal.h"

class Level;
class CompoundTag;

class Chicken : public Animal
{
public:
	eINSTANCEOF GetType() { return eTYPE_CHICKEN; }
	static Entity *create(Level *level) { return new Chicken(level); }
    bool sheared;
    float flap;
    float flapSpeed;
    float oFlapSpeed, oFlap;
    float flapping;
    int eggTime;

private:	
	void _init();

public:
	Chicken(Level *level);
	virtual bool useNewAi();

protected:
	void registerAttributes();

public:
	virtual void aiStep();	

protected:
	virtual void causeFallDamage(float distance);
	virtual int getAmbientSound();
    virtual int getHurtSound();
    virtual int getDeathSound();
	virtual void playStepSound(int xt, int yt, int zt, int t);
    virtual int getDeathLoot();
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);

public:
	virtual shared_ptr<AgableMob> getBreedOffspring(shared_ptr<AgableMob> target);
	virtual bool isFood(shared_ptr<ItemInstance> itemInstance);

};
