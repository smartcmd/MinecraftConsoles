#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.item.h"
#include "../Minecraft.Client/Textures.h"
#include "LavaSlime.h"
#include "SoundTypes.h"



LavaSlime::LavaSlime(Level *level) : Slime(level)
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	// 4J Stu - The Slime ctor has already called this, and as we don't override it here don't need to call it
	//this->defineSynchedData();
	registerAttributes();

	fireImmune = true;
}

void LavaSlime::registerAttributes()
{
	Slime::registerAttributes();

	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.2f);
}

bool LavaSlime::canSpawn()
{
	return level->difficulty > Difficulty::PEACEFUL && level->isUnobstructed(bb) && level->getCubes(shared_from_this(), bb)->empty() && !level->containsAnyLiquid(bb);
}

int LavaSlime::getArmorValue()
{
	return getSize() * 3;
}

int LavaSlime::getLightColor(float a)
{
	return 15 << 20 | 15 << 4;
}

float LavaSlime::getBrightness(float a)
{
	return 1.0f;
}

ePARTICLE_TYPE LavaSlime::getParticleName()
{
	return eParticleType_flame;
}

shared_ptr<Slime> LavaSlime::createChild()
{
	return std::make_shared<LavaSlime>(level);
}

int LavaSlime::getDeathLoot()
{
	// 4J-PB - brought forward the magma cream drops
	return Item::magmaCream_Id;
}

void LavaSlime::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel)
{
	int loot = getDeathLoot();
	if (loot > 0 && getSize() > 1)
	{
		int count = random->nextInt(4) - 2;
		if (playerBonusLevel > 0)
		{
			count += random->nextInt(playerBonusLevel + 1);
		}
		for (int i = 0; i < count; i++)
		{
			spawnAtLocation(loot, 1);
		}
	}
}

bool LavaSlime::isOnFire()
{
	return false;
}


int LavaSlime::getJumpDelay()
{
	return Slime::getJumpDelay() * 4;
}

void LavaSlime::decreaseSquish()
{
	targetSquish = targetSquish * 0.90f;
}

void LavaSlime::jumpFromGround()
{
	yd = 0.42f + getSize() * .1f;
	hasImpulse = true;
}

void LavaSlime::causeFallDamage(float distance)
{
}

bool LavaSlime::isDealsDamage()
{
	return true;
}

int LavaSlime::getAttackDamage()
{
	return Slime::getAttackDamage() + 2;
}

int LavaSlime::getHurtSound()
{
	return getSize() > 1 ? eSoundType_MOB_SLIME_BIG : eSoundType_MOB_SLIME_SMALL;
}

int LavaSlime::getDeathSound()
{
	return getSize() > 1 ? eSoundType_MOB_SLIME_BIG : eSoundType_MOB_SLIME_SMALL;
}

int LavaSlime::getSquishSound()
{
	if (getSize() > 1)
	{
		return eSoundType_MOB_MAGMACUBE_BIG;
	}
	return eSoundType_MOB_MAGMACUBE_SMALL;
}

bool LavaSlime::isInLava()
{
	// hack that makes the lava slimes move freely on the bottom of the lava
	// oceans
	return false;
}

bool LavaSlime::doPlayLandSound()
{
	return true;
}