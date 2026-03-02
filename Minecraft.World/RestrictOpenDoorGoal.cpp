#include "stdafx.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.ai.village.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.level.h"
#include "RestrictOpenDoorGoal.h"

RestrictOpenDoorGoal::RestrictOpenDoorGoal(PathfinderMob *mob)
{
	this->mob = mob;
}

bool RestrictOpenDoorGoal::canUse()
{
	if (mob->level->isDay()) return false;
	shared_ptr<Village> village = mob->level->villages->getClosestVillage(Mth::floor(mob->x), Mth::floor(mob->y), Mth::floor(mob->z), 16);
	if (village == NULL) return false;
	shared_ptr<DoorInfo> _doorInfo = village->getClosestDoorInfo(Mth::floor(mob->x), Mth::floor(mob->y), Mth::floor(mob->z));
	if (_doorInfo == NULL) return false;
	doorInfo = _doorInfo;
	return _doorInfo->distanceToInsideSqr(Mth::floor(mob->x), Mth::floor(mob->y), Mth::floor(mob->z)) < 1.5 * 1.5;
}

bool RestrictOpenDoorGoal::canContinueToUse()
{
	if (mob->level->isDay()) return false;
	shared_ptr<DoorInfo> _doorInfo = doorInfo.lock();
	if ( _doorInfo == NULL ) return false;
	return !_doorInfo->removed && _doorInfo->isInsideSide(Mth::floor(mob->x), Mth::floor(mob->z));
}

void RestrictOpenDoorGoal::start()
{
	mob->getNavigation()->setCanOpenDoors(false);
	mob->getNavigation()->setCanPassDoors(false);
}

void RestrictOpenDoorGoal::stop()
{
	mob->getNavigation()->setCanOpenDoors(true);
	mob->getNavigation()->setCanPassDoors(true);
	doorInfo = weak_ptr<DoorInfo>();
}

void RestrictOpenDoorGoal::tick()
{
	shared_ptr<DoorInfo> _doorInfo = doorInfo.lock();
	if ( _doorInfo ) _doorInfo->incBookingCount();
}