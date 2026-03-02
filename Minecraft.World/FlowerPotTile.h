#pragma once

#include "Tile.h"

class FlowerPotTile : public Tile
{
public:
	static const int TYPE_FLOWER_RED = 1;
	static const int TYPE_FLOWER_YELLOW = 2;
	static const int TYPE_SAPLING_DEFAULT = 3;
	static const int TYPE_SAPLING_EVERGREEN = 4;
	static const int TYPE_SAPLING_BIRCH = 5;
	static const int TYPE_SAPLING_JUNGLE = 6;
	static const int TYPE_MUSHROOM_RED = 7;
	static const int TYPE_MUSHROOM_BROWN = 8;
	static const int TYPE_CACTUS = 9;
	static const int TYPE_DEAD_BUSH = 10;
	static const int TYPE_FERN = 11;

	FlowerPotTile(int id);

	void updateDefaultShape();
	bool isSolidRender(bool isServerLevel = false);
	int getRenderShape();
	bool isCubeShaped();
	bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false);
	int cloneTileId(Level *level, int x, int y, int z);
	int cloneTileData(Level *level, int x, int y, int z);
	bool useOwnCloneData();
	bool mayPlace(Level *level, int x, int y, int z);
	void neighborChanged(Level *level, int x, int y, int z, int type);
	using Tile::spawnResources;
	void spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonusLevel);
	int getResource(int data, Random *random, int playerBonusLevel);
	static shared_ptr<ItemInstance> getItemFromType(int type);
	static int getTypeFromItem(shared_ptr<ItemInstance> item);
};