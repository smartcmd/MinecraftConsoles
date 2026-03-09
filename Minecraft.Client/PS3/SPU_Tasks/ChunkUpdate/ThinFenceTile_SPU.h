#pragma once
#include "Tile_SPU.h"

class ThinFenceTile_SPU : public Tile_SPU
{

public:
	ThinFenceTile_SPU(int id) : Tile_SPU(id) {}

	virtual bool isSolidRender(bool isServerLevel = false);
    virtual int getRenderShape();
    virtual bool shouldRenderFace(ChunkRebuildData *level, int x, int y, int z, int face);
    virtual void updateDefaultShape();
    virtual void updateShape(ChunkRebuildData *level, int x, int y, int z, int forceData = -1, TileEntity* forceEntity = nullptr);	// 4J added forceData, forceEntity param
    virtual Icon_SPU *getEdgeTexture();
	bool attachsTo(int tile);
};
