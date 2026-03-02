#pragma once

#include "Fireball.h"

class LargeFireball : public Fireball
{
public:
	eINSTANCEOF GetType() { return eTYPE_LARGE_FIREBALL; }
	static Entity *create(Level *level) { return new LargeFireball(level); }

public:
	int explosionPower;

	LargeFireball(Level *level);
	LargeFireball(Level *level, double x, double y, double z, double xa, double ya, double za);
	LargeFireball(Level *level, shared_ptr<LivingEntity> mob, double xa, double ya, double za);

protected:
	void onHit(HitResult *res);

public:
	void addAdditonalSaveData(CompoundTag *tag);
	void readAdditionalSaveData(CompoundTag *tag);
};