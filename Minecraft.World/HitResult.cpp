#include "stdafx.h"

#include "net.minecraft.world.entity.h"
#include "HitResult.h"

HitResult::HitResult(int x, int y, int z, int f, Vec3 *pos)
{
	type = TILE;
	this->x = x;
	this->y = y;
	this->z = z;
	this->f = f;
	this->pos = Vec3::newTemp(pos->x, pos->y, pos->z);

	this->entity = nullptr;
}

HitResult::HitResult(shared_ptr<Entity> entity)
{
	type = ENTITY;
	this->entity = entity;
	pos = Vec3::newTemp(entity->x, entity->y, entity->z);

	x = y = z = f = 0;
}

double HitResult::distanceTo(shared_ptr<Entity> e)
{
	double xd = pos->x - e->x;
	double yd = pos->y - e->y;
	double zd = pos->z - e->z;
	return xd * xd + yd * yd + zd * zd;
}