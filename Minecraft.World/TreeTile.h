#pragma once

#include "Tile.h"

class ChunkRebuildData;
class Player;

class TreeTile : public Tile
{	
	friend class Tile;
	friend class ChunkRebuildData;
public:
	static const int DARK_TRUNK = 1;
	static const int BIRCH_TRUNK = 2;
	static const int JUNGLE_TRUNK = 3;

	static const int MASK_TYPE = 0x3;
	static const int MASK_FACING = 0xC;
	static const int FACING_Y = 0 << 2;
	static const int FACING_X = 1 << 2;
	static const int FACING_Z = 2 << 2;

	static const int TREE_NAMES_LENGTH = 4;


	static const unsigned int TREE_NAMES[TREE_NAMES_LENGTH];

	static const wstring TREE_TEXTURES[];

private:
	Icon **icons;
	Icon *iconTop;

protected:
	TreeTile(int id);

public:
	virtual int getRenderShape();
	virtual int getResourceCount(Random *random);
	virtual int getResource(int data, Random *random, int playerBonusLevel);
	virtual void onRemove(Level *level, int x, int y, int z, int id, int data);
	virtual void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<Mob> by);
	virtual Icon *getTexture(int face, int data);
	virtual unsigned int getDescriptionId(int iData = -1);

protected:
	int getSpawnResourcesAuxValue(int data);

public:
	static int getWoodType(int data);
	void registerIcons(IconRegister *iconRegister);

protected:
	virtual shared_ptr<ItemInstance> getSilkTouchItemInstance(int data);
};