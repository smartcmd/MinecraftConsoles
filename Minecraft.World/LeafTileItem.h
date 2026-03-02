#pragma once
using namespace std;

#include "TileItem.h"

class LeafTileItem : public TileItem 
{
	using TileItem::getColor;
public:
	LeafTileItem(int id);

	virtual int getLevelDataForAuxValue(int auxValue);
	virtual Icon *getIcon(int itemAuxValue);
	virtual int getColor(shared_ptr<ItemInstance> item, int spriteLayer);

	virtual unsigned int getDescriptionId(shared_ptr<ItemInstance> instance);
};
