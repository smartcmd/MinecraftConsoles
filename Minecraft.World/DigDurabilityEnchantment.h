#pragma once

#include "Enchantment.h"

class DigDurabilityEnchantment : public Enchantment
{
public:
	DigDurabilityEnchantment(int id, int frequency);

	virtual int getMinCost(int level);
	virtual int getMaxCost(int level);
	virtual int getMaxLevel();
	virtual bool canEnchant(shared_ptr<ItemInstance> item);
	static bool shouldIgnoreDurabilityDrop(shared_ptr<ItemInstance> item, int level, Random *random);
};