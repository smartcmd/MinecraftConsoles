#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.phys.h"
#include "NearestAttackableTargetGoal.h"

SubselectEntitySelector::SubselectEntitySelector(NearestAttackableTargetGoal *parent, EntitySelector *subselector)
{
	m_parent = parent;
	m_subselector = subselector;
}

SubselectEntitySelector::~SubselectEntitySelector()
{
	delete m_subselector;
}

bool SubselectEntitySelector::matches(shared_ptr<Entity> entity) const
{
	if (!entity->instanceof(eTYPE_LIVINGENTITY)) return false;
	if (m_subselector != nullptr && !m_subselector->matches(entity)) return false;
	return m_parent->canAttack(dynamic_pointer_cast<LivingEntity>(entity), false);
}

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
	return false;
}

NearestAttackableTargetGoal::NearestAttackableTargetGoal(PathfinderMob *mob, const type_info& targetType, int randomInterval, bool mustSee, bool mustReach /*= false*/, EntitySelector *entitySelector /* =nullptr */)
	: TargetGoal(mob, mustSee, mustReach), targetType(targetType)
{
	this->randomInterval = randomInterval;
	this->distComp = new DistComp(mob);
	setRequiredControlFlags(TargetGoal::TargetFlag);

	this->selector = new SubselectEntitySelector(this, entitySelector);
}

NearestAttackableTargetGoal::~NearestAttackableTargetGoal()
{
	delete distComp;
	delete selector;
}

bool NearestAttackableTargetGoal::canUse()
{
	if (randomInterval > 0 && mob->getRandom()->nextInt(randomInterval) != 0) return false;
	double within = getFollowDistance();

	vector<shared_ptr<Entity> > *entities = mob->level->getEntitiesOfClass(targetType, mob->bb->grow(within, 4, within), selector);

	bool result = false;
	if(entities != nullptr && !entities->empty() )
	{
		std::sort(entities->begin(), entities->end(), *distComp);
		target = weak_ptr<LivingEntity>(dynamic_pointer_cast<LivingEntity>(entities->at(0)));
		result = true;
	}

	delete entities;
	return result;
}

void NearestAttackableTargetGoal::start()
{
	mob->setTarget(target.lock());
	TargetGoal::start();
}