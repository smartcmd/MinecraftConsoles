#pragma once

#include "Item.h"

class Player;
class Level;

class BedItem : public Item
{

public:
	BedItem(int id);

	virtual bool useOn(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly=false);
};