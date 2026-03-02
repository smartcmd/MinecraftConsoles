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
	int m_anyValueName; // 4J Added

public:
	MultiTextureTileItem(int id, Tile *parentTile,int *nameExtensions, int iLength, int anyValueName = -1); // 4J Added anyValueName

	virtual Icon *getIcon(int itemAuxValue);
	virtual int getLevelDataForAuxValue(int auxValue);
	virtual unsigned int getDescriptionId(int iData = -1);
	virtual unsigned int getDescriptionId(shared_ptr<ItemInstance> instance);
};
