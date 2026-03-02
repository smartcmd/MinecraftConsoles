#pragma once

#include "Item.h"

class SimpleFoiledItem : public Item
{
public:
	SimpleFoiledItem(int id);

	bool isFoil(shared_ptr<ItemInstance> itemInstance);
};