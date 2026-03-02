#pragma once
using namespace std;

#include "TileItem.h"

class TreeTileItem : public TileItem 
{
private:
	Tile *parentTile;

public:
	TreeTileItem(int id, Tile *parentTile);

	virtual Icon *getIcon(int itemAuxValue);
	virtual int getLevelDataForAuxValue(int auxValue);

	virtual unsigned int getDescriptionId(shared_ptr<ItemInstance> instance);
};