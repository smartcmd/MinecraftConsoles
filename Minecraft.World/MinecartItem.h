#pragma once
using namespace std;

#include "Item.h"

class MinecartItem : public Item 
{
public:
	int type;

	MinecartItem(int id, int type);

	virtual bool useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly=false);
};