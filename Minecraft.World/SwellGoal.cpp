#include "stdafx.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.ai.sensing.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.monster.h"
#include "SwellGoal.h"

SwellGoal::SwellGoal(Creeper *creeper)
{
	target = weak_ptr<LivingEntity>();

	this->creeper = creeper;
	setRequiredControlFlags(Control::MoveControlFlag);
}

bool SwellGoal::canUse()
{
	shared_ptr<LivingEntity> target = creeper->getTarget();
	return creeper->getSwellDir() > 0 || (target != NULL && (creeper->distanceToSqr(target) < 3 * 3));
}

void SwellGoal::start()
{
	creeper->getNavigation()->stop();
	target = weak_ptr<LivingEntity>(creeper->getTarget());
}

void SwellGoal::stop()
{
	target = weak_ptr<LivingEntity>();
}

void SwellGoal::tick()
{
	if (target.lock() == NULL)
	{
		creeper->setSwellDir(-1);
		return;
	}

	if (creeper->distanceToSqr(target.lock()) > 7 * 7)
	{
		creeper->setSwellDir(-1);
		return;
	}

	if (!creeper->getSensing()->canSee(target.lock()))
	{
		creeper->setSwellDir(-1);
		return;
	}

	creeper->setSwellDir(1);
}
