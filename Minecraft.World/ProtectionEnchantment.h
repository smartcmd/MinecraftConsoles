#pragma once

#include "Enchantment.h"

class ProtectionEnchantment : public Enchantment
{
public:
	static const int ALL = 0;
	static const int FIRE = 1;
	static const int FALL = 2;
	static const int EXPLOSION = 3;
	static const int PROJECTILE = 4;

private:
	static const int names[];
	static const int minCost[];
	static const int levelCost[];
	static const int levelCostSpan[];

public:
	const int type;

	ProtectionEnchantment(int id, int frequency, int type);

	virtual int getMinCost(int level);
	virtual int getMaxCost(int level);
	virtual int getMaxLevel();
	virtual int getDamageProtection(int level, DamageSource *source);
	virtual int getDescriptionId();
	virtual bool isCompatibleWith(Enchantment *other) const;
	static int getFireAfterDampener(shared_ptr<Entity> entity, int time);
	static double getExplosionKnockbackAfterDampener(shared_ptr<Entity> entity, double power);
};