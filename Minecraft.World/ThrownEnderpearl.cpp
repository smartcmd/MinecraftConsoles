#include "stdafx.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.level.h"
#include "..\Minecraft.Client\ServerPlayer.h"
#include "..\Minecraft.Client\PlayerConnection.h"
#include "ThrownEnderpearl.h"



ThrownEnderpearl::ThrownEnderpearl(Level *level) : Throwable(level)
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();
}

ThrownEnderpearl::ThrownEnderpearl(Level *level, shared_ptr<LivingEntity> mob) : Throwable(level,mob)
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();
}

ThrownEnderpearl::ThrownEnderpearl(Level *level, double x, double y, double z) : Throwable(level,x,y,z)
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();
}

void ThrownEnderpearl::onHit(HitResult *res)
{
	if (res->entity != NULL)
	{
		DamageSource *damageSource = DamageSource::thrown(shared_from_this(), getOwner() );
		res->entity->hurt(damageSource, 0);
		delete damageSource;
	}
	for (int i = 0; i < 32; i++)
	{
		level->addParticle(eParticleType_ender, x, y + random->nextDouble() * 2, z, random->nextGaussian(), 0, random->nextGaussian());
	}

	if (!level->isClientSide)
	{
		// Fix for #67486 - TCR #001: BAS Game Stability: Customer Encountered: TU8: Code: Gameplay: The title crashes on Host's console when Client Player leaves the game before the Ender Pearl thrown by him touches the ground.
		// If the owner has been removed, then ignore

		// 4J-JEV: Cheap type check first.
		if ( (getOwner() != NULL) && getOwner()->instanceof(eTYPE_SERVERPLAYER) )
		{
			shared_ptr<ServerPlayer> serverPlayer = dynamic_pointer_cast<ServerPlayer>(getOwner() );
			if (!serverPlayer->removed)
			{
				if(!serverPlayer->connection->done && serverPlayer->level == this->level)
				{
					if (getOwner()->isRiding())
					{
						getOwner()->ride(nullptr);
					}
					getOwner()->teleportTo(x, y, z);
					getOwner()->fallDistance = 0;
					getOwner()->hurt(DamageSource::fall, 5);
				}
			}
		}
		remove();
	}
}