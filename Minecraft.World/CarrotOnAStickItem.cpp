#include "stdafx.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.item.h"
#include "CarrotOnAStickItem.h"

CarrotOnAStickItem::CarrotOnAStickItem(int id) : Item(id)
{
	setMaxStackSize(1);
	setMaxDamage(25);
}

bool CarrotOnAStickItem::isHandEquipped()
{
	return true;
}

bool CarrotOnAStickItem::isMirroredArt()
{
	return true;
}

shared_ptr<ItemInstance> CarrotOnAStickItem::use(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
	if (player->isRiding())
	{
		shared_ptr<Pig> pig = dynamic_pointer_cast<Pig>(player->riding);
		if(pig)
		{
			if (pig->getControlGoal()->canBoost() && itemInstance->getMaxDamage() - itemInstance->getAuxValue() >= 7)
			{
				pig->getControlGoal()->boost();
				itemInstance->hurt(7, player);

				if (itemInstance->count == 0)
				{
					shared_ptr<ItemInstance> replacement = shared_ptr<ItemInstance>(new ItemInstance(Item::fishingRod));
					replacement->setTag(itemInstance->tag);
					return replacement;
				}
			}
		}
	}

	return itemInstance;
}