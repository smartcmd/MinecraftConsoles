#pragma once

#include "WeighedRandom.h"

class WeighedTreasure : public WeighedRandomItem
{
private:
	shared_ptr<ItemInstance> item;
	int minCount;
	int maxCount;

public:
	WeighedTreasure(int itemId, int auxValue, int minCount, int maxCount, int weight);
	WeighedTreasure(shared_ptr<ItemInstance> item, int minCount, int maxCount, int weight);

	static void addChestItems(Random *random, WeighedTreasureArray items, shared_ptr<Container> dest, int numRolls);
	static void addDispenserItems(Random *random, WeighedTreasureArray items, shared_ptr<DispenserTileEntity> dest, int numRolls);
	static WeighedTreasureArray addToTreasure(WeighedTreasureArray items, WeighedTreasure *extra);
};