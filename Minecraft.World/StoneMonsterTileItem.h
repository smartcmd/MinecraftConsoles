#pragma once
using namespace std;

#include "TileItem.h"

// 4J Stu - Class brought forward from 12w36 to fix stacking problem with silverfish stones

class StoneMonsterTileItem : public TileItem
{
public:
	StoneMonsterTileItem(int id);

	virtual int getLevelDataForAuxValue(int auxValue);
	virtual Icon *getIcon(int itemAuxValue);
	virtual unsigned int getDescriptionId(shared_ptr<ItemInstance> instance);
};