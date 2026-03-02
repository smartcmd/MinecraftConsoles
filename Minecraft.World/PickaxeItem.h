#pragma once

#include "DiggerItem.h"

#define PICKAXE_DIGGABLES 23

class PickaxeItem : public DiggerItem
{
private:
	static TileArray diggables;

public: // 
	static void staticCtor();

	PickaxeItem(int id, const Tier *tier);

public:
	virtual bool canDestroySpecial(Tile *tile);
	virtual float getDestroySpeed(shared_ptr<ItemInstance> itemInstance, Tile *tile);	// 4J - brought forward from 1.2.3
};
