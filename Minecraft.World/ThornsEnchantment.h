#pragma once

#include "Enchantment.h"

class ThornsEnchantment : public Enchantment
{
private:
	static const float CHANCE_PER_LEVEl;

public:
	ThornsEnchantment(int id, int freq);

	virtual int getMinCost(int level);
	virtual int getMaxCost(int level);
	virtual int getMaxLevel();
	virtual bool canEnchant(shared_ptr<ItemInstance> item);
	static bool shouldHit(int level, Random *random);
	static int getDamage(int level, Random *random);
	static void doThornsAfterAttack(shared_ptr<Entity> source, shared_ptr<Mob> target, Random *random);
};