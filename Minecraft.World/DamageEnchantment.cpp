#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.item.h"
#include "DamageEnchantment.h"

const int DamageEnchantment::names[] =	{IDS_ENCHANTMENT_DAMAGE_ALL, IDS_ENCHANTMENT_DAMAGE_UNDEAD, IDS_ENCHANTMENT_DAMAGE_ARTHROPODS};
const int DamageEnchantment::minCost[] = {1, 5, 5};
const int DamageEnchantment::levelCost[] = {11, 8, 8};
const int DamageEnchantment::levelCostSpan[] = {20, 20, 20};

DamageEnchantment::DamageEnchantment(int id, int frequency, int type) : Enchantment(id, frequency, EnchantmentCategory::weapon), type(type)
{
}

int DamageEnchantment::getMinCost(int level)
{
	return minCost[type] + (level - 1) * levelCost[type];
}

int DamageEnchantment::getMaxCost(int level)
{
	return getMinCost(level) + levelCostSpan[type];
}

int DamageEnchantment::getMaxLevel()
{
	return 5;
}

int DamageEnchantment::getDamageBonus(int level, shared_ptr<Mob> target)
{
	if (type == ALL)
	{
		return Mth::floor(level * 2.75f);
	}
	if (type == UNDEAD && target->getMobType() == UNDEAD)
	{
		return Mth::floor(level * 4.5f);
	}
	if (type == ARTHROPODS && target->getMobType() == ARTHROPOD)
	{
		return Mth::floor(level * 4.5f);
	}
	return 0;
}

int DamageEnchantment::getDescriptionId()
{
	return names[type];
}

bool DamageEnchantment::isCompatibleWith(Enchantment *other) const
{
	return dynamic_cast<DamageEnchantment *>(other) == NULL;
}

bool DamageEnchantment::canEnchant(shared_ptr<ItemInstance> item)
{
	HatchetItem *hatchet = dynamic_cast<HatchetItem *>(item->getItem());
	if (hatchet) return true;
	return Enchantment::canEnchant(item);
}