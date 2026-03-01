#pragma once

#include "Item.h"

class NameTagItem : public Item
{
public:
	NameTagItem(int id);

	bool interactEnemy(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, shared_ptr<LivingEntity> target);
};