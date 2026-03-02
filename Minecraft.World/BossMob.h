#pragma once

#include "Mob.h"

class Level;
class BossMobPart;

class BossMob : public Mob
{
protected:
	int maxHealth;

public:
	BossMob(Level *level);

	virtual int getMaxHealth();
	virtual bool hurt(shared_ptr<BossMobPart> bossMobPart, DamageSource *source, int damage);
	virtual bool hurt(DamageSource *source, int damage);

protected:
	virtual bool reallyHurt(DamageSource *source, int damage);
};