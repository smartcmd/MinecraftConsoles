#include "stdafx.h"
#include "net.minecraft.world.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.effect.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.monster.h"
#include "com.mojang.nbt.h"
#include "BasicTypeContainers.h"
#include "Spider.h"
#include "..\Minecraft.Client\Textures.h"
#include "SoundTypes.h"



Spider::Spider(Level *level) : Monster( level )
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());

	this->setSize(1.4f, 0.9f);
}

void Spider::defineSynchedData()
{
	Monster::defineSynchedData();

	entityData->define(DATA_FLAGS_ID, static_cast<byte>(0));
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

void Spider::registerAttributes()
{
	Monster::registerAttributes();

	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(16);
	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.8f);
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

void Spider::playStepSound(int xt, int yt, int zt, int t)
{
	playSound(eSoundType_MOB_SPIDER_STEP, 0.15f, 1);
}

void Spider::checkHurtTarget(shared_ptr<Entity> target, float d) 
{
	float br = getBrightness(1);
	if (br > 0.5f && random->nextInt(100) == 0)
	{
		attackTarget = nullptr;
		return;
	}

	if (d > 2 && d < 6 && random->nextInt(10) == 0)
	{
		if (onGround) 
		{
			double xdd = target->x - x;
			double zdd = target->z - z;
			float dd = static_cast<float>(sqrt(xdd * xdd + zdd * zdd));
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

MobGroupData *Spider::finalizeMobSpawn(MobGroupData *groupData, int extraData /*= 0*/) // 4J Added extraData param
{
	groupData = Monster::finalizeMobSpawn(groupData);

#ifndef _CONTENT_PACKAGE
	// 4J-JEV: Added for spider-jockey spawn-egg.
	if ( (level->random->nextInt(100) == 0) || (extraData != 0) )
#else
	if (level->random->nextInt(100) == 0)
#endif
	{
		shared_ptr<Skeleton> skeleton = std::make_shared<Skeleton>(level);
		skeleton->moveTo(x, y, z, yRot, 0);
		skeleton->finalizeMobSpawn(nullptr);
		level->addEntity(skeleton);
		skeleton->ride(shared_from_this());
	}

	if (groupData == nullptr)
	{
		groupData = new SpiderEffectsGroupData();

		if (level->difficulty > Difficulty::NORMAL && level->random->nextFloat() < SPIDER_SPECIAL_EFFECT_CHANCE * level->getDifficulty(x, y, z))
		{
			static_cast<SpiderEffectsGroupData *>(groupData)->setRandomEffect(level->random);
		}
	}
	if ( dynamic_cast<SpiderEffectsGroupData *>( groupData ) != nullptr)
	{
		int effect = static_cast<SpiderEffectsGroupData *>(groupData)->effectId;
		if (effect > 0 && MobEffect::effects[effect] != nullptr)
		{
			addEffect(new MobEffectInstance(effect, Integer::MAX_VALUE));
		}
	}

	return groupData;
}

const float Spider::SPIDER_SPECIAL_EFFECT_CHANCE = .1f;

Spider::SpiderEffectsGroupData::SpiderEffectsGroupData()
{
	effectId = 0;
}

void Spider::SpiderEffectsGroupData::setRandomEffect(Random *random)
{
	int selection = random->nextInt(5);
	if (selection <= 1)
	{
		effectId = MobEffect::movementSpeed->id;
	}
	else if (selection <= 2)
	{
		effectId = MobEffect::damageBoost->id;
	}
	else if (selection <= 3)
	{
		effectId = MobEffect::regeneration->id;
	}
	else if (selection <= 4)
	{
		effectId = MobEffect::invisibility->id;
	}
}