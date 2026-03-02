#pragma once

class LivingEntity;

#include "MobEffect.h"

class AbsoptionMobEffect : public MobEffect
{
public:
	AbsoptionMobEffect(int id, bool isHarmful, eMinecraftColour color);

	void removeAttributeModifiers(shared_ptr<LivingEntity> entity, BaseAttributeMap *attributes, int amplifier);
	void addAttributeModifiers(shared_ptr<LivingEntity> entity, BaseAttributeMap *attributes, int amplifier);
};