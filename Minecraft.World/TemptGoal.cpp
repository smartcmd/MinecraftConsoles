#include "stdafx.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.level.h"
#include "TemptGoal.h"

TemptGoal::TemptGoal(PathfinderMob *mob, double speedModifier, int itemId, bool canScare)
{
	px = py = pz = pRotX = pRotY = 0.0;
	player = weak_ptr<Player>();
	calmDown = 0;
	_isRunning = false;
	oldAvoidWater = false;

	this->mob = mob;
	this->speedModifier = speedModifier;
	this->itemId = itemId;
	this->canScare = canScare;
	setRequiredControlFlags(Control::MoveControlFlag | Control::LookControlFlag);
}

bool TemptGoal::canUse()
{
	if (calmDown > 0)
	{
		--calmDown;
		return false;
	}
	player = weak_ptr<Player>(mob->level->getNearestPlayer(mob->shared_from_this(), 10));
	if (player.lock() == nullptr) return false;
	mob->setDespawnProtected();		// If we've got a nearby player, then consider this mob as something we'd miss if it despawned
	shared_ptr<ItemInstance> item = player.lock()->getSelectedItem();
	if (item == nullptr) return false;
	if (item->id != itemId) return false;
	return true;
}

bool TemptGoal::canContinueToUse()
{
	if (canScare)
	{
		if(player.lock() == nullptr) return false;
		if (mob->distanceToSqr(player.lock()) < 6 * 6)
		{
			if (player.lock()->distanceToSqr(px, py, pz) > 0.1 * 0.1) return false;
			if (abs(player.lock()->xRot - pRotX) > 5 || abs(player.lock()->yRot - pRotY) > 5) return false;
		}
		else
		{
			px = player.lock()->x;
			py = player.lock()->y;
			pz = player.lock()->z;
		}
		pRotX = player.lock()->xRot;
		pRotY = player.lock()->yRot;
	}
	return canUse();
}

void TemptGoal::start()
{
	px = player.lock()->x;
	py = player.lock()->y;
	pz = player.lock()->z;
	_isRunning = true;
	oldAvoidWater = mob->getNavigation()->getAvoidWater();
	mob->getNavigation()->setAvoidWater(false);
}

void TemptGoal::stop()
{
	player = weak_ptr<Player>();
	mob->getNavigation()->stop();
	calmDown = 100;
	_isRunning = false;
	mob->getNavigation()->setAvoidWater(oldAvoidWater);
}

void TemptGoal::tick()
{
	mob->getLookControl()->setLookAt(player.lock(), 30, mob->getMaxHeadXRot());
	if (mob->distanceToSqr(player.lock()) < 2.5 * 2.5) mob->getNavigation()->stop();
	else mob->getNavigation()->moveTo(player.lock(), speedModifier);
}

bool TemptGoal::isRunning()
{
	return _isRunning;
}