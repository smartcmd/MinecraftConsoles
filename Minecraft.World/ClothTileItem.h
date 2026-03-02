#pragma once
using namespace std;

#include "TileItem.h"

class ClothTileItem : public TileItem 
{
public: 
	static const unsigned int COLOR_DESCS[];
	static const unsigned int CARPET_COLOR_DESCS[];

	ClothTileItem(int id);
	
	virtual Icon *getIcon(int itemAuxValue);
	virtual int getLevelDataForAuxValue(int auxValue);
	virtual unsigned int getDescriptionId(shared_ptr<ItemInstance> instance);
};