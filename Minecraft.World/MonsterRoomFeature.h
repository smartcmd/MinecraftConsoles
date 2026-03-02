#pragma once
#include "Feature.h"
#include "Material.h"

class MonsterRoomFeature : public Feature
{
private:
	//int tile;

public:
	virtual bool place(Level *level, Random *random, int x, int y, int z);

private:
    shared_ptr<ItemInstance> randomItem(Random *random);
    wstring randomEntityId(Random *random);
};
