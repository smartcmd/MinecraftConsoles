#include "stdafx.h"

#include "SimpleFoiledItem.h"

SimpleFoiledItem::SimpleFoiledItem(int id) : Item(id)
{
}

bool SimpleFoiledItem::isFoil(shared_ptr<ItemInstance> itemInstance)
{
	return true;
}
