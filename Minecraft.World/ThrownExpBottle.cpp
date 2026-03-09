#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "JavaMath.h"
#include "ThrownExpBottle.h"



ThrownExpBottle::ThrownExpBottle(Level *level) : Throwable(level)
{
}

ThrownExpBottle::ThrownExpBottle(Level *level, shared_ptr<LivingEntity> mob) : Throwable(level,mob)
{
}

ThrownExpBottle::ThrownExpBottle(Level *level, double x, double y, double z) : Throwable(level, x, y, z)
{
}


float ThrownExpBottle::getGravity()
{
	return 0.07f;
}

float ThrownExpBottle::getThrowPower()
{
	return 0.7f;
}

float ThrownExpBottle::getThrowUpAngleOffset()
{
	return -20;
}

void ThrownExpBottle::onHit(HitResult *res)
{

	if (!level->isClientSide)
	{
		level->levelEvent(LevelEvent::PARTICLES_POTION_SPLASH, static_cast<int>(Math::round(x)), static_cast<int>(Math::round(y)), static_cast<int>(Math::round(z)), 0);

		int xpCount = 3 + level->random->nextInt(5) + level->random->nextInt(5);
		while (xpCount > 0)
		{
			int newCount = ExperienceOrb::getExperienceValue(xpCount);
			xpCount -= newCount;
			level->addEntity(std::make_shared<ExperienceOrb>(level, x, y, z, newCount));
		}

		remove();
	}
}