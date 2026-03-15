#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "Bat.h"

Bat::Bat(Level *level) : AmbientCreature(level)
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());

	targetPosition = nullptr;

	setSize(.5f, .9f);
	setResting(true);
}

void Bat::defineSynchedData()
{
	AmbientCreature::defineSynchedData();

	entityData->define(DATA_ID_FLAGS, (char) 0);
}

float Bat::getSoundVolume()
{
	return 0.8f;
}

float Bat::getVoicePitch()
{
	return AmbientCreature::getVoicePitch() * .95f;
}

int Bat::getAmbientSound()
{
	if (isResting() && random->nextInt(4) != 0)
	{
		return -1;
	}
	return eSoundType_MOB_BAT_IDLE; //"mob.bat.idle";
}

int Bat::getHurtSound()
{
	return eSoundType_MOB_BAT_HURT; //"mob.bat.hurt";
}

int Bat::getDeathSound()
{
	return eSoundType_MOB_BAT_DEATH; //"mob.bat.death";
}

bool Bat::isPushable()
{
	// bats can't be pushed by other mobs
	return false;
}

void Bat::doPush(shared_ptr<Entity> e)
{
	// bats don't push other mobs
}

void Bat::pushEntities()
{
	// bats don't push other mobs
}

void Bat::registerAttributes()
{
	AmbientCreature::registerAttributes();

	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(6);
}

bool Bat::isResting()
{
	return (entityData->getByte(DATA_ID_FLAGS) & FLAG_RESTING) != 0;
}

void Bat::setResting(bool value)
{
	char current = entityData->getByte(DATA_ID_FLAGS);
	if (value)
	{
		entityData->set(DATA_ID_FLAGS, static_cast<char>(current | FLAG_RESTING));
	}
	else
	{
		entityData->set(DATA_ID_FLAGS, static_cast<char>(current & ~FLAG_RESTING));
	}
}

bool Bat::useNewAi()
{
	return true;
}

void Bat::tick()
{

	AmbientCreature::tick();

	if (isResting())
	{
		xd = yd = zd = 0;
		y = Mth::floor(y) + 1.0 - bbHeight;
	}
	else
	{
		yd *= .6f;
	}

}

inline int signum(double x) { return (x > 0) - (x < 0); }

void Bat::newServerAiStep()
{
	AmbientCreature::newServerAiStep();

	if (isResting())
	{
		if (!level->isSolidBlockingTile(Mth::floor(x), static_cast<int>(y) + 1, Mth::floor(z)))
		{
			setResting(false);
			level->levelEvent(nullptr, LevelEvent::SOUND_BAT_LIFTOFF, static_cast<int>(x), static_cast<int>(y), static_cast<int>(z), 0);
		}
		else
		{

			if (random->nextInt(200) == 0)
			{
				yHeadRot = random->nextInt(360);
			}

			if (level->getNearestPlayer(shared_from_this(), 4.0f) != nullptr)
			{
				setResting(false);
				level->levelEvent(nullptr, LevelEvent::SOUND_BAT_LIFTOFF, static_cast<int>(x), static_cast<int>(y), static_cast<int>(z), 0);
			}
		}
	}
	else
	{

		if (targetPosition != nullptr && (!level->isEmptyTile(targetPosition->x, targetPosition->y, targetPosition->z) || targetPosition->y < 1))
		{
			delete targetPosition;
			targetPosition = nullptr;
		}
		if (targetPosition == nullptr || random->nextInt(30) == 0 || targetPosition->distSqr(static_cast<int>(x), static_cast<int>(y), static_cast<int>(z)) < 4)
		{
			delete targetPosition;
			targetPosition = new Pos(static_cast<int>(x) + random->nextInt(7) - random->nextInt(7), static_cast<int>(y) + random->nextInt(6) - 2, static_cast<int>(z) + random->nextInt(7) - random->nextInt(7));
		}

		double dx = (targetPosition->x + .5) - x;
		double dy = (targetPosition->y + .1) - y;
		double dz = (targetPosition->z + .5) - z;

		xd = xd + (signum(dx) * .5f - xd) * .1f;
		yd = yd + (signum(dy) * .7f - yd) * .1f;
		zd = zd + (signum(dz) * .5f - zd) * .1f;

		float yRotD = static_cast<float>(atan2(zd, xd) * 180 / PI) - 90;
		float rotDiff = Mth::wrapDegrees(yRotD - yRot);
		yya = .5f;
		yRot += rotDiff;

		if (random->nextInt(100) == 0 && level->isSolidBlockingTile(Mth::floor(x), static_cast<int>(y) + 1, Mth::floor(z)))
		{
			setResting(true);
		}

	}
}

bool Bat::makeStepSound()
{
	return false;
}

void Bat::causeFallDamage(float distance)
{
}

void Bat::checkFallDamage(double ya, bool onGround)
{
	// this method is empty because flying creatures should
	// not trigger the "fallOn" tile calls (such as trampling crops)
}

bool Bat::isIgnoringTileTriggers()
{
	return true;
}

bool Bat::hurt(DamageSource *source, float dmg)
{
	if (isInvulnerable()) return false;
	if (!level->isClientSide)
	{
		if (isResting())
		{
			setResting(false);
		}
	}

	return AmbientCreature::hurt(source, dmg);
}

void Bat::readAdditionalSaveData(CompoundTag *tag)
{
	AmbientCreature::readAdditionalSaveData(tag);

	entityData->set(DATA_ID_FLAGS, tag->getByte(L"BatFlags"));
}

void Bat::addAdditonalSaveData(CompoundTag *entityTag)
{
	AmbientCreature::addAdditonalSaveData(entityTag);

	entityTag->putByte(L"BatFlags", entityData->getByte(DATA_ID_FLAGS));
}


bool Bat::canSpawn()
{
	int yt = Mth::floor(bb->y0);
	if (yt >= level->seaLevel) return false;

	int xt = Mth::floor(x);
	int zt = Mth::floor(z);

	int br = level->getRawBrightness(xt, yt, zt);
	int maxLight = 4;

	if ((Calendar::GetDayOfMonth() + 1 == 10 && Calendar::GetDayOfMonth() >= 20) || (Calendar::GetMonth() + 1 == 11 && Calendar::GetMonth() <= 3))
	{
		maxLight = 7;
	}
	else if (random->nextBoolean())
	{
		return false;
	}

	if (br > random->nextInt(maxLight)) return false;

	return AmbientCreature::canSpawn();
}