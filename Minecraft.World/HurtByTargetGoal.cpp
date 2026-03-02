#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.level.h"
#include "HurtByTargetGoal.h"

HurtByTargetGoal::HurtByTargetGoal(Mob *mob, bool alertSameType) : TargetGoal(mob, 16, false)
{
	this->alertSameType = alertSameType;
	setRequiredControlFlags(TargetGoal::TargetFlag);
}

bool HurtByTargetGoal::canUse()
{
	return canAttack(mob->getLastHurtByMob(), false);
}

void HurtByTargetGoal::start()
{
	mob->setTarget(mob->getLastHurtByMob());
	oldHurtByMob = mob->getLastHurtByMob();

	if (alertSameType)
	{
		vector<shared_ptr<Entity> > *nearby = mob->level->getEntitiesOfClass(typeid(*mob), AABB::newTemp(mob->x, mob->y, mob->z, mob->x + 1, mob->y + 1, mob->z + 1)->grow(within, 4, within));
		for(AUTO_VAR(it, nearby->begin()); it != nearby->end(); ++it)
		{
			shared_ptr<Mob> other = dynamic_pointer_cast<Mob>(*it);
			if (this->mob->shared_from_this() == other) continue;
			if (other->getTarget() != NULL) continue;
			other->setTarget(mob->getLastHurtByMob());
		}
		delete nearby;
	}

	TargetGoal::start();
}

void HurtByTargetGoal::tick()
{
	if (mob->getLastHurtByMob() != NULL && mob->getLastHurtByMob() != oldHurtByMob)
	{
		this->start();
	}
}
