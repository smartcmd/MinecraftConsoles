#pragma once
#include "Tile_SPU.h"


class DoorTile_SPU : public Tile_SPU
{
public:
	static const int UPPER_BIT = 8;
	static const int C_DIR_MASK = 3;
	static const int C_OPEN_MASK = 4;
	static const int C_LOWER_DATA_MASK = 7;
	static const int C_IS_UPPER_MASK = 8;
	static const int C_RIGHT_HINGE_MASK = 16;

private:
	static const int DOOR_TILE_TEXTURE_COUNT = 4;

public:
	DoorTile_SPU(int id) : Tile_SPU(id){}
	virtual Icon_SPU *getTexture(int face, int data);
	//@Override
	Icon_SPU *getTexture(ChunkRebuildData *level, int x, int y, int z, int face);
	//@Override
	virtual bool blocksLight() { return false; }
	virtual bool isSolidRender(bool isServerLevel = false) { return false; }
	virtual int getRenderShape() { return Tile_SPU::SHAPE_DOOR; }
	virtual void updateShape(ChunkRebuildData *level, int x, int y, int z, int forceData = -1, TileEntity* forceEntity = nullptr);	// 4J added forceData, forceEntity param

	int getDir(ChunkRebuildData *level, int x, int y, int z);
	bool isOpen(ChunkRebuildData *level, int x, int y, int z);
private:
	using Tile_SPU::setShape;
	virtual void setShape(int compositeData);
public:
	static bool isOpen(int data);
	int getCompositeData(ChunkRebuildData *level, int x, int y, int z);
};
