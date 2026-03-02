#pragma once

#include "Tile.h"

class WoolCarpetTile : public Tile
{
	friend class Tile;
protected:
	WoolCarpetTile(int id);

public:
	Icon *getTexture(int face, int data);
	AABB *getAABB(Level *level, int x, int y, int z);
	bool blocksLight();
	bool isSolidRender(bool isServerLevel = false);
	bool isCubeShaped();
	void updateDefaultShape();
	void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());

protected:
	void updateShape(int data);

public:
	bool mayPlace(Level *level, int x, int y, int z);
	void neighborChanged(Level *level, int x, int y, int z, int type);

private:
	bool checkCanSurvive(Level *level, int x, int y, int z);

public:
	bool canSurvive(Level *level, int x, int y, int z);
	bool shouldRenderFace(LevelSource *level, int x, int y, int z, int face);
	int getSpawnResourcesAuxValue(int data);
	static int getTileDataForItemAuxValue(int auxValue);
	static int getItemAuxValueForTileData(int data);
	void registerIcons(IconRegister *iconRegister);
};