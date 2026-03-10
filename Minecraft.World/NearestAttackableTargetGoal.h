#pragma once

#include "TargetGoal.h"
#include "EntitySelector.h"

class NearestAttackableTargetGoal;

// Anonymous class from NearestAttackableTargetGoal
class SubselectEntitySelector : public EntitySelector
{
private:
	EntitySelector *m_subselector;
	NearestAttackableTargetGoal *m_parent;

public:
	SubselectEntitySelector(NearestAttackableTargetGoal *parent, EntitySelector *subselector);
	~SubselectEntitySelector();
	bool matches(shared_ptr<Entity> entity) const;
};

class NearestAttackableTargetGoal : public TargetGoal
{
	friend class SubselectEntitySelector;
public:
	class DistComp
	{
	private:
		Entity *source;

	public:
		DistComp(Entity *source);

		bool operator() (shared_ptr<Entity> e1, shared_ptr<Entity> e2);
	};

private:
	const type_info& targetType;
	int randomInterval;
	DistComp *distComp;
	EntitySelector *selector;
	weak_ptr<LivingEntity> target;

public:
	NearestAttackableTargetGoal(PathfinderMob *mob, const type_info& targetType, int randomInterval, bool mustSee, bool mustReach = false, EntitySelector *entitySelector = nullptr);

	virtual ~NearestAttackableTargetGoal();

	virtual bool canUse();
	void start();
};