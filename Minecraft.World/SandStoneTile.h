#pragma once
using namespace std;

#include "Tile.h"

class ChunkRebuildData;

class SandStoneTile : public Tile 
{
	friend class ChunkRebuildData;
public:
	static const int TYPE_DEFAULT = 0;
	static const int TYPE_HEIROGLYPHS = 1;
	static const int TYPE_SMOOTHSIDE = 2;

	// Add this in when we need it
	//static final String[] SANDSTONE_NAMES = {"default", "chiseled", "smooth"};

	static const int SANDSTONE_BLOCK_NAMES = 3;
	static int SANDSTONE_NAMES[SANDSTONE_BLOCK_NAMES];

private:
	static const wstring TEXTURE_TOP;
	static const wstring TEXTURE_BOTTOM;
	static const int SANDSTONE_TILE_TEXTURE_COUNT = 3;
	static const wstring TEXTURE_NAMES[];

	Icon **icons;
	Icon *iconTop;
	Icon *iconBottom;
public:
	SandStoneTile(int id);

public:
	Icon *getTexture(int face, int data);
	virtual int getSpawnResourcesAuxValue(int data);
	void registerIcons(IconRegister *iconRegister);
};