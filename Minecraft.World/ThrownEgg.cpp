#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.damagesource.h"
#include "com.mojang.nbt.h"
#include "ThrownEgg.h"
#include "MobCategory.h"



void ThrownEgg::_init()
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();
}

ThrownEgg::ThrownEgg(Level *level) : Throwable(level)
{
	_init();
}

ThrownEgg::ThrownEgg(Level *level, shared_ptr<LivingEntity> mob) : Throwable(level,mob)
{
	_init();
}

ThrownEgg::ThrownEgg(Level *level, double x, double y, double z) : Throwable(level,x,y,z)
{
	_init();
}

void ThrownEgg::onHit(HitResult *res)
{
	if (res->entity != nullptr)
	{
		DamageSource *damageSource = DamageSource::thrown(shared_from_this(), owner);
		res->entity->hurt(damageSource, 0);
		delete damageSource;
	}
	if (!level->isClientSide && random->nextInt(8) == 0)
	{
		if(level->canCreateMore( eTYPE_CHICKEN, Level::eSpawnType_Breed) )	// 4J - added limit for number of chickens in world
		{
			int count = 1;
			if (random->nextInt(32) == 0) count = 4;
			for (int i = 0; i < count; i++)
			{
				shared_ptr<Chicken> chicken = std::make_shared<Chicken>(level);
				chicken->setAge(-20 * 60 * 20);

				chicken->moveTo(x, y, z, yRot, 0);
				chicken->setDespawnProtected();		// 4J added, default to being protected against despawning
				level->addEntity(chicken);
			}
		}
	}

	for (int i = 0; i < 8; i++)
		level->addParticle(eParticleType_snowballpoof, x, y, z, 0, 0, 0);

	if (!level->isClientSide)
	{
		remove();
	}
}
