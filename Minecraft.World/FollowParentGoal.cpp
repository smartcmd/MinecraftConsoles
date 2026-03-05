#include "stdafx.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.phys.h"
#include "BasicTypeContainers.h"
#include "FollowParentGoal.h"

FollowParentGoal::FollowParentGoal(Animal *animal, double speedModifier)
{
	timeToRecalcPath = 0;

	this->animal = animal;
	this->speedModifier = speedModifier;
}

bool FollowParentGoal::canUse()
{
	if (animal->getAge() >= 0) return false;

	vector<shared_ptr<Entity> > *parents = animal->level->getEntitiesOfClass(typeid(*animal), animal->bb->grow(8, 4, 8));

	shared_ptr<Animal> closest = nullptr;
	double closestDistSqr = Double::MAX_VALUE;
	for(auto& it : *parents)
	{
		shared_ptr<Animal> parent = dynamic_pointer_cast<Animal>(it);
		if (parent->getAge() < 0) continue;
		double distSqr = animal->distanceToSqr(parent);
		if (distSqr > closestDistSqr) continue;
		closestDistSqr = distSqr;
		closest = parent;
	}
	delete parents;

	if (closest == NULL) return false;
	if (closestDistSqr < 3 * 3) return false;
	parent = weak_ptr<Animal>(closest);
	return true;
}

bool FollowParentGoal::canContinueToUse()
{
	if (parent.lock() == NULL || !parent.lock()->isAlive()) return false;
	double distSqr = animal->distanceToSqr(parent.lock());
	if (distSqr < 3 * 3 || distSqr > 16 * 16) return false;
	return true;
}

void FollowParentGoal::start()
{
	timeToRecalcPath = 0;
}

void FollowParentGoal::stop()
{
	parent = weak_ptr<Animal>();
}

void FollowParentGoal::tick()
{
	if (--timeToRecalcPath > 0) return;
	timeToRecalcPath = 10;
	animal->getNavigation()->moveTo(parent.lock(), speedModifier);
}