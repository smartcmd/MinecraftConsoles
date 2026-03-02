#pragma once

#include "Tile.h"

class Player;

class TopSnowTile : public Tile
{
	friend class Tile;
public:
	static const int MAX_HEIGHT;
	static const int HEIGHT_MASK;

protected:
	TopSnowTile(int id);

public:
	void registerIcons(IconRegister *iconRegister);
	AABB *getAABB(Level *level, int x, int y, int z);

public:
	static float getHeight(Level *level, int x, int y, int z);

public:
	bool blocksLight();

public:
	bool isSolidRender(bool isServerLevel = false);

public:
	bool isCubeShaped();

public:
	void updateDefaultShape();
	void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());	// 4J added forceData, forceEntity param

protected:
	void updateShape(int data);

public:
	bool mayPlace(Level *level, int x, int y, int z);

public:
	void neighborChanged(Level *level, int x, int y, int z, int type);

private:
	bool checkCanSurvive(Level *level, int x, int y, int z);

public:
	void playerDestroy(Level *level, shared_ptr<Player> player, int x, int y, int z, int data);

public:
	int getResource(int data, Random *random, int playerBonusLevel);

public:
	int getResourceCount(Random *random);

public:
	void tick(Level *level, int x, int y, int z, Random *random);

public:
	bool shouldRenderFace(LevelSource *level, int x, int y, int z, int face);
	
	// 4J Added so we can check before we try to add a tile to the tick list if it's actually going to do seomthing
	virtual bool shouldTileTick(Level *level, int x,int y,int z);
};
