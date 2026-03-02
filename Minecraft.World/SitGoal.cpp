#include "stdafx.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.animal.h"
#include "SitGoal.h"

SitGoal::SitGoal(TamableAnimal *mob)
{
	_wantToSit = false;

	this->mob = mob;
	setRequiredControlFlags(Control::JumpControlFlag | Control::MoveControlFlag);
}

bool SitGoal::canUse()
{
	if (!mob->isTame()) return false;
	if (mob->isInWater()) return false;
	if (!mob->onGround) return false;

	shared_ptr<LivingEntity> owner = dynamic_pointer_cast<LivingEntity>( mob->getOwner() );
	if (owner == NULL) return true; // owner not on level

	if (mob->distanceToSqr(owner) < FollowOwnerGoal::TeleportDistance * FollowOwnerGoal::TeleportDistance && owner->getLastHurtByMob() != NULL) return false;

	return _wantToSit;
}

void SitGoal::start()
{
	mob->getNavigation()->stop();
	mob->setSitting(true);
}

void SitGoal::stop()
{
	mob->setSitting(false);
}

void SitGoal::wantToSit(bool _wantToSit)
{
	this->_wantToSit = _wantToSit;
}