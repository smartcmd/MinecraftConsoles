#pragma once

#include "Tile.h"

class WallTile : public Tile
{
public:
	static const float WALL_WIDTH;
	static const float WALL_HEIGHT;
	static const float POST_WIDTH;
	static const float POST_HEIGHT;

	static const int TYPE_NORMAL = 0;
	static const int TYPE_MOSSY = 1;

	static const unsigned int COBBLE_NAMES[2];

	WallTile(int id, Tile *baseTile);

	Icon *getTexture(int face, int data);
	int getRenderShape();
	bool isCubeShaped();
	bool isPathfindable(LevelSource *level, int x, int y, int z);
	bool isSolidRender(bool isServerLevel = false);
	void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());
	AABB *getAABB(Level *level, int x, int y, int z);
	bool connectsTo(LevelSource *level, int x, int y, int z);
	int getSpawnResourcesAuxValue(int data);
	bool shouldRenderFace(LevelSource *level, int x, int y, int z, int face);
	void registerIcons(IconRegister *iconRegister);
};