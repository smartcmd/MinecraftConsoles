#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "ItemInstance.h"
#include "BowlFoodItem.h"

BowlFoodItem::BowlFoodItem(int id, int nutrition) : FoodItem( id, nutrition, false )
{
	setMaxStackSize(1);
}

shared_ptr<ItemInstance> BowlFoodItem::useTimeDepleted(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player)
{
	FoodItem::useTimeDepleted(instance, level, player);

	return std::make_shared<ItemInstance>(Item::bowl);
}