#pragma once

#include "Bush.h"
class ChunkRebuildData;

class TallGrass : public Bush
{
	friend class Tile;
	friend ChunkRebuildData;
public:
	static const int DEAD_SHRUB = 0;
	static const int TALL_GRASS = 1;
	static const int FERN = 2;

	static const int TALL_GRASS_TILE_NAMES_LENGTH = 3;

	static const unsigned int TALL_GRASS_TILE_NAMES[TALL_GRASS_TILE_NAMES_LENGTH];

private:
	static const wstring TEXTURE_NAMES[];
	Icon **icons;

protected:
	TallGrass(int id);

public:
    virtual void updateDefaultShape(); // 4J Added override
	virtual Icon *getTexture(int face, int data);

	virtual int getColor(int auxData);
	virtual int getColor() const;

	virtual int getColor(LevelSource *level, int x, int y, int z);
	virtual int getColor(LevelSource *level, int x, int y, int z, int data);	// 4J added

	virtual int getResource(int data, Random *random, int playerBonusLevel);
	virtual int getResourceCountForLootBonus(int bonusLevel, Random *random);
	virtual void playerDestroy(Level *level, shared_ptr<Player> player, int x, int y, int z, int data);
	virtual int cloneTileData(Level *level, int x, int y, int z);
	virtual unsigned int getDescriptionId(int iData = -1);

	void registerIcons(IconRegister *iconRegister);
};
