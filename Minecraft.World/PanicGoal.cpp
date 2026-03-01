#include "stdafx.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.ai.util.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.phys.h"
#include "PanicGoal.h"

PanicGoal::PanicGoal(PathfinderMob *mob, double speedModifier)
{
	this->mob = mob;
	this->speedModifier = speedModifier;
	setRequiredControlFlags(Control::MoveControlFlag);
}

bool PanicGoal::canUse()
{
	if (mob->getLastHurtByMob() == NULL && !mob->isOnFire()) return false;
	Vec3 *pos = RandomPos::getPos(dynamic_pointer_cast<PathfinderMob>(mob->shared_from_this()), 5, 4);
	if (pos == NULL) return false;
	posX = pos->x;
	posY = pos->y;
	posZ = pos->z;
	return true;
}

void PanicGoal::start()
{
	mob->getNavigation()->moveTo(posX, posY, posZ, speedModifier);
}

bool PanicGoal::canContinueToUse()
{
	return !mob->getNavigation()->isDone();
}