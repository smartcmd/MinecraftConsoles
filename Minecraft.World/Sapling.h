#pragma once

#include "LeafTile.h"
#include "Bush.h"

class Random;
class ChunkRebuildData;

class Sapling : public Bush
{	
	friend class Tile;
	friend class ChunkRebuildData;
public:
	static const int TYPE_DEFAULT = LeafTile::NORMAL_LEAF;
	static const int TYPE_EVERGREEN = LeafTile::EVERGREEN_LEAF;
	static const int TYPE_BIRCH = LeafTile::BIRCH_LEAF;
	static const int TYPE_JUNGLE = LeafTile::JUNGLE_LEAF;

	static const int SAPLING_NAMES_SIZE = 4;

	static int SAPLING_NAMES[SAPLING_NAMES_SIZE];

private:
	static const wstring TEXTURE_NAMES[];

	Icon **icons;

	static const int TYPE_MASK = 3;
	static const int AGE_BIT = 8;

protected:
	Sapling(int id);

public:
	virtual void updateDefaultShape(); // 4J Added override
	virtual void tick(Level *level, int x, int y, int z, Random *random);

	virtual Icon *getTexture(int face, int data);
	virtual void advanceTree(Level *level, int x, int y, int z, Random *random);
	void growTree(Level *level, int x, int y, int z, Random *random);

	virtual unsigned int getDescriptionId(int iData = -1);
	bool isSapling(Level *level, int x, int y, int z, int type);

protected:
	int getSpawnResourcesAuxValue(int data);

public:
	void registerIcons(IconRegister *iconRegister);
};
