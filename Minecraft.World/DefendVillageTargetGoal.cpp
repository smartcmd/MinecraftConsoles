#include "stdafx.h"
#include "net.minecraft.world.entity.ai.village.h"
#include "net.minecraft.world.entity.animal.h"
#include "DefendVillageTargetGoal.h"

DefendVillageTargetGoal::DefendVillageTargetGoal(VillagerGolem *golem) : TargetGoal(golem, 16, false, true)
{
	this->golem = golem;
	setRequiredControlFlags(TargetGoal::TargetFlag);
}

bool DefendVillageTargetGoal::canUse()
{
	shared_ptr<Village> village = golem->getVillage();
	if (village == NULL) return false;
	potentialTarget = weak_ptr<Mob>(village->getClosestAggressor(dynamic_pointer_cast<Mob>(golem->shared_from_this())));
	return canAttack(potentialTarget.lock(), false);
}

void DefendVillageTargetGoal::start()
{
	golem->setTarget(potentialTarget.lock());
	TargetGoal::start();
}