#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "DiggingEnchantment.h"

DiggingEnchantment::DiggingEnchantment(int id, int frequency) : Enchantment(id, frequency, EnchantmentCategory::digger)
{
	setDescriptionId(IDS_ENCHANTMENT_DIGGING);
}

int DiggingEnchantment::getMinCost(int level)
{
	return 1 + 10 * (level - 1);
}

int DiggingEnchantment::getMaxCost(int level)
{
	return Enchantment::getMinCost(level) + 50;
}

int DiggingEnchantment::getMaxLevel()
{
	return 5;
}

bool DiggingEnchantment::canEnchant(shared_ptr<ItemInstance> item)
{
	if (item->getItem()->id == Item::shears_Id) return true;
	return Enchantment::canEnchant(item);
}