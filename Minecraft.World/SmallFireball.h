#pragma once

#include "Fireball.h"

class HitResult;

class SmallFireball : public Fireball
{
public:
	eINSTANCEOF GetType() { return 	eTYPE_SMALL_FIREBALL; }
	static Entity *create(Level *level) { return new SmallFireball(level); }

public:
	SmallFireball(Level *level);
	SmallFireball(Level *level, shared_ptr<Mob> mob, double xa, double ya, double za);
	SmallFireball(Level *level, double x, double y, double z, double xa, double ya, double za);

protected:
	virtual void onHit(HitResult *res);

public:
	virtual bool isPickable();
	virtual bool hurt(DamageSource *source, int damage);
};