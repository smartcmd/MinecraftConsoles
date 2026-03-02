#include "stdafx.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.phys.h"
#include "LookControl.h"

LookControl::LookControl(Mob *mob)
{
	yMax = xMax = 0.0f;
	hasWanted = false;
	wantedX = wantedY = wantedZ = 0.0;

	this->mob = mob;
}

void LookControl::setLookAt(shared_ptr<Entity> target, float yMax, float xMax)
{
	this->wantedX = target->x;
	shared_ptr<Mob> targetMob = dynamic_pointer_cast<Mob>(target);
	if (targetMob != NULL) this->wantedY = target->y + targetMob->getHeadHeight();
	else this->wantedY = (target->bb->y0 + target->bb->y1) / 2;
	this->wantedZ = target->z;
	this->yMax = yMax;
	this->xMax = xMax;
	hasWanted = true;
}

void LookControl::setLookAt(double x, double y, double z, float yMax, float xMax)
{
	this->wantedX = x;
	this->wantedY = y;
	this->wantedZ = z;
	this->yMax = yMax;
	this->xMax = xMax;
	hasWanted = true;
}

void LookControl::tick()
{
	mob->xRot = 0;

	if (hasWanted)
	{
		hasWanted = false;

		double xd = wantedX - mob->x;
		double yd = wantedY - (mob->y + mob->getHeadHeight());
		double zd = wantedZ - mob->z;
		double sd = sqrt(xd * xd + zd * zd);

		float yRotD = (float) (atan2(zd, xd) * 180 / PI) - 90;
		float xRotD = (float) -(atan2(yd, sd) * 180 / PI);
		mob->xRot = rotlerp(mob->xRot, xRotD, xMax);
		mob->yHeadRot = rotlerp(mob->yHeadRot, yRotD, yMax);
	}
	else
	{		
		mob->yHeadRot = rotlerp(mob->yHeadRot, mob->yBodyRot, 10);
	}

	float headDiffBody = Mth::wrapDegrees(mob->yHeadRot - mob->yBodyRot);

	if (!mob->getNavigation()->isDone())
	{
		// head clamped to body
		if (headDiffBody < -75) mob->yHeadRot = mob->yBodyRot - 75;
		if (headDiffBody > 75) mob->yHeadRot = mob->yBodyRot + 75;
	}
}

float LookControl::rotlerp(float a, float b, float max)
{
	float diff = b - a;
	while (diff < -180)
		diff += 360;
	while (diff >= 180)
		diff -= 360;
	if (diff > max)
	{
		diff = max;
	}
	if (diff < -max)
	{
		diff = -max;
	}
	return a + diff;
}

bool LookControl::isHasWanted()
{
	return hasWanted;
}

float LookControl::getYMax()
{
	return yMax;
}

float LookControl::getXMax()
{
	return xMax;
}

double LookControl::getWantedX()
{
	return wantedX;
}

double LookControl::getWantedY()
{
	return wantedY;
}

double LookControl::getWantedZ()
{
	return wantedZ;
}