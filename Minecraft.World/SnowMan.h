#pragma once

#include "Golem.h"
#include "RangedAttackMob.h"

class SnowMan : public Golem, public RangedAttackMob
{
public:
	eINSTANCEOF GetType() { return eTYPE_SNOWMAN; }
	static Entity *create(Level *level) { return new SnowMan(level); }

public:
	SnowMan(Level *level);
	virtual bool useNewAi();

protected:
	virtual void registerAttributes();

public:
	virtual void aiStep();

protected:
	virtual int getDeathLoot();
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);

public:
	virtual void performRangedAttack(shared_ptr<LivingEntity> target, float power);
};