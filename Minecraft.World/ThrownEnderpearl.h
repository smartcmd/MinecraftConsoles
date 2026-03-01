#pragma once

#include "Throwable.h"

class HitResult;

class ThrownEnderpearl : public Throwable
{
public:
	eINSTANCEOF GetType() { return eTYPE_THROWNENDERPEARL; }
	static Entity *create(Level *level) { return new ThrownEnderpearl(level); }

	ThrownEnderpearl(Level *level);
	ThrownEnderpearl(Level *level, shared_ptr<LivingEntity> mob);
	ThrownEnderpearl(Level *level, double x, double y, double z);

protected:
	virtual void onHit(HitResult *res);
};