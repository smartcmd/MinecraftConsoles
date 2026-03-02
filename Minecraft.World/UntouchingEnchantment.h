#pragma once

#include "Enchantment.h"

class UntouchingEnchantment : public Enchantment
{
public:
	UntouchingEnchantment(int id, int frequency);

	virtual int getMinCost(int level);
	virtual int getMaxCost(int level);
	virtual int getMaxLevel();
	virtual bool isCompatibleWith(Enchantment *other) const;
	virtual bool canEnchant(shared_ptr<ItemInstance> item);
};