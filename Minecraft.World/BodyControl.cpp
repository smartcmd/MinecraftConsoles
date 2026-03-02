#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "BodyControl.h"

const float BodyControl::maxClampAngle = 75.0f;

BodyControl::BodyControl(LivingEntity *mob)
{
	this->mob = mob;

	timeStill = 0;
	lastHeadY = 0.0f;
}

void BodyControl::clientTick()
{
	double xd = mob->x - mob->xo;
	double zd = mob->z - mob->zo;

	if (xd * xd + zd * zd > MoveControl::MIN_SPEED_SQR)
	{
		// we are moving.
		mob->yBodyRot = mob->yRot;
		mob->yHeadRot = clamp(mob->yBodyRot, mob->yHeadRot, maxClampAngle);
		lastHeadY = mob->yHeadRot;
		timeStill = 0;
		return;
	}

	// Body will align to head after looking long enough in a direction
	float clampAngle = maxClampAngle;
	if (abs(mob->yHeadRot - lastHeadY) > 15)
	{
		timeStill = 0;
		lastHeadY = mob->yHeadRot;
	}
	else
	{
		++timeStill;
		static const int timeStillBeforeTurn = 10;
		if (timeStill > timeStillBeforeTurn) clampAngle = max(1 - (timeStill - timeStillBeforeTurn) / 10.f, 0.0f) * maxClampAngle;
	}

	mob->yBodyRot = clamp(mob->yHeadRot, mob->yBodyRot, clampAngle);
}

float BodyControl::clamp(float clampTo, float clampFrom, float clampAngle)
{
	float headDiffBody = Mth::wrapDegrees(clampTo - clampFrom);
	if (headDiffBody < -clampAngle) headDiffBody = -clampAngle;
	if (headDiffBody >= clampAngle) headDiffBody = +clampAngle;
	return clampTo - headDiffBody;
}