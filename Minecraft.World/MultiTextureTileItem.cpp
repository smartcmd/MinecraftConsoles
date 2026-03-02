#include "stdafx.h"
#include "Tile.h"
#include "MultiTextureTileItem.h"
#include "net.minecraft.world.item.crafting.h"


MultiTextureTileItem::MultiTextureTileItem(int id, Tile *parentTile, int *nameExtensions, int iLength, int anyValueName) : TileItem(id)
{
	this->parentTile = parentTile;
	this->nameExtensions = nameExtensions;
	this->m_iNameExtensionsLength=iLength;

	if(anyValueName != -1)
	{
		m_anyValueName = anyValueName;
	}
	else
	{
		m_anyValueName = nameExtensions[0];
	}

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
	return nameExtensions[iData];
}

unsigned int MultiTextureTileItem::getDescriptionId(shared_ptr<ItemInstance> instance) 
{
	int auxValue = instance->getAuxValue();
	if (auxValue == Recipes::ANY_AUX_VALUE || auxValue < 0 || auxValue >= m_iNameExtensionsLength) 
	{
		return m_anyValueName;
	}
	return nameExtensions[auxValue];
}