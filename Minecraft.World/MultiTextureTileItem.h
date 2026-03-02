#pragma once

#include "TileItem.h"

class Tile;

class MultiTextureTileItem : public TileItem 
{
private:
	Tile *parentTile;
	//private final String[] nameExtensions;
	int *nameExtensions;
	int m_iNameExtensionsLength;

public:
	MultiTextureTileItem(int id, Tile *parentTile,int *nameExtensions, int iLength);

	virtual Icon *getIcon(int itemAuxValue);
	virtual int getLevelDataForAuxValue(int auxValue);
	virtual unsigned int getDescriptionId(int iData = -1);
	virtual unsigned int getDescriptionId(shared_ptr<ItemInstance> instance);
};
