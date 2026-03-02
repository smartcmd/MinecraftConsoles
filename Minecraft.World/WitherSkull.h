#pragma once

#include "Fireball.h"

class WitherSkull : public Fireball
{
public:
	eINSTANCEOF GetType() { return 	eTYPE_WITHER_SKULL; }
	static Entity *create(Level *level) { return new WitherSkull(level); }

private:
	static const int DATA_DANGEROUS = 10;

public:
	WitherSkull(Level *level);
	WitherSkull(Level *level, shared_ptr<LivingEntity> mob, double xa, double ya, double za);

protected:
	virtual float getInertia();

public:
	WitherSkull(Level *level, double x, double y, double z, double xa, double ya, double za);

	virtual bool isOnFire();
	virtual float getTileExplosionResistance(Explosion *explosion, Level *level, int x, int y, int z, Tile *tile);

protected:
	virtual void onHit(HitResult *res);

public:
	virtual bool isPickable();
	virtual bool hurt(DamageSource *source, float damage);

protected:
	virtual void defineSynchedData();

public:
	virtual bool isDangerous();
	virtual void setDangerous(bool value);

protected:
	virtual bool shouldBurn(); // 4J Added.
};