#pragma once

#include "Tile.h"

class QuartzBlockTile : public Tile
{
	friend class ChunkRebuildData;
public:
	static const int TYPE_DEFAULT = 0;
	static const int TYPE_CHISELED = 1;
	static const int TYPE_LINES_Y = 2;
	static const int TYPE_LINES_X = 3;
	static const int TYPE_LINES_Z = 4;

	static const int QUARTZ_BLOCK_NAMES = 5;

	static int BLOCK_NAMES[QUARTZ_BLOCK_NAMES];

private:
	static const int QUARTZ_BLOCK_TEXTURES = 5;

	static const wstring TEXTURE_TOP;
	static const wstring TEXTURE_CHISELED_TOP;
	static const wstring TEXTURE_LINES_TOP;
	static const wstring TEXTURE_BOTTOM;
	static const wstring TEXTURE_NAMES[QUARTZ_BLOCK_TEXTURES];

	Icon *icons[QUARTZ_BLOCK_TEXTURES];
	Icon *iconChiseledTop;
	Icon *iconLinesTop;
	Icon *iconTop;
	Icon *iconBottom;

public:
	QuartzBlockTile(int id);

	Icon *getTexture(int face, int data);
	int getPlacedOnFaceDataValue(Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue);
	int getSpawnResourcesAuxValue(int data);

protected:
	shared_ptr<ItemInstance> getSilkTouchItemInstance(int data);

public:
	int getRenderShape();
	void registerIcons(IconRegister *iconRegister);
};