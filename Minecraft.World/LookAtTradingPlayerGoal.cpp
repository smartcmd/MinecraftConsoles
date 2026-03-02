#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.npc.h"
#include "LookAtTradingPlayerGoal.h"

LookAtTradingPlayerGoal::LookAtTradingPlayerGoal(Villager *villager) : LookAtPlayerGoal((Mob *)villager, typeid(Player), 8)
{
	this->villager = villager;
}

bool LookAtTradingPlayerGoal::canUse()
{
	if (villager->isTrading())
	{
		lookAt = weak_ptr<Entity>(dynamic_pointer_cast<Entity>(villager->getTradingPlayer()));
		return true;
	}
	return false;
}