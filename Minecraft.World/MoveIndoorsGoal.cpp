#include "stdafx.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.ai.util.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.ai.village.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.dimension.h"
#include "MoveIndoorsGoal.h"

MoveIndoorsGoal::MoveIndoorsGoal(PathfinderMob *mob)
{
	insideX = insideZ = -1;

	this->mob = mob;
	setRequiredControlFlags(Control::MoveControlFlag);
}

bool MoveIndoorsGoal::canUse()
{
	if ((mob->level->isDay() && !mob->level->isRaining()) || mob->level->dimension->hasCeiling) return false;
	if (mob->getRandom()->nextInt(50) != 0) return false;
	if (insideX != -1 && mob->distanceToSqr(insideX, mob->y, insideZ) < 2 * 2) return false;
	shared_ptr<Village> village = mob->level->villages->getClosestVillage(Mth::floor(mob->x), Mth::floor(mob->y), Mth::floor(mob->z), 14);
	if (village == NULL) return false;
	shared_ptr<DoorInfo> _doorInfo = village->getBestDoorInfo(Mth::floor(mob->x), Mth::floor(mob->y), Mth::floor(mob->z));
	doorInfo = _doorInfo;
	return _doorInfo != NULL;
}

bool MoveIndoorsGoal::canContinueToUse()
{
	return !mob->getNavigation()->isDone();
}

void MoveIndoorsGoal::start()
{
	insideX = -1;
	shared_ptr<DoorInfo> _doorInfo = doorInfo.lock();
	if( _doorInfo == NULL )
	{
		doorInfo = weak_ptr<DoorInfo>();
		return;
	}
	if (mob->distanceToSqr(_doorInfo->getIndoorX(), _doorInfo->y, _doorInfo->getIndoorZ()) > 16 * 16)
	{
		Vec3 *pos = RandomPos::getPosTowards(dynamic_pointer_cast<PathfinderMob>(mob->shared_from_this()), 14, 3, Vec3::newTemp(_doorInfo->getIndoorX() + 0.5, _doorInfo->getIndoorY(), _doorInfo->getIndoorZ() + 0.5));
		if (pos != NULL) mob->getNavigation()->moveTo(pos->x, pos->y, pos->z, 0.3f);
	}
	else mob->getNavigation()->moveTo(_doorInfo->getIndoorX() + 0.5, _doorInfo->getIndoorY(), _doorInfo->getIndoorZ() + 0.5, 0.3f);
}

void MoveIndoorsGoal::stop()
{
	shared_ptr<DoorInfo> _doorInfo = doorInfo.lock();
	if( _doorInfo == NULL )
	{
		doorInfo = weak_ptr<DoorInfo>();
		return;
	}

	insideX = _doorInfo->getIndoorX();
	insideZ = _doorInfo->getIndoorZ();
	doorInfo = weak_ptr<DoorInfo>();
}