#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "HealthBoostMobEffect.h"

HealthBoostMobEffect::HealthBoostMobEffect(int id, bool isHarmful, eMinecraftColour color) : MobEffect(id, isHarmful, color)
{
}

void HealthBoostMobEffect::removeAttributeModifiers(shared_ptr<LivingEntity> entity, BaseAttributeMap *attributes, int amplifier)
{
	MobEffect::removeAttributeModifiers(entity, attributes, amplifier);
	if (entity->getHealth() > entity->getMaxHealth())
	{
		entity->setHealth(entity->getMaxHealth());
	}
}
