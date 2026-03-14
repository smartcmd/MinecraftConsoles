#include "stdafx.h"
#include "net.minecraft.world.entity.ai.village.h"
#include "net.minecraft.world.entity.animal.h"
#include "DefendVillageTargetGoal.h"

DefendVillageTargetGoal::DefendVillageTargetGoal(VillagerGolem *golem) : TargetGoal(golem, false, true)
{
	this->golem = golem;
	setRequiredControlFlags(TargetGoal::TargetFlag);
}

bool DefendVillageTargetGoal::canUse()
{
	shared_ptr<Village> village = golem->getVillage();
	if (village == nullptr) return false;
	potentialTarget = weak_ptr<LivingEntity>(village->getClosestAggressor(dynamic_pointer_cast<LivingEntity>(golem->shared_from_this())));
	shared_ptr<LivingEntity> potTarget = potentialTarget.lock();
	if (!canAttack(potTarget, false))
	{
		// look for bad players
		if (mob->getRandom()->nextInt(20) == 0)
		{
			potentialTarget = village->getClosestBadStandingPlayer(dynamic_pointer_cast<LivingEntity>(golem->shared_from_this() ));
			return canAttack(potTarget, false);
		}
		return false;
	}
	else
	{
		return true;
	}
}

void DefendVillageTargetGoal::start()
{
	golem->setTarget(potentialTarget.lock());
	TargetGoal::start();
}