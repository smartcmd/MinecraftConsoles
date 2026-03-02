#pragma once

#include "Fireball.h"

class HitResult;

class DragonFireball : public Fireball
{
public:
	eINSTANCEOF GetType() { return 	eTYPE_DRAGON_FIREBALL; }
	static Entity *create(Level *level) { return new DragonFireball(level); }

public:
	static const double SPLASH_RANGE;

private:
	static const double SPLASH_RANGE_SQ;

public:
	DragonFireball(Level *level);
	DragonFireball(Level *level, shared_ptr<LivingEntity> mob, double xa, double ya, double za);
	DragonFireball(Level *level, double x, double y, double z, double xa, double ya, double za);

protected:
	virtual void onHit(HitResult *res);

public:
	virtual bool isPickable();
	virtual bool hurt(DamageSource *source, float damage);

protected:
// 4J Added TU9
	virtual ePARTICLE_TYPE getTrailParticleType();
	
	virtual bool shouldBurn();
};