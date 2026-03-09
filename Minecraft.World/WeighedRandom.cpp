#include "stdafx.h"
#include "WeighedRandom.h"

int WeighedRandom::getTotalWeight(vector<WeighedRandomItem *> *items)
{
    int totalWeight = 0;
	for( const auto& item : *items)
	{
		totalWeight += item->randomWeight;
	}
    return totalWeight;
}

WeighedRandomItem *WeighedRandom::getRandomItem(Random *random, vector<WeighedRandomItem *> *items, int totalWeight)
{
    if (totalWeight <= 0)
	{
        __debugbreak();
    }

	int selection = random->nextInt(totalWeight);

	for(const auto& item : *items)
	{
		selection -= item->randomWeight;
        if (selection < 0)
		{
            return item;
        }
	}
    return nullptr;
}

WeighedRandomItem *WeighedRandom::getRandomItem(Random *random, vector<WeighedRandomItem *> *items)
{
	return getRandomItem(random, items, getTotalWeight(items));
}

int WeighedRandom::getTotalWeight(WeighedRandomItemArray items)
{
    int totalWeight = 0;
	for( unsigned int i = 0; i < items.length; i++ )
	{
        totalWeight += items[i]->randomWeight;
    }
    return totalWeight;
}

WeighedRandomItem *WeighedRandom::getRandomItem(Random *random, WeighedRandomItemArray items, int totalWeight)
{
    if (totalWeight <= 0)
	{
        __debugbreak();
    }

    int selection = random->nextInt(totalWeight);
	for( unsigned int i = 0; i < items.length; i++ )
	{
		selection -= items[i]->randomWeight;
        if (selection < 0)
		{
            return items[i];
        }
	}
    return nullptr;
}


WeighedRandomItem *WeighedRandom::getRandomItem(Random *random, WeighedRandomItemArray items)
{
	return getRandomItem(random, items, getTotalWeight(items));
}