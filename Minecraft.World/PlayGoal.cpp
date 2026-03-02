#include "stdafx.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.ai.util.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.npc.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.phys.h"
#include "BasicTypeContainers.h"
#include "PlayGoal.h"

PlayGoal::PlayGoal(Villager *mob, float speed)
{
	followFriend = weak_ptr<Mob>();
	wantedX = wantedY = wantedZ = 0.0;
	playTime = 0;

	this->mob = mob;
	this->speed = speed;
	setRequiredControlFlags(Control::MoveControlFlag);
}

bool PlayGoal::canUse()
{
	if (mob->getAge() >= 0) return false;
	if (mob->getRandom()->nextInt(400) != 0) return false;

	vector<shared_ptr<Entity> > *children = mob->level->getEntitiesOfClass(typeid(Villager), mob->bb->grow(6, 3, 6));
	double closestDistSqr = Double::MAX_VALUE;
	//for (Entity c : children)
	for(AUTO_VAR(it, children->begin()); it != children->end(); ++it)
	{
		shared_ptr<Entity> c = *it;
		if (c.get() == mob) continue;
		shared_ptr<Villager> friendV = dynamic_pointer_cast<Villager>(c);
		if (friendV->isChasing()) continue;
		if (friendV->getAge() >= 0) continue;
		double distSqr = friendV->distanceToSqr(mob->shared_from_this());
		if (distSqr > closestDistSqr) continue;
		closestDistSqr = distSqr;
		followFriend = weak_ptr<Mob>(friendV);
	}
	delete children;

	if (followFriend.lock() == NULL)
	{
		Vec3 *pos = RandomPos::getPos(dynamic_pointer_cast<PathfinderMob>(mob->shared_from_this()), 16, 3);
		if (pos == NULL) return false;
	}
	return true;
}

bool PlayGoal::canContinueToUse()
{
	return playTime > 0 && followFriend.lock() != NULL;
}

void PlayGoal::start()
{
	if (followFriend.lock() != NULL) mob->setChasing(true);
	playTime = 1000;
}

void PlayGoal::stop()
{
	mob->setChasing(false);
	followFriend = weak_ptr<Mob>();
}

void PlayGoal::tick()
{
	--playTime;
	if (followFriend.lock() != NULL)
	{
		if (mob->distanceToSqr(followFriend.lock()) > 2 * 2) mob->getNavigation()->moveTo(followFriend.lock(), speed);
	}
	else
	{
		if (mob->getNavigation()->isDone())
		{
			Vec3 *pos = RandomPos::getPos(dynamic_pointer_cast<PathfinderMob>(mob->shared_from_this()), 16, 3);
			if (pos == NULL) return;
			mob->getNavigation()->moveTo(pos->x, pos->y, pos->z, speed);
		}
	}
}