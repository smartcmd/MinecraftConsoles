#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.animal.h"
#include "OwnerHurtByTargetGoal.h"

OwnerHurtByTargetGoal::OwnerHurtByTargetGoal(TamableAnimal *tameAnimal) : TargetGoal(tameAnimal, 32, false)
{
	this->tameAnimal = tameAnimal;
	setRequiredControlFlags(TargetGoal::TargetFlag);
}

bool OwnerHurtByTargetGoal::canUse()
{
	if (!tameAnimal->isTame()) return false;
	shared_ptr<Mob> owner = tameAnimal->getOwner();
	if (owner == NULL) return false;
	ownerLastHurtBy = weak_ptr<Mob>(owner->getLastHurtByMob());
	return canAttack(ownerLastHurtBy.lock(), false);
}

void OwnerHurtByTargetGoal::start()
{
	mob->setTarget(ownerLastHurtBy.lock());
	TargetGoal::start();
}