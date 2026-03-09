#pragma once
#include "Tile_SPU.h"

class FenceTile_SPU : public Tile_SPU
{
public:
	FenceTile_SPU(int id) : Tile_SPU(id) {}
	virtual void updateShape(ChunkRebuildData *level, int x, int y, int z, int forceData = -1, TileEntity* forceEntity = nullptr); // 4J added forceData, forceEntity param
	virtual bool blocksLight();
	virtual bool isSolidRender(bool isServerLevel = false);
	virtual int getRenderShape();
	virtual bool shouldRenderFace(ChunkRebuildData *level, int x, int y, int z, int face);
	bool connectsTo(ChunkRebuildData *level, int x, int y, int z);
};