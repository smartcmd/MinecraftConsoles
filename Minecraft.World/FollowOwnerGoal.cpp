#include "stdafx.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.phys.h"
#include "FollowOwnerGoal.h"

FollowOwnerGoal::FollowOwnerGoal(TamableAnimal *tamable, double speedModifier, float startDistance, float stopDistance)
{
	owner = weak_ptr<Mob>();
	timeToRecalcPath = 0;
	oldAvoidWater = false;

	this->tamable = tamable;
	level = tamable->level;
	this->speedModifier = speedModifier;
	navigation = tamable->getNavigation();
	this->startDistance = startDistance;
	this->stopDistance = stopDistance;
	setRequiredControlFlags(Control::MoveControlFlag | Control::LookControlFlag);
}

bool FollowOwnerGoal::canUse()
{
	shared_ptr<LivingEntity> owner = dynamic_pointer_cast<LivingEntity>( tamable->getOwner() );
	if (owner == nullptr) return false;
	if (tamable->isSitting()) return false;
	if (tamable->distanceToSqr(owner) < startDistance * startDistance) return false;
	this->owner = weak_ptr<LivingEntity>(owner);
	return true;
}

bool FollowOwnerGoal::canContinueToUse()
{
	return owner.lock() != nullptr && !navigation->isDone() && tamable->distanceToSqr(owner.lock()) > stopDistance * stopDistance && !tamable->isSitting();
}

void FollowOwnerGoal::start()
{
	timeToRecalcPath = 0;
	oldAvoidWater = tamable->getNavigation()->getAvoidWater();
	tamable->getNavigation()->setAvoidWater(false);
}

void FollowOwnerGoal::stop()
{
	owner = weak_ptr<Mob>();
	navigation->stop();
	tamable->getNavigation()->setAvoidWater(oldAvoidWater);
}

void FollowOwnerGoal::tick()
{
	tamable->getLookControl()->setLookAt(owner.lock(), 10, tamable->getMaxHeadXRot());
	if (tamable->isSitting()) return;

	if (--timeToRecalcPath > 0) return;
	timeToRecalcPath = 10;

	if (navigation->moveTo(owner.lock(), speedModifier)) return;
	if (tamable->isLeashed()) return;
	if (tamable->distanceToSqr(owner.lock()) < TeleportDistance * TeleportDistance) return;

	// find a good spawn position nearby the owner
	int sx = Mth::floor(owner.lock()->x) - 2;
	int sz = Mth::floor(owner.lock()->z) - 2;
	int y = Mth::floor(owner.lock()->bb->y0);
	for (int x = 0; x <= 4; x++)
	{
		for (int z = 0; z <= 4; z++)
		{
			if (x >= 1 && z >= 1 && x <= 3 && z <= 3)
			{
				continue;
			}
			if (level->isTopSolidBlocking(sx + x, y - 1, sz + z) && !level->isSolidBlockingTile(sx + x, y, sz + z) && !level->isSolidBlockingTile(sx + x, y + 1, sz + z))
			{
				tamable->moveTo(sx + x + .5f, y, sz + z + .5f, tamable->yRot, tamable->xRot);
				navigation->stop();
				return;
			}
		}
	}
}