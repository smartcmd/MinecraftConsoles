#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.material.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.damagesource.h"
#include "com.mojang.nbt.h"
#include "JavaMath.h"
#include "SharedConstants.h"
#include "ExperienceOrb.h"
#include "SoundTypes.h"



const int ExperienceOrb::LIFETIME = 5 * 60 * SharedConstants::TICKS_PER_SECOND; // Five minutes!

void ExperienceOrb::_init()
{
	tickCount = 0;
	age = 0;

	throwTime = 0;

	health = 5;
	value = 0;
	followingPlayer = nullptr;
	followingTime = 0;
}

ExperienceOrb::ExperienceOrb(Level *level, double x, double y, double z, int count) : Entity(level)
{
	_init();

	setSize(0.5f, 0.5f);
	heightOffset = bbHeight / 2.0f;
	setPos(x, y, z);

	yRot = (float) (Math::random() * 360);

	xd = (float) (Math::random() * 0.2f - 0.1f) * 2;
	yd = (float) (Math::random() * 0.2) * 2;
	zd = (float) (Math::random() * 0.2f - 0.1f) * 2;

	value = count;
}

bool ExperienceOrb::makeStepSound()
{
	return false;
}

ExperienceOrb::ExperienceOrb(Level *level) : Entity( level )
{
	_init();

	setSize(0.25f, 0.25f);
	heightOffset = bbHeight / 2.0f;
}

void ExperienceOrb::defineSynchedData()
{
}

int ExperienceOrb::getLightColor(float a)
{
	float l = 0.5f;
	if (l < 0) l = 0;
	if (l > 1) l = 1;
	int br = Entity::getLightColor(a);

	int br1 = (br) & 0xff;
	int br2 = (br >> 16) & 0xff;
	br1 += (int) (l * 15 * 16);
	if (br1 > 15 * 16) br1 = 15 * 16;
	//        br2 = 15*16;
	return br1 | br2 << 16;
}

void ExperienceOrb::tick()
{
	Entity::tick();
	if (throwTime > 0) throwTime--;
	xo = x;
	yo = y;
	zo = z;

	yd -= 0.03f;
	if (level->getMaterial(Mth::floor(x), Mth::floor(y), Mth::floor(z)) == Material::lava)
	{
		yd = 0.2f;
		xd = (random->nextFloat() - random->nextFloat()) * 0.2f;
		zd = (random->nextFloat() - random->nextFloat()) * 0.2f;
		level->playSound(shared_from_this(), eSoundType_RANDOM_FIZZ, 0.4f, 2.0f + random->nextFloat() * 0.4f);
	}
	checkInTile(x, (bb->y0 + bb->y1) / 2, z);

	double maxDist = 8;
	// 4J - PC Comment
	// Usually exp orbs will get created at the same time so smoothen the lagspikes
	if (followingTime < tickCount - SharedConstants::TICKS_PER_SECOND + (entityId % 100))
	{
		if (followingPlayer == NULL || followingPlayer->distanceToSqr(shared_from_this()) > maxDist * maxDist)
		{
			followingPlayer = level->getNearestPlayer(shared_from_this(), maxDist);
		}
		followingTime = tickCount;
	}
	if (followingPlayer != NULL)
	{
		double xdd = (followingPlayer->x - x) / maxDist;
		double ydd = (followingPlayer->y + followingPlayer->getHeadHeight() - y) / maxDist;
		double zdd = (followingPlayer->z - z) / maxDist;
		double dd = sqrt(xdd * xdd + ydd * ydd + zdd * zdd);
		double power = 1 - dd;
		if (power > 0)
		{
			power = power * power;
			xd += xdd / dd * power * 0.1;
			yd += ydd / dd * power * 0.1;
			zd += zdd / dd * power * 0.1;
		}
	}

	move(xd, yd, zd);

	float friction = 0.98f;
	if (onGround)
	{
		friction = 0.6f * 0.98f;
		int t = level->getTile(Mth::floor(x), Mth::floor(bb->y0) - 1, Mth::floor(z));
		if (t > 0)
		{
			friction = Tile::tiles[t]->friction * 0.98f;
		}
	}

	xd *= friction;
	yd *= 0.98f;
	zd *= friction;

	if (onGround)
	{
		yd *= -0.9f;
	}

	tickCount++;

	age++;
	if (age >= LIFETIME)
	{
		remove();
	}
}

bool ExperienceOrb::updateInWaterState()
{
	return level->checkAndHandleWater(bb, Material::water, shared_from_this());
}

void ExperienceOrb::burn(int dmg)
{
	hurt(DamageSource::inFire, dmg);
}

bool ExperienceOrb::hurt(DamageSource *source, int damage)
{
	markHurt();
	health -= damage;
	if (health <= 0)
	{
		remove();
	}
	return false;
}

void ExperienceOrb::addAdditonalSaveData(CompoundTag *entityTag)
{
	entityTag->putShort(L"Health", (byte) health);
	entityTag->putShort(L"Age", (short) age);
	entityTag->putShort(L"Value", (short) value);
}

void ExperienceOrb::readAdditionalSaveData(CompoundTag *tag)
{
	health = tag->getShort(L"Health") & 0xff;
	age = tag->getShort(L"Age");
	value = tag->getShort(L"Value");
}

void ExperienceOrb::playerTouch(shared_ptr<Player> player)
{
	if (level->isClientSide) return;

	if (throwTime == 0 && player->takeXpDelay == 0)
	{
		player->takeXpDelay = 2;
		// 4J - sound change brought forward from 1.2.3
		level->playSound(shared_from_this(), eSoundType_RANDOM_ORB, 0.1f, 0.5f * ((random->nextFloat() - random->nextFloat()) * 0.7f + 1.8f));
		player->take(shared_from_this(), 1);
		player->increaseXp(value);
		remove();
	}
}

int ExperienceOrb::getValue()
{
	return value;
}

int ExperienceOrb::getIcon()
{

	if (value >= 2477)
	{
		return 10;
	}
	else if (value >= 1237)
	{
		return 9;
	}
	else if (value >= 617)
	{
		return 8;
	}
	else if (value >= 307)
	{
		return 7;
	}
	else if (value >= 149)
	{
		return 6;
	}
	else if (value >= 73)
	{
		return 5;
	}
	else if (value >= 37)
	{
		return 4;
	}
	else if (value >= 17)
	{
		return 3;
	}
	else if (value >= 7)
	{
		return 2;
	}
	else if (value >= 3)
	{
		return 1;
	}

	return 0;
}

/**
* Fetches the biggest possible experience orb value based on a maximum
* value. The current algorithm is next prime which is at least twice more
* than the previous one.
* 
* @param maxValue
* @return
*/
int ExperienceOrb::getExperienceValue(int maxValue)
{

	if (maxValue >= 2477)
	{
		return 2477;
	}
	else if (maxValue >= 1237)
	{
		return 1237;
	}
	else if (maxValue >= 617)
	{
		return 617;
	}
	else if (maxValue >= 307)
	{
		return 307;
	}
	else if (maxValue >= 149)
	{
		return 149;
	}
	else if (maxValue >= 73)
	{
		return 73;
	}
	else if (maxValue >= 37)
	{
		return 37;
	}
	else if (maxValue >= 17)
	{
		return 17;
	}
	else if (maxValue >= 7)
	{
		return 7;
	}
	else if (maxValue >= 3)
	{
		return 3;
	}

	return 1;
}

bool ExperienceOrb::isAttackable()
{
	return false;
}

// 4J added
bool ExperienceOrb::shouldRender(Vec3 *c)
{
	double xd = x - c->x;
	double yd = y - c->y;
	double zd = z - c->z;
	double distance = xd * xd + yd * yd + zd * zd;

	// 4J - don't render experience orbs that are less than 2 metres away, to try and avoid large particles that are causing us problems with photosensitivity testing - issues when you go
	// near a large pile of experience orbs that all rush towards the near clip plane
	if( distance < 4 ) return false;

	return Entity::shouldRender(c);
}