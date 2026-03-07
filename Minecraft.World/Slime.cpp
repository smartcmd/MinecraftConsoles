#include "stdafx.h"
#include "net.minecraft.world.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.storage.h"
#include "net.minecraft.world.level.chunk.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.damagesource.h"
#include "com.mojang.nbt.h"
#include "Slime.h"
#include "..\Minecraft.Client\Textures.h"
#include "SoundTypes.h"



void Slime::_init()
{
	jumpDelay = 0;

	targetSquish = 0;
	squish = 0;
	oSquish = 0;
}

Slime::Slime(Level *level) : Mob( level )
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());

	_init();

	int size = 1 << (random->nextInt(3));
	heightOffset = 0;
	jumpDelay = random->nextInt(20) + 10;
	setSize(size);
}

void Slime::defineSynchedData()
{
	Mob::defineSynchedData();

	entityData->define(ID_SIZE, (byte) 1);
}

void Slime::setSize(int size)
{
	entityData->set(ID_SIZE, (byte) size);
	setSize(0.6f * size, 0.6f * size);
	setPos(x, y, z);
	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(size * size);
	setHealth(getMaxHealth());
	xpReward = size;
}

int Slime::getSize()
{
	return entityData->getByte(ID_SIZE);
}

void Slime::addAdditonalSaveData(CompoundTag *tag)
{
	Mob::addAdditonalSaveData(tag);
	tag->putInt(L"Size", getSize() - 1);
}

void Slime::readAdditionalSaveData(CompoundTag *tag)
{
	Mob::readAdditionalSaveData(tag);
	setSize(tag->getInt(L"Size") + 1);
}

ePARTICLE_TYPE Slime::getParticleName()
{
	return eParticleType_slime;
}

int Slime::getSquishSound()
{
	return getSize() > 1 ? eSoundType_MOB_SLIME_BIG : eSoundType_MOB_SLIME_SMALL;
}

void Slime::tick()
{
	if (!level->isClientSide && level->difficulty == Difficulty::PEACEFUL && getSize() > 0)
	{
		removed = true;
	}

	squish = squish + (targetSquish - squish) * .5f;

	oSquish = squish;
	bool wasOnGround = onGround;
	Mob::tick();
	if (onGround && !wasOnGround)
	{
		int size = getSize();
		for (int i = 0; i < size * 8; i++)
		{
			float dir = random->nextFloat() * PI * 2;
			float d = random->nextFloat() * 0.5f + 0.5f;
			float xd = Mth::sin(dir) * size * 0.5f * d;
			float zd = Mth::cos(dir) * size * 0.5f * d;
			level->addParticle(getParticleName(), x + xd, bb->y0, z + zd, 0, 0, 0);
		}

		if (doPlayLandSound()) 
		{
			playSound(getSquishSound(), getSoundVolume(), ((random->nextFloat() - random->nextFloat()) * 0.2f + 1.0f) / 0.8f);
		}
		targetSquish = -0.5f;
	}
	// 4J Stu - Brought forward from 1.3 in TU7 to fix lava slime render
	else if (!onGround && wasOnGround)
	{
		targetSquish = 1;
	}
	decreaseSquish();

	if (level->isClientSide)
	{
		int size = getSize();
		setSize(0.6f * size, 0.6f * size);
	}
}

void Slime::serverAiStep() 
{
	checkDespawn();
	shared_ptr<Player> player = level->getNearestAttackablePlayer(shared_from_this(), 16);
	if (player != NULL)
	{
		lookAt(player, 10, 20);
	}
	if (onGround && jumpDelay-- <= 0) 
	{
		jumpDelay = getJumpDelay();
		if (player != NULL)
		{
			jumpDelay /= 3;
		}
		jumping = true;
		if (doPlayJumpSound())
		{
			playSound(getSquishSound(), getSoundVolume(), ((random->nextFloat() - random->nextFloat()) * 0.2f + 1.0f) * 0.8f);
		}

		// 4J Removed TU7 to bring forward change to fix lava slime render in MP
		//targetSquish = 1;
		xxa = 1 - random->nextFloat() * 2;
		yya = (float) 1 * getSize();
	} 
	else
	{
		jumping = false;
		if (onGround)
		{
			xxa = yya = 0;
		}
	}
}

void Slime::decreaseSquish()
{
	targetSquish = targetSquish * 0.6f;
}

int Slime::getJumpDelay()
{
	return random->nextInt(20) + 10;
}

shared_ptr<Slime> Slime::createChild()
{
	return shared_ptr<Slime>( new Slime(level) );
}

void Slime::remove()
{
	int size = getSize();
	if (!level->isClientSide && size > 1 && getHealth() <= 0)
	{
		int count = 2 + random->nextInt(3);
		for (int i = 0; i < count; i++)
		{
			// The mob spawner can currently make a maximum of 25 slimes (limited to 50% of the total amount of monsters which is 50)
			// and so limit to slightly more than this so we have some head room to make a few spawned children. Also always create at least one
			// new slime since we are getting rid of this one anyway.
			if( i == 0 || level->countInstanceOf( eTYPE_SLIME, true) < 35 )
			{
				float xd = (i % 2 - 0.5f) * size / 4.0f;
				float zd = (i / 2 - 0.5f) * size / 4.0f;
				shared_ptr<Slime> slime = createChild();
				slime->setSize(size / 2);
				slime->moveTo(x + xd, y + 0.5, z + zd, random->nextFloat() * 360, 0);
				level->addEntity(slime);
			}
		}
	}
	Mob::remove();
}

void Slime::playerTouch(shared_ptr<Player> player)
{
	if (isDealsDamage())
	{
		int size = getSize();
		if (canSee(player) && distanceToSqr(player) < (0.6 * size) * (0.6 * size))
		{
			DamageSource *damageSource = DamageSource::mobAttack( dynamic_pointer_cast<Mob>( shared_from_this() ) );
			if (player->hurt(damageSource, getAttackDamage()))
			{
				playSound(eSoundType_MOB_SLIME_ATTACK, 1, (random->nextFloat() - random->nextFloat()) * 0.2f + 1.0f);
			}
			delete damageSource;
		}
	}
}

bool Slime::isDealsDamage()
{
	return getSize() > 1;
}

int Slime::getAttackDamage()
{
	return getSize();
}

int Slime::getHurtSound() 
{
	return getSize() > 1 ? eSoundType_MOB_SLIME_BIG : eSoundType_MOB_SLIME_SMALL;
}

int Slime::getDeathSound()
{
	return getSize() > 1 ? eSoundType_MOB_SLIME_BIG : eSoundType_MOB_SLIME_SMALL;
}

int Slime::getDeathLoot()
{
	if (getSize() == 1) return Item::slimeBall->id;
	return 0;
}

bool Slime::canSpawn()
{
	LevelChunk *lc = level->getChunkAt( Mth::floor(x), Mth::floor(z));
	if (level->getLevelData()->getGenerator() == LevelType::lvl_flat && random->nextInt(4) != 1)
	{
		return false;
	}
	Random *lcr = lc->getRandom(987234911l);	// 4J - separated out so we can delete
	if ((getSize() == 1 || level->difficulty > Difficulty::PEACEFUL))
	{
		// spawn slime in swamplands at night
		Biome *biome = level->getBiome(Mth::floor(x), Mth::floor(z));

		if (biome == Biome::swampland && y > 50 && y < 70 && random->nextFloat() < 0.5f)
		{
			if (random->nextFloat() < level->getMoonBrightness() && level->getRawBrightness(Mth::floor(x), Mth::floor(y), Mth::floor(z)) <= random->nextInt(8))
			{
				return Mob::canSpawn();
			}
		}
		if (random->nextInt(10) == 0 && lcr->nextInt(10) == 0 && y < 40)
		{
			return Mob::canSpawn();
		}
	}

	delete lcr;
	return false;
}

float Slime::getSoundVolume() 
{
	return 0.4f * getSize();
}

int Slime::getMaxHeadXRot()
{
	return 0;
}

bool Slime::doPlayJumpSound()
{
	return getSize() > 0;
}

bool Slime::doPlayLandSound()
{
	return getSize() > 2;
}