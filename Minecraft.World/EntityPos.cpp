#include "stdafx.h"
#include "Entity.h"
#include "EntityPos.h"

EntityPos::EntityPos(double x, double y, double z, float yRot, float xRot)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->yRot = yRot;
	this->xRot = xRot;
	rot = true;
	move = true;
}

EntityPos::EntityPos(double x, double y, double z)
{
	yRot = xRot = 0.0f;

	this->x = x;
	this->y = y;
	this->z = z;
	move = true;
	rot = false;
}

EntityPos::EntityPos(float yRot, float xRot)
{
	x = y = z = 0.0;

	this->yRot = yRot;
	this->xRot = xRot;
	rot = true;
	move = false;
}

EntityPos *EntityPos::lerp(shared_ptr<Entity> e, float f)
{
	double xd = e->x+(x-e->x)*f;
	double yd = e->y+(y-e->y)*f;
	double zd = e->z+(z-e->z)*f;

	float yrdd = Mth::wrapDegrees(yRot - e->yRot);
	float xrdd = Mth::wrapDegrees(xRot - e->xRot);

	float yrd = Mth::wrapDegrees(e->yRot + yrdd * f);
	float xrd = Mth::wrapDegrees(e->xRot + xrdd * f);

	if (rot && move)
	{
		return new EntityPos(xd, yd, zd, yrd, xrd);
	}
	if (move)
	{
		return new EntityPos(xd, yd, zd);
	}
	if (rot)
	{
		return new EntityPos(yrd, xrd);
	}
	return NULL;
}