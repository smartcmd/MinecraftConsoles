#include "stdafx.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.item.h"
#include "SharedConstants.h"
#include "JavaMath.h"
#include "EyeOfEnderSignal.h"



void EyeOfEnderSignal::_init()
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();

	// Initialisors
	shakeTime = 0;
	tx = ty = tz = 0.0;
	life = 0;
	surviveAfterDeath = false;
}


EyeOfEnderSignal::EyeOfEnderSignal(Level *level) : Entity(level)
{
	_init();
	setSize(0.25f, 0.25f);
}

void EyeOfEnderSignal::defineSynchedData()
{
}


bool EyeOfEnderSignal::shouldRenderAtSqrDistance(double distance)
{
	double size = bb->getSize() * 4;
	size *= 64.0f;
	return distance < size * size;
}

EyeOfEnderSignal::EyeOfEnderSignal(Level *level, double x, double y, double z) : Entity(level)
{
	_init();
	life = 0;

	setSize(0.25f, 0.25f);

	this->setPos(x, y, z);
	this->heightOffset = 0;
}

void EyeOfEnderSignal::signalTo(double tx, int ty, double tz)
{


	double dx = tx - x, dz = tz - z;
	float dist = sqrt(dx * dx + dz * dz);

	if (dist > 12)
	{
		this->tx = x + (dx / dist) * 12;
		this->tz = z + (dz / dist) * 12;
		this->ty = y + 8;
	}
	else
	{
		this->tx = tx;
		this->ty = ty;
		this->tz = tz;
	}

	life = 0;
	surviveAfterDeath = random->nextInt(5) > 0;
}

void EyeOfEnderSignal::lerpMotion(double xd, double yd, double zd)
{
	this->xd = xd;
	this->yd = yd;
	this->zd = zd;
	if (xRotO == 0 && yRotO == 0)
	{
		float sd = (float) sqrt(xd * xd + zd * zd);
		yRotO = this->yRot = (float) (atan2(xd, zd) * 180 / PI);
		xRotO = this->xRot = (float) (atan2(yd, (double)sd) * 180 / PI);
	}
}

void EyeOfEnderSignal::tick()
{
	xOld = x;
	yOld = y;
	zOld = z;
	Entity::tick();

	x += xd;
	y += yd;
	z += zd;

	float sd = (float) sqrt(xd * xd + zd * zd);
	yRot = (float) (atan2(xd, zd) * 180 / PI);
	xRot = (float) (atan2(yd, (double)sd) * 180 / PI);

	while (xRot - xRotO < -180)
		xRotO -= 360;
	while (xRot - xRotO >= 180)
		xRotO += 360;

	while (yRot - yRotO < -180)
		yRotO -= 360;
	while (yRot - yRotO >= 180)
		yRotO += 360;

	xRot = xRotO + (xRot - xRotO) * 0.2f;
	yRot = yRotO + (yRot - yRotO) * 0.2f;

	if (!level->isClientSide)
	{
		double dx = tx - x, dz = tz - z;
		float tdist = (float) sqrt(dx * dx + dz * dz);
		float angle = (float) atan2(dz, dx);
		double tspeed = (sd + (tdist - sd) * .0025);
		if (tdist < 1)
		{
			tspeed *= .8;
			yd *= .8;
		}
		xd = cos(angle) * tspeed;
		zd = sin(angle) * tspeed;

		if (y < ty)
		{
			yd = yd + (1 - yd) * .015f;
		}
		else
		{
			yd = yd + (-1 - yd) * .015f;
		}

	}

	float s = 1 / 4.0f;
	if (isInWater())
	{
		for (int i = 0; i < 4; i++)
		{
			level->addParticle(eParticleType_bubble, x - xd * s, y - yd * s, z - zd * s, xd, yd, zd);
		}
	}
	else
	{
		level->addParticle(eParticleType_ender, x - xd * s + random->nextDouble() * .6 - .3, y - yd * s - .5, z - zd * s + random->nextDouble() * .6 - .3, xd, yd, zd);
	}

	if (!level->isClientSide)
	{
		setPos(x, y, z);


		life++;
		if (life > SharedConstants::TICKS_PER_SECOND * 4 && !level->isClientSide)
		{
			remove();
			if (surviveAfterDeath)
			{
				level->addEntity(shared_ptr<ItemEntity>( new ItemEntity(level, x, y, z, shared_ptr<ItemInstance>(new ItemInstance(Item::eyeOfEnder)))));
			}
			else
			{
				level->levelEvent(LevelEvent::PARTICLES_EYE_OF_ENDER_DEATH, (int) Math::round(x), (int) Math::round(y), (int) Math::round(z), 0);
			}
		}
	}
}

void EyeOfEnderSignal::addAdditonalSaveData(CompoundTag *tag)
{
}

void EyeOfEnderSignal::readAdditionalSaveData(CompoundTag *tag)
{
}

float EyeOfEnderSignal::getShadowHeightOffs()
{
	return 0;
}

float EyeOfEnderSignal::getBrightness(float a)
{
	return 1.0f;
}

int EyeOfEnderSignal::getLightColor(float a)
{
	return 15 << 20 | 15 << 4;
}

bool EyeOfEnderSignal::isAttackable()
{
	return false;
}