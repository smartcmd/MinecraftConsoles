#pragma once

#include "Enchantment.h"

class DiggingEnchantment : public Enchantment
{
public:
	DiggingEnchantment(int id, int frequency);

	virtual int getMinCost(int level);
	virtual int getMaxCost(int level);
	virtual int getMaxLevel();
	virtual bool canEnchant(shared_ptr<ItemInstance> item);
};