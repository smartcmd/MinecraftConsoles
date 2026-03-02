#pragma once

#include "Item.h"

class ItemInstance;

class ExperienceItem : public Item
{
public:
	ExperienceItem(int id);

	virtual bool isFoil(shared_ptr<ItemInstance> itemInstance);
	virtual shared_ptr<ItemInstance> use(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player);
	virtual bool TestUse(Level *level, shared_ptr<Player> player);
};