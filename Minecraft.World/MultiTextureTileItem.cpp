#include "stdafx.h"
#include "Tile.h"
#include "MultiTextureTileItem.h"


MultiTextureTileItem::MultiTextureTileItem(int id, Tile *parentTile, int *nameExtensions, int iLength) : TileItem(id)
{
	this->parentTile = parentTile;
	this->nameExtensions = nameExtensions;
	this->m_iNameExtensionsLength=iLength;

	setMaxDamage(0);
	setStackedByData(true);
}

Icon *MultiTextureTileItem::getIcon(int itemAuxValue) 
{
	return parentTile->getTexture(2, itemAuxValue);
}

int MultiTextureTileItem::getLevelDataForAuxValue(int auxValue) 
{
		return auxValue;
}

unsigned int MultiTextureTileItem::getDescriptionId(int iData)
{
	if (iData < 0 || iData >= m_iNameExtensionsLength) 
	{
		iData = 0;
	}
	//return super.getDescriptionId() + "." + nameExtensions[auxValue];
	return nameExtensions[iData];
}

unsigned int MultiTextureTileItem::getDescriptionId(shared_ptr<ItemInstance> instance) 
{
		int auxValue = instance->getAuxValue();
		if (auxValue < 0 || auxValue >= m_iNameExtensionsLength) 
		{
			auxValue = 0;
		}
		//return super.getDescriptionId() + "." + nameExtensions[auxValue];
		return nameExtensions[auxValue];
}

