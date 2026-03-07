#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.level.h"
#include "HurtByTargetGoal.h"

HurtByTargetGoal::HurtByTargetGoal(PathfinderMob *mob, bool alertSameType) : TargetGoal(mob, false)
{
	this->alertSameType = alertSameType;
	setRequiredControlFlags(TargetGoal::TargetFlag);
	timestamp = 0;
}

bool HurtByTargetGoal::canUse()
{
	int ts = mob->getLastHurtByMobTimestamp();
	return ts != timestamp && canAttack(mob->getLastHurtByMob(), false);
}

void HurtByTargetGoal::start()
{
	mob->setTarget(mob->getLastHurtByMob());
	timestamp = mob->getLastHurtByMobTimestamp();

	if (alertSameType)
	{
		double within = getFollowDistance();
		vector<shared_ptr<Entity> > *nearby = mob->level->getEntitiesOfClass(typeid(*mob), AABB::newTemp(mob->x, mob->y, mob->z, mob->x + 1, mob->y + 1, mob->z + 1)->grow(within, 4, within));
		for(auto& it : *nearby)
		{
			shared_ptr<PathfinderMob> other = dynamic_pointer_cast<PathfinderMob>(it);
			if (this->mob->shared_from_this() == other) continue;
			if (other->getTarget() != NULL) continue;
			if (other->isAlliedTo(mob->getLastHurtByMob())) continue; // don't target allies
			other->setTarget(mob->getLastHurtByMob());
		}
		delete nearby;
	}

	TargetGoal::start();
}