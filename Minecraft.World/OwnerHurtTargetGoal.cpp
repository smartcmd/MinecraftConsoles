#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.animal.h"
#include "OwnerHurtTargetGoal.h"

OwnerHurtTargetGoal::OwnerHurtTargetGoal(TamableAnimal *tameAnimal) : TargetGoal(tameAnimal, 32, false)
{
	this->tameAnimal = tameAnimal;
	setRequiredControlFlags(TargetGoal::TargetFlag);
}

bool OwnerHurtTargetGoal::canUse()
{
	if (!tameAnimal->isTame()) return false;
	shared_ptr<Mob> owner = tameAnimal->getOwner();
	if (owner == NULL) return false;
	ownerLastHurt = weak_ptr<Mob>(owner->getLastHurtMob());
	return canAttack(ownerLastHurt.lock(), false);
}

void OwnerHurtTargetGoal::start()
{
	mob->setTarget(ownerLastHurt.lock());
	TargetGoal::start();
}