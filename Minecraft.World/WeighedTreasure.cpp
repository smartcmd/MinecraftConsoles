#include "stdafx.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "net.minecraft.world.item.h"
#include "WeighedRandom.h"
#include "WeighedTreasure.h"

WeighedTreasure::WeighedTreasure(int itemId, int auxValue, int minCount, int maxCount, int weight) : WeighedRandomItem(weight)
{
	this->item = shared_ptr<ItemInstance>( new ItemInstance(itemId, 1, auxValue) );
	this->minCount = minCount;
	this->maxCount = maxCount;
}

WeighedTreasure::WeighedTreasure(shared_ptr<ItemInstance> item, int minCount, int maxCount, int weight) : WeighedRandomItem(weight)
{
	this->item = item;
	this->minCount = minCount;
	this->maxCount = maxCount;
}

void WeighedTreasure::addChestItems(Random *random, WeighedTreasureArray items, shared_ptr<ChestTileEntity> dest, int numRolls)
{
	for (int r = 0; r < numRolls; r++)
	{
		WeighedTreasure *treasure = (WeighedTreasure *) WeighedRandom::getRandomItem(random, *((WeighedRandomItemArray *)&items));

		int count = treasure->minCount + random->nextInt(treasure->maxCount - treasure->minCount + 1);
		if (treasure->item->getMaxStackSize() >= count)
		{
			shared_ptr<ItemInstance> copy = treasure->item->copy();
			copy->count = count;
			dest->setItem(random->nextInt(dest->getContainerSize()), copy);
		}
		else
		{
			// use multiple slots
			for (int c = 0; c < count; c++)
			{
				shared_ptr<ItemInstance> copy = treasure->item->copy();
				copy->count = 1;
				dest->setItem(random->nextInt(dest->getContainerSize()), copy);
			}
		}
	}
}

void WeighedTreasure::addDispenserItems(Random *random, WeighedTreasureArray items, shared_ptr<DispenserTileEntity> dest, int numRolls)
{
	for (int r = 0; r < numRolls; r++)
	{
		WeighedTreasure *treasure = (WeighedTreasure *) WeighedRandom::getRandomItem(random, *((WeighedRandomItemArray *)&items));

		int count = treasure->minCount + random->nextInt(treasure->maxCount - treasure->minCount + 1);
		if (treasure->item->getMaxStackSize() >= count)
		{
			shared_ptr<ItemInstance> copy = treasure->item->copy();
			copy->count = count;
			dest->setItem(random->nextInt(dest->getContainerSize()), copy);
		}
		else
		{
			// use multiple slots
			for (int c = 0; c < count; c++)
			{
				shared_ptr<ItemInstance> copy = treasure->item->copy();
				copy->count = 1;
				dest->setItem(random->nextInt(dest->getContainerSize()), copy);
			}
		}
	}
}

WeighedTreasureArray WeighedTreasure::addToTreasure(WeighedTreasureArray items, WeighedTreasure *extra)
{
	WeighedTreasureArray result(items.length + 1);
	int i = 0;

	for (int j = 0; j < items.length; j++)
	{
		result[i++] = items[j];
	}

	result[i++] = extra;

	return result;
}