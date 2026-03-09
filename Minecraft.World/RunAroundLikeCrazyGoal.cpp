#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "RandomPos.h"
#include "RunAroundLikeCrazyGoal.h"

RunAroundLikeCrazyGoal::RunAroundLikeCrazyGoal(EntityHorse *mob, double speedModifier)
{
	horse = mob;
	this->speedModifier = speedModifier;
	setRequiredControlFlags(Control::MoveControlFlag);
}

bool RunAroundLikeCrazyGoal::canUse()
{
	if (horse->isTamed() || horse->rider.lock() == nullptr) return false;
	Vec3 *pos = RandomPos::getPos(dynamic_pointer_cast<PathfinderMob>(horse->shared_from_this()), 5, 4);
	if (pos == nullptr) return false;
	posX = pos->x;
	posY = pos->y;
	posZ = pos->z;
	return true;
}

void RunAroundLikeCrazyGoal::start()
{
	horse->getNavigation()->moveTo(posX, posY, posZ, speedModifier);
}

bool RunAroundLikeCrazyGoal::canContinueToUse()
{
	return !horse->getNavigation()->isDone() && horse->rider.lock() != nullptr;
}

void RunAroundLikeCrazyGoal::tick()
{
	if (horse->getRandom()->nextInt(50) == 0)
	{

		if ( horse->rider.lock()->instanceof(eTYPE_PLAYER) )
		{
			int temper = horse->getTemper();
			int maxTemper = horse->getMaxTemper();
			if (maxTemper > 0 && horse->getRandom()->nextInt(maxTemper) < temper)
			{
				horse->tameWithName(dynamic_pointer_cast<Player>(horse->rider.lock()));
				horse->level->broadcastEntityEvent(horse->shared_from_this(), EntityEvent::TAMING_SUCCEEDED);
				return;
			}
			horse->modifyTemper(5);
		}

		horse->rider.lock()->ride(nullptr);
		horse->rider = weak_ptr<LivingEntity>();
		horse->makeMad();
		horse->level->broadcastEntityEvent(horse->shared_from_this(), EntityEvent::TAMING_FAILED);
	}
}