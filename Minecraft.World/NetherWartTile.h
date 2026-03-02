#pragma once
#include "Bush.h"

class ChunkRebuildData;
class NetherWartTile : public Bush
{
	friend class ChunkRebuildData;
private:
	static const int MAX_AGE = 3;

	static const int NETHER_STALK_TEXTURE_COUNT = 3;
	Icon *icons[NETHER_STALK_TEXTURE_COUNT];

public:
	NetherWartTile(int id);
	virtual void updateDefaultShape(); // 4J Added override
	virtual bool mayPlaceOn(int tile);

	// Brought forward to fix #60073 - TU7: Content: Gameplay: Nether Warts cannot be placed next to each other in the Nether
	virtual bool canSurvive(Level *level, int x, int y, int z);

	virtual void tick(Level *level, int x, int y, int z, Random *random);
	virtual void growCropsToMax(Level *level, int x, int y, int z);
	virtual Icon *getTexture(int face, int data);
	virtual int getRenderShape();
	virtual void spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonus);
	virtual int getResource(int data, Random *random, int playerBonusLevel);
	virtual int getResourceCount(Random *random);
	virtual int cloneTileId(Level *level, int x, int y, int z);
	void registerIcons(IconRegister *iconRegister);
};
