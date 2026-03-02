#pragma once

#include "Item.h"

class LeashItem : public Item
{
public:
	LeashItem(int id);

	bool useOn(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly=false);
	static bool bindPlayerMobs(shared_ptr<Player> player, Level *level, int x, int y, int z);
	static bool bindPlayerMobsTest(shared_ptr<Player> player, Level *level, int x, int y, int z);
};