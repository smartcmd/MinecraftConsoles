#pragma once

#include "Item.h"

class Player;
class Level;

class FlintAndSteelItem : public Item
{
public:
	FlintAndSteelItem(int id);

	virtual bool useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly=false);
};
