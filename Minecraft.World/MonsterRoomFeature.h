#pragma once
#include "Feature.h"
#include "Material.h"

class WeighedTreasure;

class MonsterRoomFeature : public Feature
{
private:
	
		static const int TREASURE_ITEMS_COUNT = 15;
		static WeighedTreasure *monsterRoomTreasure[TREASURE_ITEMS_COUNT];

public:
	virtual bool place(Level *level, Random *random, int x, int y, int z);

private:
	wstring randomEntityId(Random *random);
};
