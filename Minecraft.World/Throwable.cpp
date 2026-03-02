#include "stdafx.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.level.h"
#include "com.mojang.nbt.h"
#include "Throwable.h"



void Throwable::_throwableInit()
{
	xTile = -1;
	yTile = -1;
	zTile = -1;
	lastTile = 0;
	inGround = false;
	shakeTime = 0;
	owner = nullptr;
	life = 0;
	flightTime = 0;
}

Throwable::Throwable(Level *level) : Entity(level)
{
	_throwableInit();
	setSize(0.25f, 0.25f);
}

void Throwable::defineSynchedData()
{
}

bool Throwable::shouldRenderAtSqrDistance(double distance)
{
	double size = bb->getSize() * 4;
	size *= 64.0f;
	return distance < size * size;
}

Throwable::Throwable(Level *level, shared_ptr<Mob> mob) : Entity(level)
{
	_throwableInit();
	this->owner = mob;

	setSize(4 / 16.0f, 4 / 16.0f);

	this->moveTo(mob->x, mob->y + mob->getHeadHeight(), mob->z, mob->yRot, mob->xRot);


	x -= cos(yRot / 180 * PI) * 0.16f;
	y -= 0.1f;
	z -= sin(yRot / 180 * PI) * 0.16f;
	this->setPos(x, y, z);
	this->heightOffset = 0;


	float speed = 0.4f;
	xd = (-sin(yRot / 180 * PI) * cos(xRot / 180 * PI)) * speed;
	zd = (cos(yRot / 180 * PI) * cos(xRot / 180 * PI)) * speed;
	yd = (-sin((xRot + getThrowUpAngleOffset()) / 180 * PI)) * speed;

	shoot(xd, yd, zd, getThrowPower(), 1);
}

Throwable::Throwable(Level *level, double x, double y, double z) : Entity(level)
{
	_throwableInit();
	life = 0;

	setSize(4 / 16.0f, 4 / 16.0f);

	this->setPos(x, y, z);
	this->heightOffset = 0;
}


float Throwable::getThrowPower()
{
	return 1.5f;
}

float Throwable::getThrowUpAngleOffset()
{
	return 0;
}

void Throwable::shoot(double xd, double yd, double zd, float pow, float uncertainty)
{
	float dist = sqrt(xd * xd + yd * yd + zd * zd);

	xd /= dist;
	yd /= dist;
	zd /= dist;

	xd += (random->nextGaussian()) * 0.0075f * uncertainty;
	yd += (random->nextGaussian()) * 0.0075f * uncertainty;
	zd += (random->nextGaussian()) * 0.0075f * uncertainty;

	xd *= pow;
	yd *= pow;
	zd *= pow;

	this->xd = xd;
	this->yd = yd;
	this->zd = zd;

	float sd = (float) sqrt(xd * xd + zd * zd);

	yRotO = this->yRot = (float) (atan2(xd, zd) * 180 / PI);
	xRotO = this->xRot = (float) (atan2(yd, (double)sd) * 180 / PI);
	life = 0;
}

void Throwable::lerpMotion(double xd, double yd, double zd)
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

void Throwable::tick()
{
	xOld = x;
	yOld = y;
	zOld = z;
	Entity::tick();

	if (shakeTime > 0) shakeTime--;

	if (inGround)
	{
		int tile = level->getTile(xTile, yTile, zTile);
		if (tile == lastTile)
		{
			life++;
			if (life == 20 * 60) remove();
			return;
		}
		else
		{
			inGround = false;

			xd *= random->nextFloat() * 0.2f;
			yd *= random->nextFloat() * 0.2f;
			zd *= random->nextFloat() * 0.2f;
			life = 0;
			flightTime = 0;
		}
	}
	else
	{
		flightTime++;
	}

	Vec3 *from = Vec3::newTemp(x, y, z);
	Vec3 *to = Vec3::newTemp(x + xd, y + yd, z + zd);
	HitResult *res = level->clip(from, to);

	from = Vec3::newTemp(x, y, z);
	to = Vec3::newTemp(x + xd, y + yd, z + zd);
	if (res != NULL)
	{
		to = Vec3::newTemp(res->pos->x, res->pos->y, res->pos->z);
	}

	if (!level->isClientSide)
	{
		shared_ptr<Entity> hitEntity = nullptr;
		vector<shared_ptr<Entity> > *objects = level->getEntities(shared_from_this(), this->bb->expand(xd, yd, zd)->grow(1, 1, 1));
		double nearest = 0;
		for (int i = 0; i < objects->size(); i++)
		{
			shared_ptr<Entity> e = objects->at(i);
			if (!e->isPickable() || (e == owner && flightTime < 5)) continue;

			float rr = 0.3f;
			AABB *bb = e->bb->grow(rr, rr, rr);
			HitResult *p = bb->clip(from, to);
			if (p != NULL)
			{
				double dd = from->distanceTo(p->pos);
				delete p;
				if (dd < nearest || nearest == 0)
				{
					hitEntity = e;
					nearest = dd;
				}
			}
		}

		if (hitEntity != NULL)
		{
			if(res != NULL) delete res;
			res = new HitResult(hitEntity);
		}
	}

	if (res != NULL)
	{
		onHit(res);
		delete res;
	}
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


	float inertia = 0.99f;
	float gravity = getGravity();

	if (isInWater())
	{
		for (int i = 0; i < 4; i++)
		{
			float s = 1 / 4.0f;
			level->addParticle(eParticleType_bubble, x - xd * s, y - yd * s, z - zd * s, xd, yd, zd);
		}
		inertia = 0.80f;
	}

	xd *= inertia;
	yd *= inertia;
	zd *= inertia;
	yd -= gravity;

	setPos(x, y, z);
}

float Throwable::getGravity()
{
	return 0.03f;
}


void Throwable::addAdditonalSaveData(CompoundTag *tag)
{
	tag->putShort(L"xTile", (short) xTile);
	tag->putShort(L"yTile", (short) yTile);
	tag->putShort(L"zTile", (short) zTile);
	tag->putByte(L"inTile", (byte) lastTile);
	tag->putByte(L"shake", (byte) shakeTime);
	tag->putByte(L"inGround", (byte) (inGround ? 1 : 0));
}

void Throwable::readAdditionalSaveData(CompoundTag *tag)
{
	xTile = tag->getShort(L"xTile");
	yTile = tag->getShort(L"yTile");
	zTile = tag->getShort(L"zTile");
	lastTile = tag->getByte(L"inTile") & 0xff;
	shakeTime = tag->getByte(L"shake") & 0xff;
	inGround = tag->getByte(L"inGround") == 1;
}

float Throwable::getShadowHeightOffs()
{
	return 0;
}