#pragma once

#include "Item.h"

class BookItem : public Item
{
public:
	BookItem(int id);

	bool isEnchantable(shared_ptr<ItemInstance> itemInstance);
	int getEnchantmentValue();
};