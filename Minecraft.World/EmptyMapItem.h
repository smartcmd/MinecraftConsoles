#pragma once

#include "ComplexItem.h"

class EmptyMapItem : public ComplexItem
{
public:
	EmptyMapItem(int id);

	shared_ptr<ItemInstance> use(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player);
};