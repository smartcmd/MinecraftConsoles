#include "stdafx.h"
#include "net.minecraft.world.effect.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "SharedConstants.h"
#include "GoldenAppleItem.h"

GoldenAppleItem::GoldenAppleItem(int id, int nutrition, float saturationMod, bool isMeat) : FoodItem(id, nutrition, saturationMod, isMeat)
{
	setStackedByData(true);
}

bool GoldenAppleItem::isFoil(shared_ptr<ItemInstance> itemInstance)
{
	return itemInstance->getAuxValue() > 0;
}

const Rarity *GoldenAppleItem::getRarity(shared_ptr<ItemInstance> itemInstance)
{
	if (itemInstance->getAuxValue() == 0)
	{
		return Rarity::rare;
	}
	return Rarity::epic;
}

void GoldenAppleItem::addEatEffect(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player)
{
	if (instance->getAuxValue() > 0)
	{
		if (!level->isClientSide)
		{
			player->addEffect(new MobEffectInstance(MobEffect::regeneration->id, 30 * SharedConstants::TICKS_PER_SECOND, 3));
			player->addEffect(new MobEffectInstance(MobEffect::damageResistance->id, 300 * SharedConstants::TICKS_PER_SECOND, 0));
			player->addEffect(new MobEffectInstance(MobEffect::fireResistance->id, 300 * SharedConstants::TICKS_PER_SECOND, 0));
		}
	}
	else
	{
		FoodItem::addEatEffect(instance, level, player);
	}
}

unsigned int GoldenAppleItem::getUseDescriptionId(int iData /*= -1*/)
{
	if (iData == 0) return IDS_DESC_GOLDENAPPLE;
	else			return IDS_DESC_ENCHANTED_GOLDENAPPLE;
}

unsigned int GoldenAppleItem::getUseDescriptionId(shared_ptr<ItemInstance> instance)
{
	return this->getUseDescriptionId(instance->getAuxValue());
}