#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.entity.monster.h"
#include "Snowball.h"



void Snowball::_init()
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();
}

Snowball::Snowball(Level *level) : Throwable(level)
{
	_init();
}

Snowball::Snowball(Level *level, shared_ptr<LivingEntity> mob) : Throwable(level,mob)
{
	_init();
}

Snowball::Snowball(Level *level, double x, double y, double z) : Throwable(level,x,y,z)
{
	_init();
}

void Snowball::onHit(HitResult *res)
{
	if (res->entity != nullptr)
	{
		int damage = 0;
		if ( res->entity->instanceof(eTYPE_BLAZE) )
		{
			damage = 3;
		}

		DamageSource *damageSource = DamageSource::thrown(shared_from_this(), getOwner());
		res->entity->hurt(damageSource, damage);
		delete damageSource;
	}
	for (int i = 0; i < 8; i++)
		level->addParticle(eParticleType_snowballpoof, x, y, z, 0, 0, 0);
	if (!level->isClientSide)
	{
		remove();
	}
}