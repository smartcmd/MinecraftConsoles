#include "stdafx.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.ai.util.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.phys.h"
#include "MoveTowardsTargetGoal.h"

MoveTowardsTargetGoal::MoveTowardsTargetGoal(PathfinderMob *mob, float speed, float within)
{
	this->mob = mob;
	this->speed = speed;
	this->within = within;
	setRequiredControlFlags(Control::MoveControlFlag);
}

bool MoveTowardsTargetGoal::canUse()
{
	target = weak_ptr<Mob>(mob->getTarget());
	if (target.lock() == NULL) return false;
	if (target.lock()->distanceToSqr(mob->shared_from_this()) > within * within) return false;
	Vec3 *pos = RandomPos::getPosTowards(dynamic_pointer_cast<PathfinderMob>(mob->shared_from_this()), 16, 7, Vec3::newTemp(target.lock()->x, target.lock()->y, target.lock()->z));
	if (pos == NULL) return false;
	wantedX = pos->x;
	wantedY = pos->y;
	wantedZ = pos->z;
	return true;
}

bool MoveTowardsTargetGoal::canContinueToUse()
{
	return target.lock() != NULL && !mob->getNavigation()->isDone() && target.lock()->isAlive() && target.lock()->distanceToSqr(mob->shared_from_this()) < within * within;
}

void MoveTowardsTargetGoal::stop()
{
	target = weak_ptr<Mob>();
}

void MoveTowardsTargetGoal::start()
{
	mob->getNavigation()->moveTo(wantedX, wantedY, wantedZ, speed);
}