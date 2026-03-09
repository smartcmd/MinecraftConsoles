#include "stdafx.h"
#include "net.minecraft.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.h"
#include "JavaMath.h"
#include "DragonFireball.h"



const double DragonFireball::SPLASH_RANGE = 4.0;
const double DragonFireball::SPLASH_RANGE_SQ = DragonFireball::SPLASH_RANGE * DragonFireball::SPLASH_RANGE;

DragonFireball::DragonFireball(Level *level) : Fireball(level)
{
	setSize(5 / 16.0f, 5 / 16.0f);
}

DragonFireball::DragonFireball(Level *level, shared_ptr<LivingEntity> mob, double xa, double ya, double za) : Fireball(level, mob, xa, ya, za)
{
	setSize(5 / 16.0f, 5 / 16.0f);
}

DragonFireball::DragonFireball(Level *level, double x, double y, double z, double xa, double ya, double za) : Fireball(level, x, y, z, xa, ya, za)
{
	setSize(5 / 16.0f, 5 / 16.0f);
}

void DragonFireball::onHit(HitResult *res)
{
	if (!level->isClientSide)
	{
		AABB *aoe = bb->grow(SPLASH_RANGE, SPLASH_RANGE / 2, SPLASH_RANGE);
		vector<shared_ptr<Entity> > *entitiesOfClass = level->getEntitiesOfClass(typeid(LivingEntity), aoe);
		if ( entitiesOfClass )
		{
			if (!entitiesOfClass->empty())
			{
				for (auto& it : *entitiesOfClass)
				{
					shared_ptr<LivingEntity> e = dynamic_pointer_cast<LivingEntity>(it);
					double dist = distanceToSqr(e);
					if (dist < SPLASH_RANGE_SQ)
					{
						double scale = 1.0 - (sqrt(dist) / SPLASH_RANGE);
						if (e == res->entity)
						{
							scale = 1;
						}
						e->hurt(DamageSource::dragonbreath, 8 * scale);
					}
				}
			}
			delete entitiesOfClass;
		}
		level->levelEvent(LevelEvent::ENDERDRAGON_FIREBALL_SPLASH, static_cast<int>(Math::round(x)), static_cast<int>(Math::round(y)), static_cast<int>(Math::round(z)), 0);

		remove();
	}
}

bool DragonFireball::isPickable()
{
	return false;
}

bool DragonFireball::hurt(DamageSource *source, float damage)
{
	return false;
}

ePARTICLE_TYPE DragonFireball::getTrailParticleType()
{
	return eParticleType_dragonbreath;
}

bool DragonFireball::shouldBurn()
{
	return false;
}