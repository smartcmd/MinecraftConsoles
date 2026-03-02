#pragma once

#include "Item.h"

class CarrotOnAStickItem : public Item
{
public:
	CarrotOnAStickItem(int id);

	bool isHandEquipped();
	bool isMirroredArt();
	shared_ptr<ItemInstance> use(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player);
};