#pragma once

#include "ColoredTileItem.h"

class WaterLilyTileItem : public ColoredTileItem
{
public:
	using ColoredTileItem::getColor;
	WaterLilyTileItem(int id);

	virtual shared_ptr<ItemInstance> use(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player);
	virtual bool TestUse(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player);
	virtual int getColor(int data, int spriteLayer);
};
