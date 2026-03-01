#pragma once
#include "Tile.h"

class ChunkRebuildData;
class HugeMushroomTile : public Tile
{
	friend class ChunkRebuildData;
public:
	static const int MUSHROOM_TYPE_BROWN = 0;
	static const int MUSHROOM_TYPE_RED = 1;

	static const wstring TEXTURE_STEM;
	static const wstring TEXTURE_INSIDE;

private:
	static const int HUGE_MUSHROOM_TEXTURE_COUNT = 2;
	static const wstring TEXTURE_TYPE[];
	int type;
	Icon **icons;
	Icon *iconStem;
	Icon *iconInside;
public:
	HugeMushroomTile(int id, Material *material, int type);
	Icon *getTexture(int face, int data);
	int getResourceCount(Random *random);
	int getResource(int data, Random *random, int playerBonusLevel);
	int cloneTileId(Level *level, int x, int y, int z);
	void registerIcons(IconRegister *iconRegister);
};
