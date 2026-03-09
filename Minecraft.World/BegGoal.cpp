#include "stdafx.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "BegGoal.h"

BegGoal::BegGoal(Wolf *wolf, float lookDistance)
{
	player = weak_ptr<Player>();
	lookTime = 0;

	this->wolf = wolf;
	this->level = wolf->level;
	this->lookDistance = lookDistance;
	setRequiredControlFlags(Control::LookControlFlag);
}

bool BegGoal::canUse()
{
	player = weak_ptr<Player>(level->getNearestPlayer(wolf->shared_from_this(), lookDistance));
	if (player.lock() == nullptr) return false;
	wolf->setDespawnProtected();
	return playerHoldingInteresting(player.lock());
}

bool BegGoal::canContinueToUse()
{
	if (player.lock() == nullptr || !player.lock()->isAlive()) return false;
	if (wolf->distanceToSqr(player.lock()) > lookDistance * lookDistance) return false;
	wolf->setDespawnProtected();
	return lookTime > 0 && playerHoldingInteresting(player.lock());
}

void BegGoal::start()
{
	wolf->setIsInterested(true);
	lookTime = 40 + wolf->getRandom()->nextInt(40);
}

void BegGoal::stop()
{
	wolf->setIsInterested(false);
	player = weak_ptr<Player>();
}

void BegGoal::tick()
{
	wolf->getLookControl()->setLookAt(player.lock()->x, player.lock()->y + player.lock()->getHeadHeight(), player.lock()->z, 10, wolf->getMaxHeadXRot());
	--lookTime;
}

bool BegGoal::playerHoldingInteresting(shared_ptr<Player> player)
{
	shared_ptr<ItemInstance> item = player->inventory->getSelected();
	if (item == nullptr) return false;
	if (!wolf->isTame() && item->id == Item::bone_Id) return true;
	return wolf->isFood(item);
}