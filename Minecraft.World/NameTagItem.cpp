#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "NameTagItem.h"

NameTagItem::NameTagItem(int id) : Item(id)
{
}

bool NameTagItem::interactEnemy(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, shared_ptr<LivingEntity> target)
{
	if (!itemInstance->hasCustomHoverName()) return false;

	if ( (target != NULL) && target->instanceof(eTYPE_MOB) )
	{
		shared_ptr<Mob> mob = dynamic_pointer_cast<Mob>(target);
		mob->setCustomName(itemInstance->getHoverName());
		mob->setPersistenceRequired();
		itemInstance->count--;
		return true;
	}

	return Item::interactEnemy(itemInstance, player, target);
}