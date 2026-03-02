#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.effect.h"
#include "AbsoptionMobEffect.h"

AbsoptionMobEffect::AbsoptionMobEffect(int id, bool isHarmful, eMinecraftColour color) : MobEffect(id, isHarmful, color)
{
}

void AbsoptionMobEffect::removeAttributeModifiers(shared_ptr<LivingEntity> entity, BaseAttributeMap *attributes, int amplifier)
{
	entity->setAbsorptionAmount(entity->getAbsorptionAmount() - 4 * (amplifier + 1));
	MobEffect::removeAttributeModifiers(entity, attributes, amplifier);
}

void AbsoptionMobEffect::addAttributeModifiers(shared_ptr<LivingEntity> entity, BaseAttributeMap *attributes, int amplifier)
{
	entity->setAbsorptionAmount(entity->getAbsorptionAmount() + 4 * (amplifier + 1));
	MobEffect::addAttributeModifiers(entity, attributes, amplifier);
}
