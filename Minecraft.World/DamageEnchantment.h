#pragma once

#include "Enchantment.h"

class DamageEnchantment : public Enchantment
{
public:
	static const int ALL = 0;
	static const int UNDEAD = 1;
	static const int ARTHROPODS = 2;

private:
	static const int names[];
	static const int minCost[];
	static const int levelCost[];
	static const int levelCostSpan[];

public:
	const int type;

	DamageEnchantment(int id, int frequency, int type);

	virtual int getMinCost(int level);
	virtual int getMaxCost(int level);
	virtual int getMaxLevel();
	virtual int getDamageBonus(int level, shared_ptr<Mob> target);
	virtual int getDescriptionId();
	virtual bool isCompatibleWith(Enchantment *other) const;
	virtual bool canEnchant(shared_ptr<ItemInstance> item);
};