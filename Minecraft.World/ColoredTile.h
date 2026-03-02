#pragma once

#include "Tile.h"

class ColoredTile : public Tile
{
	friend class ChunkRebuildData;

private:
	static const int ICON_COUNT = 16;
	Icon *icons[ICON_COUNT];

public:
	ColoredTile(int id, Material *material);

	Icon *getTexture(int face, int data);
	virtual int getSpawnResourcesAuxValue(int data);
	static int getTileDataForItemAuxValue(int auxValue);
	static int getItemAuxValueForTileData(int data);
	virtual void registerIcons(IconRegister *iconRegister);
};