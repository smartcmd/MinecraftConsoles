#include "stdafx.h"
#include "net.minecraft.world.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.effect.h"
#include "net.minecraft.world.entity.h"
#include "com.mojang.nbt.h"
#include "Spider.h"
#include "..\Minecraft.Client\Textures.h"
#include "SoundTypes.h"



Spider::Spider(Level *level) : Monster( level )
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();

	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that the derived version of the function is called
	health = getMaxHealth();

	this->textureIdx = TN_MOB_SPIDER; // 4J was L"/mob/spider.png";
	this->setSize(1.4f, 0.9f);
	runSpeed = 0.8f;
}

void Spider::defineSynchedData()
{
	Monster::defineSynchedData();

	entityData->define(DATA_FLAGS_ID, (byte) 0);
}

void Spider::tick()
{
	Monster::tick();

	if (!level->isClientSide)
	{
		// this is to synchronize the spiders' climb state
		// in multiplayer (to stop them from "flashing")
		setClimbing(horizontalCollision);
	}
}

int Spider::getMaxHealth()
{
	return 16;
}

double Spider::getRideHeight()
{
	return bbHeight * 0.75 - 0.5f;
}

bool Spider::makeStepSound()
{
	return false;
}

shared_ptr<Entity> Spider::findAttackTarget()
{
#ifndef _FINAL_BUILD
#ifdef _DEBUG_MENUS_ENABLED
	if(app.GetMobsDontAttackEnabled())
	{
		return shared_ptr<Player>();
	}
#endif
#endif

	float br = getBrightness(1);
	if (br < 0.5f)
	{
		double maxDist = 16;
		return level->getNearestAttackablePlayer(shared_from_this(), maxDist);
	}
	return shared_ptr<Entity>();
}

int Spider::getAmbientSound()
{
	return eSoundType_MOB_SPIDER_AMBIENT;
}

int Spider::getHurtSound()
{
	return eSoundType_MOB_SPIDER_AMBIENT;
}

int Spider::getDeathSound()
{
	return eSoundType_MOB_SPIDER_DEATH;
}

void Spider::checkHurtTarget(shared_ptr<Entity> target, float d) 
{
	float br = getBrightness(1);
	if (br > 0.5f && random->nextInt(100) == 0)
	{
		this->attackTarget = nullptr;
		return;
	}

	if (d > 2 && d < 6 && random->nextInt(10) == 0)
	{
		if (onGround) 
		{
			double xdd = target->x - x;
			double zdd = target->z - z;
			float dd = (float) sqrt(xdd * xdd + zdd * zdd);
			xd = (xdd / dd * 0.5f) * 0.8f + xd * 0.2f;
			zd = (zdd / dd * 0.5f) * 0.8f + zd * 0.2f;
			yd = 0.4f;
		}
	} 
	else
	{
		Monster::checkHurtTarget(target, d);
	}
}

int Spider::getDeathLoot()
{
	return Item::string->id;
}

void Spider::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel)
{
	Monster::dropDeathLoot(wasKilledByPlayer, playerBonusLevel);

	if (wasKilledByPlayer && (random->nextInt(3) == 0 || random->nextInt(1 + playerBonusLevel) > 0))
	{
		spawnAtLocation(Item::spiderEye_Id, 1);
	}
}

/**
 * The the spiders act as if they're always on a ladder, which enables them
 * to climb walls.
 */

bool Spider::onLadder() 
{
	return isClimbing();
}
	
void Spider::makeStuckInWeb()
{
	// do nothing - spiders don't get stuck in web
}

float Spider::getModelScale()
{
	return 1.0f;
}

MobType Spider::getMobType()
{
	return ARTHROPOD;
}

bool Spider::canBeAffected(MobEffectInstance *newEffect)
{
	if (newEffect->getId() == MobEffect::poison->id)
	{
		return false;
	}
	return Monster::canBeAffected(newEffect);
}

bool Spider::isClimbing()
{
	return (entityData->getByte(DATA_FLAGS_ID) & 0x1) != 0;
}

void Spider::setClimbing(bool value)
{
	byte flags = entityData->getByte(DATA_FLAGS_ID);
	if (value)
	{
		flags |= 0x1;
	}
	else
	{
		flags &= ~0x1;
	}
	entityData->set(DATA_FLAGS_ID, flags);
}