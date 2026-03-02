#pragma once
#include "DefaultDispenseItemBehavior.h"

class Projectile;
class Position;

class AbstractProjectileDispenseBehavior : public DefaultDispenseItemBehavior
{
public:
	virtual shared_ptr<ItemInstance> execute(BlockSource *source, shared_ptr<ItemInstance> dispensed, eOUTCOME &outcome);

protected:
	virtual void playSound(BlockSource *source, eOUTCOME outcome);
	virtual float getUncertainty();
	virtual float getPower();
	virtual shared_ptr<Projectile> getProjectile(Level *world, Position *position) = 0;
};