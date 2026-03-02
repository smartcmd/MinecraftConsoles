#include "stdafx.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.ai.sensing.h"
#include "net.minecraft.world.entity.ai.util.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.pathfinder.h"
#include "net.minecraft.world.phys.h"
#include "AvoidPlayerGoal.h"

AvoidPlayerGoal::AvoidPlayerGoal(PathfinderMob *mob, const type_info& avoidType, float maxDist, float walkSpeed, float sprintSpeed) : avoidType(avoidType)
{
	this->mob = mob;
	//this->avoidType = avoidType;
	this->maxDist = maxDist;
	this->walkSpeed = walkSpeed;
	this->sprintSpeed = sprintSpeed;
	this->pathNav = mob->getNavigation();
	setRequiredControlFlags(Control::MoveControlFlag);

	toAvoid = weak_ptr<Entity>();
	path = NULL;
}

AvoidPlayerGoal::~AvoidPlayerGoal()
{
	if(path != NULL) delete path;
}

bool AvoidPlayerGoal::canUse()
{
	if (avoidType == typeid(Player))
	{
		shared_ptr<TamableAnimal> tamableAnimal = dynamic_pointer_cast<TamableAnimal>(mob->shared_from_this());
		if (tamableAnimal != NULL && tamableAnimal->isTame()) return false;
		toAvoid = weak_ptr<Entity>(mob->level->getNearestPlayer(mob->shared_from_this(), maxDist));
		if (toAvoid.lock() == NULL) return false;
	}
	else
	{
		vector<shared_ptr<Entity> > *entities = mob->level->getEntitiesOfClass(avoidType, mob->bb->grow(maxDist, 3, maxDist));
		if (entities->empty())
		{
			delete entities;
			return false;
		}
		toAvoid = weak_ptr<Entity>(entities->at(0));
		delete entities;
	}

	if (!mob->getSensing()->canSee(toAvoid.lock())) return false;

	Vec3 *pos = RandomPos::getPosAvoid(dynamic_pointer_cast<PathfinderMob>(mob->shared_from_this()), 16, 7, Vec3::newTemp(toAvoid.lock()->x, toAvoid.lock()->y, toAvoid.lock()->z));
	if (pos == NULL) return false;
	if (toAvoid.lock()->distanceToSqr(pos->x, pos->y, pos->z) < toAvoid.lock()->distanceToSqr(mob->shared_from_this())) return false;
	delete path;
	path = pathNav->createPath(pos->x, pos->y, pos->z);
	if (path == NULL) return false;
	if (!path->endsInXZ(pos)) return false;
	return true;
}

bool AvoidPlayerGoal::canContinueToUse()
{
	return toAvoid.lock() != NULL && !pathNav->isDone();
}

void AvoidPlayerGoal::start()
{
	pathNav->moveTo(path, walkSpeed);
	path = NULL;
}

void AvoidPlayerGoal::stop()
{
	toAvoid = weak_ptr<Entity>();
}

void AvoidPlayerGoal::tick()
{
	if (mob->distanceToSqr(toAvoid.lock()) < 7 * 7) mob->getNavigation()->setSpeed(sprintSpeed);
	else mob->getNavigation()->setSpeed(walkSpeed);
}