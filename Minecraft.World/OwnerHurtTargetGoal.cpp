#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.animal.h"
#include "OwnerHurtTargetGoal.h"

OwnerHurtTargetGoal::OwnerHurtTargetGoal(TamableAnimal *tameAnimal) : TargetGoal(tameAnimal, 32, false)
{
	this->tameAnimal = tameAnimal;
	setRequiredControlFlags(TargetGoal::TargetFlag);
	timestamp = 0;
}

bool OwnerHurtTargetGoal::canUse()
{
	if (!tameAnimal->isTame()) return false;
	shared_ptr<LivingEntity> owner = dynamic_pointer_cast<LivingEntity>( tameAnimal->getOwner() );
	if (owner == nullptr) return false;
	ownerLastHurt = weak_ptr<LivingEntity>(owner->getLastHurtMob());
	int ts = owner->getLastHurtMobTimestamp();
	shared_ptr<LivingEntity> locked = ownerLastHurt.lock();
	return ts != timestamp && canAttack(locked, false) && tameAnimal->wantsToAttack(locked, owner);
}

void OwnerHurtTargetGoal::start()
{
	mob->setTarget(ownerLastHurt.lock());

	shared_ptr<LivingEntity> owner = dynamic_pointer_cast<LivingEntity>( tameAnimal->getOwner() );
	if (owner != nullptr)
	{
		timestamp = owner->getLastHurtMobTimestamp();


		TargetGoal::start();
	}
}