#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.ai.sensing.h"
#include "net.minecraft.world.phys.h"
#include "MeleeAttackGoal.h"

void MeleeAttackGoal::_init(Mob *mob, float speed, bool trackTarget)
{
	this->attackType = eTYPE_NOTSET;
	this->mob = mob;
	this->level = mob->level;
	this->speed = speed;
	this->trackTarget = trackTarget;
	setRequiredControlFlags(Control::MoveControlFlag | Control::LookControlFlag);


	attackTime = 0;
	path = NULL;
	timeToRecalcPath = 0;
}

MeleeAttackGoal::MeleeAttackGoal(Mob *mob, eINSTANCEOF attackType, float speed, bool trackTarget)
{
	_init(mob, speed, trackTarget);
	this->attackType = attackType;
}

MeleeAttackGoal::MeleeAttackGoal(Mob *mob, float speed, bool trackTarget)
{
	_init(mob,speed,trackTarget);
}

MeleeAttackGoal::~MeleeAttackGoal()
{
	if(path != NULL) delete path;
}

bool MeleeAttackGoal::canUse()
{
	shared_ptr<Mob> bestTarget = mob->getTarget();
	if (bestTarget == NULL) return false;
	if(!bestTarget->isAlive()) return false;
	if (attackType != eTYPE_NOTSET && (attackType & bestTarget->GetType()) != attackType) return false;
	target = weak_ptr<Mob>(bestTarget);
	delete path;
	path = mob->getNavigation()->createPath(target.lock());
	return path != NULL;
}

bool MeleeAttackGoal::canContinueToUse()
{
	shared_ptr<Mob> bestTarget = mob->getTarget();
	if (bestTarget == NULL) return false;
	if (target.lock() == NULL || !target.lock()->isAlive()) return false;
	if (!trackTarget) return !mob->getNavigation()->isDone();
	if (!mob->isWithinRestriction(Mth::floor(target.lock()->x), Mth::floor(target.lock()->y), Mth::floor(target.lock()->z))) return false;
	return true;
}

void MeleeAttackGoal::start()
{
	mob->getNavigation()->moveTo(path, speed);
	path = NULL;
	timeToRecalcPath = 0;
}

void MeleeAttackGoal::stop()
{
	target = weak_ptr<Mob>();
	mob->getNavigation()->stop();
}

void MeleeAttackGoal::tick()
{
	mob->getLookControl()->setLookAt(target.lock(), 30, 30);
	if (trackTarget || mob->getSensing()->canSee(target.lock()))
	{
		if (--timeToRecalcPath <= 0)
		{
			timeToRecalcPath = 4 + mob->getRandom()->nextInt(7);
			mob->getNavigation()->moveTo(target.lock(), speed);
		}
	}

	attackTime = max(attackTime - 1, 0);

	double meleeRadiusSqr = (mob->bbWidth * 2) * (mob->bbWidth * 2);
	if (mob->distanceToSqr(target.lock()->x, target.lock()->bb->y0, target.lock()->z) > meleeRadiusSqr) return;
	if (attackTime > 0) return;
	attackTime = 20;
	mob->doHurtTarget(target.lock());
}
