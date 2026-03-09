#include "stdafx.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.h"
#include "SaddleItem.h"

SaddleItem::SaddleItem(int id) : Item(id)
{
	maxStackSize = 1;
}

bool SaddleItem::interactEnemy(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, shared_ptr<LivingEntity> mob) 
{
	if ( (mob != nullptr) && mob->instanceof(eTYPE_PIG) )
	{
		shared_ptr<Pig> pig = dynamic_pointer_cast<Pig>(mob);
		if (!pig->hasSaddle() && !pig->isBaby()) 
		{
			pig->setSaddle(true);
			itemInstance->count--;
		}
		return true;
	}
	return false;
}

bool SaddleItem::hurtEnemy(shared_ptr<ItemInstance> itemInstance, shared_ptr<LivingEntity> mob, shared_ptr<LivingEntity> attacker) 
{
	interactEnemy(itemInstance, nullptr, mob);
	return true;
}