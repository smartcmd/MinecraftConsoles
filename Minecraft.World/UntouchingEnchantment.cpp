#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "UntouchingEnchantment.h"

UntouchingEnchantment::UntouchingEnchantment(int id, int frequency) : Enchantment(id, frequency, EnchantmentCategory::digger)
{
	setDescriptionId(IDS_ENCHANTMENT_UNTOUCHING);
}

int UntouchingEnchantment::getMinCost(int level)
{
	return 15;
}

int UntouchingEnchantment::getMaxCost(int level)
{
	return Enchantment::getMinCost(level) + 50;
}

int UntouchingEnchantment::getMaxLevel()
{
	return 1;
}

bool UntouchingEnchantment::isCompatibleWith(Enchantment *other) const
{
	return Enchantment::isCompatibleWith(other) && other->id != resourceBonus->id;
}

bool UntouchingEnchantment::canEnchant(shared_ptr<ItemInstance> item)
{
	if (item->getItem()->id == Item::shears_Id) return true;
	return Enchantment::canEnchant(item);
}