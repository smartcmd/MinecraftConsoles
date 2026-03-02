#pragma once
using namespace std;

#include "Throwable.h"

class HitResult;

class ThrownEgg : public Throwable
{
public:
	eINSTANCEOF GetType() { return eTYPE_THROWNEGG; }

private:
	void _init();

public:
	ThrownEgg(Level *level);
	ThrownEgg(Level *level, shared_ptr<LivingEntity> mob);
	ThrownEgg(Level *level, double x, double y, double z);

protected:
	virtual void onHit(HitResult *res);
};
