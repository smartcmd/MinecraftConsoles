#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.phys.h"
#include "NearestAttackableTargetGoal.h"

NearestAttackableTargetGoal::DistComp::DistComp(Entity *source)
{
	this->source = source;
}

bool NearestAttackableTargetGoal::DistComp::operator() (shared_ptr<Entity> e1, shared_ptr<Entity> e2)
{
	// Should return true if e1 comes before e2 in the sorted list
	double distSqr1 = source->distanceToSqr(e1);
	double distSqr2 = source->distanceToSqr(e2);
	if (distSqr1 < distSqr2) return true;
	if (distSqr1 > distSqr2) return false;
	return true;
}

NearestAttackableTargetGoal::NearestAttackableTargetGoal(Mob *mob, const type_info& targetType, float within, int randomInterval, bool mustSee, bool mustReach /*= false*/) : TargetGoal(mob, within, mustSee, mustReach), targetType(targetType)
{
	//this->targetType = targetType;
	this->within = within;
	this->randomInterval = randomInterval;
	this->distComp = new DistComp(mob);
	setRequiredControlFlags(TargetGoal::TargetFlag);
}

NearestAttackableTargetGoal::~NearestAttackableTargetGoal()
{
	delete distComp;
}

bool NearestAttackableTargetGoal::canUse()
{
	if (randomInterval > 0 && mob->getRandom()->nextInt(randomInterval) != 0) return false;
	if (targetType == typeid(Player))
	{
		shared_ptr<Mob> potentialTarget = mob->level->getNearestAttackablePlayer(mob->shared_from_this(), within);
		if (canAttack(potentialTarget, false))
		{
			target = weak_ptr<Mob>(potentialTarget);
			return true;
		}
	}
	else
	{
		vector<shared_ptr<Entity> > *entities = mob->level->getEntitiesOfClass(targetType, mob->bb->grow(within, 4, within));
		//Collections.sort(entities, distComp);
		std::sort(entities->begin(), entities->end(), *distComp);
		for(AUTO_VAR(it, entities->begin()); it != entities->end(); ++it)
		{
			shared_ptr<Mob> potTarget = dynamic_pointer_cast<Mob>(*it);
			if (canAttack(potTarget, false))
			{
				target = weak_ptr<Mob>(potTarget);
				return true;
			}
		}
		delete entities;
	}
	return false;
}

void NearestAttackableTargetGoal::start()
{
	mob->setTarget(target.lock());
	TargetGoal::start();
}