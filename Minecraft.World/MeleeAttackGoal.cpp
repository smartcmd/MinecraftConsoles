#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.ai.sensing.h"
#include "net.minecraft.world.phys.h"
#include "MeleeAttackGoal.h"

void MeleeAttackGoal::_init(PathfinderMob *mob, double speedModifier, bool trackTarget)
{
	this->attackType = eTYPE_NOTSET;
	this->mob = mob;
	level = mob->level;
	this->speedModifier = speedModifier;
	this->trackTarget = trackTarget;
	setRequiredControlFlags(Control::MoveControlFlag | Control::LookControlFlag);


	attackTime = 0;
	path = NULL;
	timeToRecalcPath = 0;
}

MeleeAttackGoal::MeleeAttackGoal(PathfinderMob *mob, eINSTANCEOF attackType, double speedModifier, bool trackTarget)
{
	_init(mob, speedModifier, trackTarget);
	this->attackType = attackType;
}

MeleeAttackGoal::MeleeAttackGoal(PathfinderMob *mob, double speedModifier, bool trackTarget)
{
	_init(mob,speedModifier,trackTarget);
}

MeleeAttackGoal::~MeleeAttackGoal()
{
	if(path != NULL) delete path;
}

bool MeleeAttackGoal::canUse()
{
	shared_ptr<LivingEntity> target = mob->getTarget();
	if (target == NULL) return false;
	if (!target->isAlive()) return false;
	if (attackType != NULL && !target->instanceof(attackType)) return false;
	delete path;
	path = mob->getNavigation()->createPath(target);
	return path != NULL;
}

bool MeleeAttackGoal::canContinueToUse()
{
	shared_ptr<LivingEntity> target = mob->getTarget();
	if (target == NULL) return false;
	if (!target->isAlive()) return false;
	if (!trackTarget) return !mob->getNavigation()->isDone();
	if (!mob->isWithinRestriction(Mth::floor(target->x), Mth::floor(target->y), Mth::floor(target->z))) return false;
	return true;
}

void MeleeAttackGoal::start()
{
	mob->getNavigation()->moveTo(path, speedModifier);
	path = NULL;
	timeToRecalcPath = 0;
}

void MeleeAttackGoal::stop()
{
	mob->getNavigation()->stop();
}

void MeleeAttackGoal::tick()
{
	shared_ptr<LivingEntity> target = mob->getTarget();
	mob->getLookControl()->setLookAt(target, 30, 30);
	if (trackTarget || mob->getSensing()->canSee(target))
	{
		if (--timeToRecalcPath <= 0)
		{
			timeToRecalcPath = 4 + mob->getRandom()->nextInt(7);
			mob->getNavigation()->moveTo(target, speedModifier);
		}
	}

	attackTime = max(attackTime - 1, 0);

	double meleeRadiusSqr = (mob->bbWidth * 2) * (mob->bbWidth * 2) + target->bbWidth;
	if (mob->distanceToSqr(target->x, target->bb->y0, target->z) > meleeRadiusSqr) return;
	if (attackTime > 0) return;
	attackTime = 20;
	if (mob->getCarriedItem() != NULL) mob->swing();
	mob->doHurtTarget(target);
}
