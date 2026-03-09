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
    Goal::setRequiredControlFlags(Control::MoveControlFlag);
}

bool PanicGoal::canUse()
{
	if (mob->getLastHurtByMob() == nullptr && !mob->isOnFire()) return false;
	const int hurtTimeout = mob->getLastHurtByMobTimestamp();
	if (hurtTimeout == 0) return false;
    static thread_local std::mt19937 rng(std::random_device{}());
	std::uniform_int_distribution<int> dist(60, 100);
	const int panicDuration = dist(rng);
	if (mob->tickCount - hurtTimeout > panicDuration) return false;
	const Vec3* pos = RandomPos::getPos(dynamic_pointer_cast<PathfinderMob>(mob->shared_from_this()), 5, 4);
	if (pos == nullptr) return false;
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
	if (mob->getLastHurtByMob() == nullptr && !mob->isOnFire()) return false;
	return !mob->getNavigation()->isDone();
}