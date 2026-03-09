#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.animal.h"
#include "OwnerHurtByTargetGoal.h"

OwnerHurtByTargetGoal::OwnerHurtByTargetGoal(TamableAnimal *tameAnimal) : TargetGoal(tameAnimal, 32, false)
{
	this->tameAnimal = tameAnimal;
	timestamp = 0;
	setRequiredControlFlags(TargetGoal::TargetFlag);
}

bool OwnerHurtByTargetGoal::canUse()
{
	if (!tameAnimal->isTame()) return false;
	shared_ptr<LivingEntity> owner = dynamic_pointer_cast<LivingEntity>( tameAnimal->getOwner() );
	if (owner == nullptr) return false;
	ownerLastHurtBy = weak_ptr<LivingEntity>(owner->getLastHurtByMob());
	int ts = owner->getLastHurtByMobTimestamp();

	shared_ptr<LivingEntity> locked = ownerLastHurtBy.lock();
	return ts != timestamp && canAttack(locked, false) && tameAnimal->wantsToAttack(locked, owner);
}

void OwnerHurtByTargetGoal::start()
{
	mob->setTarget(ownerLastHurtBy.lock());

	shared_ptr<LivingEntity> owner = dynamic_pointer_cast<LivingEntity>( tameAnimal->getOwner() );
	if (owner != nullptr)
	{
		timestamp = owner->getLastHurtByMobTimestamp();
	}

	TargetGoal::start();
}