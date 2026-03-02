#include "stdafx.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.ai.sensing.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.pathfinder.h"
#include "net.minecraft.world.phys.h"
#include "TargetGoal.h"

void TargetGoal::_init(Mob *mob, float within, bool mustSee, bool mustReach)
{
	reachCache = EmptyReachCache;
	reachCacheTime = 0;
	unseenTicks = 0;

	this->mob = mob;
	this->within = within;
	this->mustSee = mustSee;
	this->mustReach = mustReach;
}

TargetGoal::TargetGoal(Mob *mob, float within, bool mustSee)
{
	_init(mob, within, mustSee, false);
}

TargetGoal::TargetGoal(Mob *mob, float within, bool mustSee, bool mustReach)
{
	_init(mob,within,mustSee,mustReach);
}

bool TargetGoal::canContinueToUse()
{
	shared_ptr<Mob> target = mob->getTarget();
	if (target == NULL) return false;
	if (!target->isAlive()) return false;
	if (mob->distanceToSqr(target) > within * within) return false;
	if (mustSee)
	{
		if (mob->getSensing()->canSee(target))
		{
			unseenTicks = 0;
		}
		else
		{
			if (++unseenTicks > UnseenMemoryTicks) return false;
		}
	}
	return true;
}

void TargetGoal::start()
{
	reachCache = EmptyReachCache;
	reachCacheTime = 0;
	unseenTicks = 0;
}

void TargetGoal::stop()
{
	mob->setTarget(nullptr);
}

bool TargetGoal::canAttack(shared_ptr<Mob> target, bool allowInvulnerable)
{
	if (target == NULL) return false;
	if (target == mob->shared_from_this()) return false;
	if (!target->isAlive()) return false;
	if (!mob->canAttackType(target->GetType())) return false;

	shared_ptr<TamableAnimal> tamableAnimal = dynamic_pointer_cast<TamableAnimal>(mob->shared_from_this());
	if (tamableAnimal != NULL && tamableAnimal->isTame())
	{
		shared_ptr<TamableAnimal> tamableTarget = dynamic_pointer_cast<TamableAnimal>(target);
		if (tamableTarget != NULL && tamableTarget->isTame()) return false;
		if (target == tamableAnimal->getOwner()) return false;
	}
	else if (dynamic_pointer_cast<Player>(target) != NULL)
	{
		if (!allowInvulnerable && (dynamic_pointer_cast<Player>(target))->abilities.invulnerable) return false;
	}

	if (!mob->isWithinRestriction(Mth::floor(target->x), Mth::floor(target->y), Mth::floor(target->z))) return false;

	if (mustSee && !mob->getSensing()->canSee(target)) return false;

	if (mustReach)
	{
		if (--reachCacheTime <= 0) reachCache = EmptyReachCache;
		if (reachCache == EmptyReachCache) reachCache = canReach(target) ? CanReachCache : CantReachCache;
		if (reachCache == CantReachCache) return false;
	}

	return true;
}

bool TargetGoal::canReach(shared_ptr<Mob> target)
{
	reachCacheTime = 10 + mob->getRandom()->nextInt(5);
	Path *path = mob->getNavigation()->createPath(target);
	if (path == NULL) return false;
	Node *last = path->last();
	if (last == NULL)
	{
		delete path;
		return false;
	}
	int xx = last->x - Mth::floor(target->x);
	int zz = last->z - Mth::floor(target->z);
	delete path;
	return xx * xx + zz * zz <= 1.5 * 1.5;
}