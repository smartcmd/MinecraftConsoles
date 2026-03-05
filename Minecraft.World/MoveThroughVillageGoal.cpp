#include "stdafx.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.ai.util.h"
#include "net.minecraft.world.entity.ai.village.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.level.h"
#include "BasicTypeContainers.h"
#include "MoveThroughVillageGoal.h"
#include "Path.h"

MoveThroughVillageGoal::MoveThroughVillageGoal(PathfinderMob *mob, double speedModifier, bool onlyAtNight)
{
	path = NULL;
	doorInfo = weak_ptr<DoorInfo>();

	this->mob = mob;
	this->speedModifier = speedModifier;
	this->onlyAtNight = onlyAtNight;
	setRequiredControlFlags(Control::MoveControlFlag);
}

MoveThroughVillageGoal::~MoveThroughVillageGoal()
{
	if(path != NULL) delete path;
}

bool MoveThroughVillageGoal::canUse()
{
	updateVisited();

	if (onlyAtNight && mob->level->isDay()) return false;

	shared_ptr<Village> village = mob->level->villages->getClosestVillage(Mth::floor(mob->x), Mth::floor(mob->y), Mth::floor(mob->z), 0);
	if (village == NULL) return false;

	shared_ptr<DoorInfo> _doorInfo = getNextDoorInfo(village);
	if (_doorInfo == NULL) return false;
	doorInfo = _doorInfo;

	bool oldCanOpenDoors = mob->getNavigation()->canOpenDoors();
	mob->getNavigation()->setCanOpenDoors(false);
	delete path;

	path = mob->getNavigation()->createPath(_doorInfo->x, _doorInfo->y, _doorInfo->z);
	mob->getNavigation()->setCanOpenDoors(oldCanOpenDoors);
	if (path != NULL) return true;

	Vec3 *pos = RandomPos::getPosTowards(dynamic_pointer_cast<PathfinderMob>(mob->shared_from_this()), 10, 7, Vec3::newTemp(_doorInfo->x, _doorInfo->y, _doorInfo->z));
	if (pos == NULL) return false;
	mob->getNavigation()->setCanOpenDoors(false);
	delete path;
	path = mob->getNavigation()->createPath(pos->x, pos->y, pos->z);
	mob->getNavigation()->setCanOpenDoors(oldCanOpenDoors);
	return path != NULL;
}

bool MoveThroughVillageGoal::canContinueToUse()
{
	if (mob->getNavigation()->isDone()) return false;
	float dist = mob->bbWidth + 4.f;
	shared_ptr<DoorInfo> _doorInfo = doorInfo.lock();
	if( _doorInfo == NULL ) return false;

	return mob->distanceToSqr(_doorInfo->x, _doorInfo->y, _doorInfo->z) > dist * dist;
}

void MoveThroughVillageGoal::start()
{
	mob->getNavigation()->moveTo(path, speedModifier);
	path = NULL;
}

void MoveThroughVillageGoal::stop()
{
	shared_ptr<DoorInfo> _doorInfo = doorInfo.lock();
	if( _doorInfo == NULL ) return;

	if (mob->getNavigation()->isDone() || mob->distanceToSqr(_doorInfo->x, _doorInfo->y, _doorInfo->z) < 4 * 4)
	{
		visited.push_back(doorInfo);
	}
}

shared_ptr<DoorInfo> MoveThroughVillageGoal::getNextDoorInfo(shared_ptr<Village> village)
{
	shared_ptr<DoorInfo> closest = nullptr;
	int closestDistSqr = Integer::MAX_VALUE;
	vector<shared_ptr<DoorInfo> > *doorInfos = village->getDoorInfos();
	for(auto& di : *doorInfos)
	{
		int distSqr = di->distanceToSqr(Mth::floor(mob->x), Mth::floor(mob->y), Mth::floor(mob->z));
		if (distSqr < closestDistSqr)
		{
			if (hasVisited(di)) continue;
			closest = di;
			closestDistSqr = distSqr;
		}
	}
	return closest;
}

bool MoveThroughVillageGoal::hasVisited(shared_ptr<DoorInfo>di)
{
    for (auto it = visited.begin(); it != visited.end();)
    {
		shared_ptr<DoorInfo> di2 = (*it).lock();
		if( di2 )
		{
			it = visited.erase(it);
		}
		else
		{
			if (di->x == di2->x && di->y == di2->y && di->z == di2->z) return true;
			++it;
		}
	}
	return false;
}

void MoveThroughVillageGoal::updateVisited()
{
	if (visited.size() > 15) visited.erase(visited.begin());
}