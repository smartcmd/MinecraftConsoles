#pragma once

#include "Item.h"

class EnderpearlItem : public Item
{
public:
	EnderpearlItem(int id);

	virtual shared_ptr<ItemInstance> use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player);
	// 4J added
	virtual bool TestUse(Level *level, shared_ptr<Player> player);
};