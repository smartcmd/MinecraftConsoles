#pragma once

#include "Entity.h"

class DamageSource;

class DelayedRelease : public Entity
{
public:
	virtual eINSTANCEOF GetType() { return eTYPE_DELAYEDRELEASE; }

private:
	shared_ptr<Entity> toRelease;
	int delay;

public:
	DelayedRelease(Level *level, shared_ptr<Entity> toRelease, int delay);

protected:
	virtual bool makeStepSound();

public:
	virtual void tick();
	virtual bool hurt(DamageSource *source, float damage);

protected:
	virtual void defineSynchedData();
	virtual void readAdditionalSaveData(CompoundTag *tag);
	virtual void addAdditonalSaveData(CompoundTag *tag);
};