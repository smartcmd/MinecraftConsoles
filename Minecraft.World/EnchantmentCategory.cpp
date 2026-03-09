#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "EnchantmentCategory.h"

const EnchantmentCategory *EnchantmentCategory::all = new EnchantmentCategory();
const EnchantmentCategory *EnchantmentCategory::armor = new EnchantmentCategory();
const EnchantmentCategory *EnchantmentCategory::armor_feet = new EnchantmentCategory();
const EnchantmentCategory *EnchantmentCategory::armor_legs = new EnchantmentCategory();
const EnchantmentCategory *EnchantmentCategory::armor_torso = new EnchantmentCategory();
const EnchantmentCategory *EnchantmentCategory::armor_head = new EnchantmentCategory();
const EnchantmentCategory *EnchantmentCategory::weapon = new EnchantmentCategory();
const EnchantmentCategory *EnchantmentCategory::digger = new EnchantmentCategory();
const EnchantmentCategory *EnchantmentCategory::bow = new EnchantmentCategory();

bool EnchantmentCategory::canEnchant(Item *item) const
{
	if (this == all) return true;

	if (dynamic_cast<ArmorItem *>( item ) != nullptr)
	{
		if (this == armor) return true;
		ArmorItem *ai = static_cast<ArmorItem *>(item);
		if (ai->slot == ArmorItem::SLOT_HEAD) return this == armor_head;
		if (ai->slot == ArmorItem::SLOT_LEGS) return this == armor_legs;
		if (ai->slot == ArmorItem::SLOT_TORSO) return this == armor_torso;
		if (ai->slot == ArmorItem::SLOT_FEET) return this == armor_feet;
		return false;
	}
	else if (dynamic_cast<WeaponItem *>(item) != nullptr)
	{
		return this == weapon;
	}
	else if (dynamic_cast<DiggerItem *>(item) != nullptr)
	{
		return this == digger;
	}
	else if (dynamic_cast<BowItem *>(item) != nullptr)
	{
		return this == bow;
	}
	return false;
}