#include "stdafx.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.item.enchantment.h"
#include "net.minecraft.world.entity.h"
#include "ProtectionEnchantment.h"

const int ProtectionEnchantment::names[] = {IDS_ENCHANTMENT_PROTECT_ALL, IDS_ENCHANTMENT_PROTECT_FIRE, IDS_ENCHANTMENT_PROTECT_FALL, IDS_ENCHANTMENT_PROTECT_EXPLOSION, IDS_ENCHANTMENT_PROTECT_PROJECTILE};
const int ProtectionEnchantment::minCost[] = {1, 10, 5, 5, 3};
const int ProtectionEnchantment::levelCost[] = {11, 8, 6, 8, 6};
const int ProtectionEnchantment::levelCostSpan[] = {20, 12, 10, 12, 15};

ProtectionEnchantment::ProtectionEnchantment(int id, int frequency, int type) : Enchantment(id, frequency, EnchantmentCategory::armor), type(type)
{
	if (type == FALL)
	{
		category = EnchantmentCategory::armor_feet;
	}
}

int ProtectionEnchantment::getMinCost(int level)
{
	return minCost[type] + (level - 1) * levelCost[type];
}

int ProtectionEnchantment::getMaxCost(int level)
{
	return getMinCost(level) + levelCostSpan[type];
}

int ProtectionEnchantment::getMaxLevel()
{
	return 4;
}

int ProtectionEnchantment::getDamageProtection(int level, DamageSource *source)
{
	if (source->isBypassInvul()) return 0;

	float protect = (6 + level * level) / 3.0f;

	if (type == ALL) return Mth::floor(protect * 0.75f);
	if (type == FIRE && source->isFire()) return Mth::floor(protect * 1.25f);
	if (type == FALL && source == DamageSource::fall) return Mth::floor(protect * 2.5f);
	if (type == EXPLOSION && source == DamageSource::explosion) return Mth::floor(protect * 1.5f);
	if (type == PROJECTILE && source->isProjectile()) return Mth::floor(protect * 1.5f);
	return 0;
}

int ProtectionEnchantment::getDescriptionId()
{
	return names[type];
}

bool ProtectionEnchantment::isCompatibleWith(Enchantment *other) const
{		
	ProtectionEnchantment *pe = dynamic_cast<ProtectionEnchantment *>( other );
	if (pe != NULL)
	{
		if (pe->type == this->type)
		{
			return false;
		}
		if (this->type == FALL || pe->type == FALL)
		{
			return true;
		}
		return false;
	}
	return Enchantment::isCompatibleWith(other);
}

int ProtectionEnchantment::getFireAfterDampener(shared_ptr<Entity> entity, int time)
{
	int level = EnchantmentHelper::getEnchantmentLevel(Enchantment::fireProtection->id, entity->getEquipmentSlots());

	if (level > 0)
	{
		time -= Mth::floor(time * (level * 0.15f));
	}

	return time;
}

double ProtectionEnchantment::getExplosionKnockbackAfterDampener(shared_ptr<Entity> entity, double power)
{
	int level = EnchantmentHelper::getEnchantmentLevel(Enchantment::explosionProtection->id, entity->getEquipmentSlots());

	if (level > 0)
	{
		power -= Mth::floor(power * (level * 0.15f));
	}

	return power;
}