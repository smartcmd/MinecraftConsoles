#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.food.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.effect.h"
#include "net.minecraft.h"
#include "ItemInstance.h"
#include "FoodItem.h"
#include "SoundTypes.h"

// 4J : WESTY : Other award ... eating cooked pork chop.
#include "net.minecraft.stats.h"

void FoodItem::_init()
{
	// 4J Initialisers
	canAlwaysEat = false;
	effectId = 0;
	effectDurationSeconds = 0;
	effectAmplifier = 0;
	effectProbability = 0.0f;
}

FoodItem::FoodItem(int id, int nutrition, float saturationMod, bool isMeat)
	: Item( id ), nutrition(nutrition), saturationModifier( saturationMod ), m_isMeat( isMeat )
{
	_init();
}

FoodItem::FoodItem(int id, int nutrition, bool isMeat)
	: Item( id ), nutrition(nutrition), saturationModifier( FoodConstants::FOOD_SATURATION_NORMAL ), m_isMeat( isMeat )
{
	_init();
}

shared_ptr<ItemInstance> FoodItem::useTimeDepleted(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player)
{
	instance->count--;
	player->getFoodData()->eat(this);
	// 4J - new sound brought forward from 1.2.3
	level->playEntitySound(player, eSoundType_RANDOM_BURP, 0.5f, level->random->nextFloat() * 0.1f + 0.9f);

	addEatEffect(instance, level, player);

	return instance;
}

void FoodItem::addEatEffect(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player)
{
	if (!level->isClientSide && effectId > 0 && level->random->nextFloat() < effectProbability)
	{
		player->addEffect(new MobEffectInstance(effectId, effectDurationSeconds * SharedConstants::TICKS_PER_SECOND, effectAmplifier));
	}

}

int FoodItem::getUseDuration(shared_ptr<ItemInstance> itemInstance)
{
	return EAT_DURATION;
}

UseAnim FoodItem::getUseAnimation(shared_ptr<ItemInstance> itemInstance)
{
	return UseAnim_eat;
}

shared_ptr<ItemInstance> FoodItem::use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player)
{
	if (player->canEat(canAlwaysEat))
	{
		player->startUsingItem(instance, getUseDuration(instance));
	}

	// 4J : WESTY : Other award ... eating cooked pork chop.
	// 4J-JEV: This is just for an avatar award on the xbox.
#ifdef _XBOX 
	if ( instance->getItem() == Item::porkChop_cooked )
	{
		player->awardStat(GenericStats::eatPorkChop(),GenericStats::param_eatPorkChop());
	}
#endif

	return instance;
}

float FoodItem::getSaturationModifier()
{
	return saturationModifier;
}

int FoodItem::getNutrition()
{
	return nutrition;
}

bool FoodItem::isMeat()
{
	return m_isMeat;
}

FoodItem *FoodItem::setEatEffect(int id, int durationInSecods, int amplifier, float effectProbability)
{
	effectId = id;
	effectDurationSeconds = durationInSecods;
	effectAmplifier = amplifier;
	this->effectProbability = effectProbability;
	return this;
}

FoodItem *FoodItem::setCanAlwaysEat()
{
	canAlwaysEat = true;
	return this;
}

// 4J Added
bool FoodItem::canEat(shared_ptr<Player> player)
{
	return player->canEat(canAlwaysEat);
}
