#pragma once

#include "../Minecraft.World/DefaultDispenseItemBehavior.h"

class Projectile;

class AbstractProjectileDispenseBehavior : public DefaultDispenseItemBehavior
{
public:
	shared_ptr<ItemInstance> execute(BlockSource *source, shared_ptr<ItemInstance> dispensed);

protected:
	virtual void playSound(BlockSource *source);
	virtual shared_ptr<Projectile> getProjectile(Level *world, Position *position) = 0;
	virtual float getUncertainty();
	virtual float getPower();
};