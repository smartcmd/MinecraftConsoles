#pragma once

#include "MobEffect.h"

class LivingEntity;
class BaseAttributeMap;

class HealthBoostMobEffect : public MobEffect
{
public:
	HealthBoostMobEffect(int id, bool isHarmful, eMinecraftColour color);

	void removeAttributeModifiers(shared_ptr<LivingEntity> entity, BaseAttributeMap *attributes, int amplifier);
};