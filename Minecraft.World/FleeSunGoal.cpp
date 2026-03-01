#include "stdafx.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.phys.h"
#include "FleeSunGoal.h"

FleeSunGoal::FleeSunGoal(PathfinderMob *mob, double speedModifier)
{
	this->mob = mob;
	this->speedModifier = speedModifier;
	this->level = mob->level;
	setRequiredControlFlags(Control::MoveControlFlag);
}

bool FleeSunGoal::canUse()
{
	if (!level->isDay()) return false;
	if (!mob->isOnFire()) return false;
	if (!level->canSeeSky(Mth::floor(mob->x), (int) mob->bb->y0, Mth::floor(mob->z))) return false;

	Vec3 *pos = getHidePos();
	if (pos == NULL) return false;
	wantedX = pos->x;
	wantedY = pos->y;
	wantedZ = pos->z;
	return true;
}

bool FleeSunGoal::canContinueToUse()
{
	return !mob->getNavigation()->isDone();
}

void FleeSunGoal::start()
{
	mob->getNavigation()->moveTo(wantedX, wantedY, wantedZ, speedModifier);
}

Vec3 *FleeSunGoal::getHidePos()
{
	Random *random = mob->getRandom();
	for (int i = 0; i < 10; i++)
	{
		int xt = Mth::floor(mob->x + random->nextInt(20) - 10);
		int yt = Mth::floor(mob->bb->y0 + random->nextInt(6) - 3);
		int zt = Mth::floor(mob->z + random->nextInt(20) - 10);
		if (!level->canSeeSky(xt, yt, zt) && mob->getWalkTargetValue(xt, yt, zt) < 0) return Vec3::newTemp(xt, yt, zt);
	}
	return NULL;
}