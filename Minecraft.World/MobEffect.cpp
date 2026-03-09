#include "stdafx.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.food.h"
#include "net.minecraft.world.effect.h"
#include "SharedConstants.h"

MobEffect *MobEffect::effects[NUM_EFFECTS];

MobEffect *MobEffect::voidEffect;
MobEffect *MobEffect::movementSpeed;
MobEffect *MobEffect::movementSlowdown;
MobEffect *MobEffect::digSpeed;
MobEffect *MobEffect::digSlowdown;
MobEffect *MobEffect::damageBoost;
MobEffect *MobEffect::heal;
MobEffect *MobEffect::harm;
MobEffect *MobEffect::jump;
MobEffect *MobEffect::confusion;
MobEffect *MobEffect::regeneration;
MobEffect *MobEffect::damageResistance;
MobEffect *MobEffect::fireResistance;
MobEffect *MobEffect::waterBreathing;
MobEffect *MobEffect::invisibility;
MobEffect *MobEffect::blindness;
MobEffect *MobEffect::nightVision;
MobEffect *MobEffect::hunger;
MobEffect *MobEffect::weakness;
MobEffect *MobEffect::poison;
MobEffect *MobEffect::wither;
MobEffect *MobEffect::healthBoost;
MobEffect *MobEffect::absorption;
MobEffect *MobEffect::saturation;
MobEffect *MobEffect::reserved_24;
MobEffect *MobEffect::reserved_25;
MobEffect *MobEffect::reserved_26;
MobEffect *MobEffect::reserved_27;
MobEffect *MobEffect::reserved_28;
MobEffect *MobEffect::reserved_29;
MobEffect *MobEffect::reserved_30;
MobEffect *MobEffect::reserved_31;

void MobEffect::staticCtor()
{
	voidEffect = nullptr;
	movementSpeed =		(new MobEffect(1, false, eMinecraftColour_Effect_MovementSpeed))			->setDescriptionId(IDS_POTION_MOVESPEED)		->setPostfixDescriptionId(IDS_POTION_MOVESPEED_POSTFIX)->setIcon(MobEffect::e_MobEffectIcon_Speed)->addAttributeModifier(SharedMonsterAttributes::MOVEMENT_SPEED, eModifierId_POTION_MOVESPEED, 0.2f, AttributeModifier::OPERATION_MULTIPLY_TOTAL); //setIcon(0, 0);
	movementSlowdown =	(new MobEffect(2, true, eMinecraftColour_Effect_MovementSlowDown))			->setDescriptionId(IDS_POTION_MOVESLOWDOWN)		->setPostfixDescriptionId(IDS_POTION_MOVESLOWDOWN_POSTFIX)->setIcon(MobEffect::e_MobEffectIcon_Slowness)->addAttributeModifier(SharedMonsterAttributes::MOVEMENT_SPEED, eModifierId_POTION_MOVESLOWDOWN, -0.15f, AttributeModifier::OPERATION_MULTIPLY_TOTAL); //->setIcon(1, 0);
	digSpeed =			(new MobEffect(3, false, eMinecraftColour_Effect_DigSpeed))					->setDescriptionId(IDS_POTION_DIGSPEED)			->setPostfixDescriptionId(IDS_POTION_DIGSPEED_POSTFIX)->setDurationModifier(1.5)->setIcon(MobEffect::e_MobEffectIcon_Haste); //->setIcon(2, 0);
	digSlowdown =		(new MobEffect(4, true, eMinecraftColour_Effect_DigSlowdown))				->setDescriptionId(IDS_POTION_DIGSLOWDOWN)		->setPostfixDescriptionId(IDS_POTION_DIGSLOWDOWN_POSTFIX)->setIcon(MobEffect::e_MobEffectIcon_MiningFatigue); //->setIcon(3, 0);
	damageBoost =		(new AttackDamageMobEffect(5, false, eMinecraftColour_Effect_DamageBoost))	->setDescriptionId(IDS_POTION_DAMAGEBOOST)		->setPostfixDescriptionId(IDS_POTION_DAMAGEBOOST_POSTFIX)->setIcon(MobEffect::e_MobEffectIcon_Strength)->addAttributeModifier(SharedMonsterAttributes::ATTACK_DAMAGE, eModifierId_POTION_DAMAGEBOOST, 3, AttributeModifier::OPERATION_MULTIPLY_TOTAL); //->setIcon(4, 0);
	heal =				(new InstantenousMobEffect(6, false, eMinecraftColour_Effect_Heal))			->setDescriptionId(IDS_POTION_HEAL)				->setPostfixDescriptionId(IDS_POTION_HEAL_POSTFIX);
	harm =				(new InstantenousMobEffect(7, true, eMinecraftColour_Effect_Harm))			->setDescriptionId(IDS_POTION_HARM)				->setPostfixDescriptionId(IDS_POTION_HARM_POSTFIX);
	jump =				(new MobEffect(8, false, eMinecraftColour_Effect_Jump))						->setDescriptionId(IDS_POTION_JUMP)				->setPostfixDescriptionId(IDS_POTION_JUMP_POSTFIX)->setIcon(MobEffect::e_MobEffectIcon_JumpBoost); //->setIcon(2, 1);
	confusion =			(new MobEffect(9, true, eMinecraftColour_Effect_Confusion))					->setDescriptionId(IDS_POTION_CONFUSION)		->setPostfixDescriptionId(IDS_POTION_CONFUSION_POSTFIX)->setDurationModifier(.25)->setIcon(MobEffect::e_MobEffectIcon_Nausea); //->setIcon(3, 1);
	regeneration =		(new MobEffect(10, false, eMinecraftColour_Effect_Regeneration))			->setDescriptionId(IDS_POTION_REGENERATION)		->setPostfixDescriptionId(IDS_POTION_REGENERATION_POSTFIX)->setDurationModifier(.25)->setIcon(MobEffect::e_MobEffectIcon_Regeneration); //->setIcon(7, 0);
	damageResistance =	(new MobEffect(11, false, eMinecraftColour_Effect_DamageResistance))		->setDescriptionId(IDS_POTION_RESISTANCE)		->setPostfixDescriptionId(IDS_POTION_RESISTANCE_POSTFIX)->setIcon(MobEffect::e_MobEffectIcon_Resistance); //->setIcon(6, 1);
	fireResistance =	(new MobEffect(12, false, eMinecraftColour_Effect_FireResistance))			->setDescriptionId(IDS_POTION_FIRERESISTANCE)	->setPostfixDescriptionId(IDS_POTION_FIRERESISTANCE_POSTFIX)->setIcon(MobEffect::e_MobEffectIcon_FireResistance); //->setIcon(7, 1);
	waterBreathing =	(new MobEffect(13, false, eMinecraftColour_Effect_WaterBreathing))			->setDescriptionId(IDS_POTION_WATERBREATHING)	->setPostfixDescriptionId(IDS_POTION_WATERBREATHING_POSTFIX)->setIcon(MobEffect::e_MobEffectIcon_WaterBreathing); //->setIcon(0, 2);
	invisibility =		(new MobEffect(14, false, eMinecraftColour_Effect_Invisiblity))				->setDescriptionId(IDS_POTION_INVISIBILITY)		->setPostfixDescriptionId(IDS_POTION_INVISIBILITY_POSTFIX)->setIcon(MobEffect::e_MobEffectIcon_Invisiblity); //->setIcon(0, 1);
	blindness =			(new MobEffect(15, true, eMinecraftColour_Effect_Blindness))				->setDescriptionId(IDS_POTION_BLINDNESS)		->setPostfixDescriptionId(IDS_POTION_BLINDNESS_POSTFIX)->setDurationModifier(.25)->setIcon(MobEffect::e_MobEffectIcon_Blindness); //->setIcon(5, 1);
	nightVision =		(new MobEffect(16, false, eMinecraftColour_Effect_NightVision))				->setDescriptionId(IDS_POTION_NIGHTVISION)		->setPostfixDescriptionId(IDS_POTION_NIGHTVISION_POSTFIX)->setIcon(MobEffect::e_MobEffectIcon_NightVision); //->setIcon(4, 1);
	hunger =			(new MobEffect(17, true, eMinecraftColour_Effect_Hunger))					->setDescriptionId(IDS_POTION_HUNGER)			->setPostfixDescriptionId(IDS_POTION_HUNGER_POSTFIX)->setIcon(MobEffect::e_MobEffectIcon_Hunger); //->setIcon(1, 1);
	weakness =			(new AttackDamageMobEffect(18, true, eMinecraftColour_Effect_Weakness))		->setDescriptionId(IDS_POTION_WEAKNESS)			->setPostfixDescriptionId(IDS_POTION_WEAKNESS_POSTFIX)->setIcon(MobEffect::e_MobEffectIcon_Weakness)->addAttributeModifier(SharedMonsterAttributes::ATTACK_DAMAGE, eModifierId_POTION_WEAKNESS, 2, AttributeModifier::OPERATION_ADDITION); //->setIcon(5, 0);
	poison =			(new MobEffect(19, true, eMinecraftColour_Effect_Poison))					->setDescriptionId(IDS_POTION_POISON)			->setPostfixDescriptionId(IDS_POTION_POISON_POSTFIX)->setDurationModifier(.25)->setIcon(MobEffect::e_MobEffectIcon_Poison); //->setIcon(6, 0);
	wither =			(new MobEffect(20, true, eMinecraftColour_Effect_Wither))					->setDescriptionId(IDS_POTION_WITHER)			->setPostfixDescriptionId(IDS_POTION_WITHER_POSTFIX)->setIcon(MobEffect::e_MobEffectIcon_Wither)->setDurationModifier(.25);
	healthBoost =		(new HealthBoostMobEffect(21, false, eMinecraftColour_Effect_HealthBoost))	->setDescriptionId(IDS_POTION_HEALTHBOOST)		->setPostfixDescriptionId(IDS_POTION_HEALTHBOOST_POSTFIX)->setIcon(MobEffect::e_MobEffectIcon_HealthBoost)->addAttributeModifier(SharedMonsterAttributes::MAX_HEALTH, eModifierId_POTION_HEALTHBOOST, 4, AttributeModifier::OPERATION_ADDITION);
	absorption =		(new AbsoptionMobEffect(22, false, eMinecraftColour_Effect_Absoprtion))		->setDescriptionId(IDS_POTION_ABSORPTION)		->setPostfixDescriptionId(IDS_POTION_ABSORPTION_POSTFIX)->setIcon(MobEffect::e_MobEffectIcon_Absorption);
	saturation =		(new InstantenousMobEffect(23, false, eMinecraftColour_Effect_Saturation))	->setDescriptionId(IDS_POTION_SATURATION)		->setPostfixDescriptionId(IDS_POTION_SATURATION_POSTFIX);
	reserved_24 = nullptr;
	reserved_25 = nullptr;
	reserved_26 = nullptr;
	reserved_27 = nullptr;
	reserved_28 = nullptr;
	reserved_29 = nullptr;
	reserved_30 = nullptr;
	reserved_31 = nullptr;
}

MobEffect::MobEffect(int id, bool isHarmful, eMinecraftColour color) : id(id), _isHarmful(isHarmful), color(color)
{
	descriptionId = -1;
	m_postfixDescriptionId = -1;
	icon = e_MobEffectIcon_None;
	_isDisabled = false;

	effects[id] = this;

	if (isHarmful)
	{
		durationModifier = .5;
	}
	else
	{
		durationModifier = 1.0;
	}
}

// 4J Removed as using different value for icon
//MobEffect *MobEffect::setIcon(int xPos, int yPos)
//{
//	icon = xPos + yPos * 8;
//	return this;
//}

MobEffect *MobEffect::setIcon(EMobEffectIcon icon)
{
	this->icon = icon;
	return this;
}

int MobEffect::getId()
{
	return id;
}

/**
* This method should perform periodic updates on the player. Mainly used
* for regeneration effects and the like. Other effects, such as blindness,
* are in effect for the whole duration of the effect.
*
* @param mob
* @param amplification
*/
void MobEffect::applyEffectTick(shared_ptr<LivingEntity> mob, int amplification)
{
	// Maybe move this to separate class implementations in the future?
	if (id == regeneration->id)
	{
		if (mob->getHealth() < mob->getMaxHealth())
		{
			mob->heal(1);
		}
	}
	else if (id == poison->id)
	{
		if (mob->getHealth() > 1)
		{
			mob->hurt(DamageSource::magic, 1);
		}
	}
	else if (id == wither->id)
	{
		mob->hurt(DamageSource::wither, 1);
	}
	else if ( (id == hunger->id) && mob->instanceof(eTYPE_PLAYER) )
	{
		// every tick, cause the same amount of exhaustion as when removing
		// a block, times amplification
		dynamic_pointer_cast<Player>(mob)->causeFoodExhaustion(FoodConstants::EXHAUSTION_MINE * (amplification + 1));
	}
	else if ( (id == saturation->id) && mob->instanceof(eTYPE_PLAYER) )
	{
		if (!mob->level->isClientSide)
		{
			dynamic_pointer_cast<Player>(mob)->getFoodData()->eat(amplification + 1, FoodConstants::FOOD_SATURATION_MAX);
		}
	}
	else if ((id == heal->id && !mob->isInvertedHealAndHarm()) || (id == harm->id && mob->isInvertedHealAndHarm()))
	{
		mob->heal(max(4 << amplification, 0));
	}
	else if ((id == harm->id && !mob->isInvertedHealAndHarm()) || (id == heal->id && mob->isInvertedHealAndHarm()))
	{
		mob->hurt(DamageSource::magic, 6 << amplification);
	}
}

void MobEffect::applyInstantenousEffect(shared_ptr<LivingEntity> source, shared_ptr<LivingEntity> mob, int amplification, double scale)
{
	if ((id == heal->id && !mob->isInvertedHealAndHarm()) || (id == harm->id && mob->isInvertedHealAndHarm()))
	{
		int amount = static_cast<int>(scale * (double)(4 << amplification) + .5);
		mob->heal(amount);
	}
	else if ((id == harm->id && !mob->isInvertedHealAndHarm()) || (id == heal->id && mob->isInvertedHealAndHarm()))
	{
		int amount = static_cast<int>(scale * (double)(6 << amplification) + .5);
		if (source == nullptr)
		{
			mob->hurt(DamageSource::magic, amount);
		}
		else
		{
			DamageSource *damageSource = DamageSource::indirectMagic(mob, source);
			mob->hurt(damageSource, amount);
			delete damageSource;
		}
	}
}

bool MobEffect::isInstantenous()
{
	return false;
}

/**
* This parameter says if the applyEffect method should be called depending
* on the remaining duration ticker. For instance, the regeneration will be
* activated every 8 ticks, healing one point of health.
*
* @param remainingDuration
* @param amplification
*            Effect amplification, starts at 0 (weakest)
* @return
*/
bool MobEffect::isDurationEffectTick(int remainingDuration, int amplification)
{

	// Maybe move this to separate class implementations in the future?
	if (id == regeneration->id)
	{
		// tick intervals are 50, 25, 12, 6..
		int interval = 50 >> amplification;
		if (interval > 0)
		{
			return (remainingDuration % interval) == 0;
		}
		return true;
	}
	else if (id == poison->id)
	{
		// tick intervals are 25, 12, 6..
		int interval = 25 >> amplification;
		if (interval > 0)
		{
			return (remainingDuration % interval) == 0;
		}
		return true;
	}
	else if (id == wither->id)
	{
		int interval = 40 >> amplification;
		if (interval > 0)
		{
			return (remainingDuration % interval) == 0;
		}
		return true;
	}
	else if (id == hunger->id)
	{
		return true;
	}

	return false;
}

MobEffect *MobEffect::setDescriptionId(unsigned int id)
{
	descriptionId = id;
	return this;
}

unsigned int MobEffect::getDescriptionId(int iData)
{
	return descriptionId;
}

MobEffect *MobEffect::setPostfixDescriptionId(unsigned int id)
{
	m_postfixDescriptionId = id;
	return this;
}

unsigned int MobEffect::getPostfixDescriptionId(int iData)
{
	return m_postfixDescriptionId;
}

bool MobEffect::hasIcon()
{
	return icon != e_MobEffectIcon_None;
}

MobEffect::EMobEffectIcon MobEffect::getIcon()
{
	return icon;
}

bool MobEffect::isHarmful()
{
	return _isHarmful;
}

wstring MobEffect::formatDuration(MobEffectInstance *instance)
{
	if (instance->isNoCounter())
	{
		return L"**:**";
	}
	int duration = instance->getDuration();

	int seconds = duration / SharedConstants::TICKS_PER_SECOND;
	int minutes = seconds / 60;
	seconds %= 60;

	wchar_t temp[8];
	ZeroMemory(&temp,8*(sizeof(wchar_t)));

	if (seconds < 10)
	{
		swprintf(temp, 8, L"%d:0%d",minutes,seconds);
		//return minutes + ":0" + seconds;
	}
	else
	{
		swprintf(temp, 8, L"%d:%d",minutes,seconds);
		//return minutes + ":" + seconds;
	}

	return temp;
}

MobEffect *MobEffect::setDurationModifier(double durationModifier)
{
	this->durationModifier = durationModifier;
	return this;
}

double MobEffect::getDurationModifier()
{
	return durationModifier;
}

MobEffect *MobEffect::setDisabled()
{
	_isDisabled = true;
	return this;
}

bool MobEffect::isDisabled()
{
	return _isDisabled;
}

eMinecraftColour MobEffect::getColor()
{
	return color;
}

MobEffect *MobEffect::addAttributeModifier(Attribute *attribute, eMODIFIER_ID id, double amount, int operation)
{
	AttributeModifier *effect = new AttributeModifier(id, amount, operation);
	attributeModifiers.insert(std::pair<Attribute*,AttributeModifier*>(attribute, effect));
	return this;
}

unordered_map<Attribute *, AttributeModifier *> *MobEffect::getAttributeModifiers()
{
	return &attributeModifiers;
}

void MobEffect::removeAttributeModifiers(shared_ptr<LivingEntity> entity, BaseAttributeMap *attributes, int amplifier)
{
	for (auto& it : attributeModifiers)
	{
		AttributeInstance *attribute = attributes->getInstance(it.first);

		if (attribute != nullptr)
		{
			attribute->removeModifier(it.second);
		}
	}
}

void MobEffect::addAttributeModifiers(shared_ptr<LivingEntity> entity, BaseAttributeMap *attributes, int amplifier)
{
	for (auto& it : attributeModifiers)
	{
		AttributeInstance *attribute = attributes->getInstance(it.first);

		if (attribute != nullptr)
		{
			AttributeModifier *original = it.second;
			attribute->removeModifier(original);
			attribute->addModifier(new AttributeModifier(original->getId(), getAttributeModifierValue(amplifier, original), original->getOperation()));
		}
	}
}

double MobEffect::getAttributeModifierValue(int amplifier, AttributeModifier *original)
{
	return original->getAmount() * (amplifier + 1);
}