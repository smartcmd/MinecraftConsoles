#pragma once
using namespace std;

#include "Item.h"

class ShearsItem : public Item 
{
public:
	ShearsItem(int itemId);
	virtual bool mineBlock(shared_ptr<ItemInstance> itemInstance, Level *level, int tile, int x, int y, int z, shared_ptr<Mob> owner);
	virtual bool canDestroySpecial(Tile *tile);
	virtual float getDestroySpeed(shared_ptr<ItemInstance> itemInstance, Tile *tile);
};