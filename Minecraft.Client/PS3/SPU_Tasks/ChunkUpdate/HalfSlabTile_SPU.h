#pragma once
#include "Tile_SPU.h"


class HalfSlabTile_SPU : public Tile_SPU
{
public:
	static const int TYPE_MASK = 7;
	static const int TOP_SLOT_BIT = 8;

	HalfSlabTile_SPU(int id) : Tile_SPU(id) {}
	virtual void updateShape(ChunkRebuildData *level, int x, int y, int z, int forceData = -1, TileEntity* forceEntity = nullptr);	// 4J added forceData, forceEntity param
	virtual void updateDefaultShape();
	virtual bool isSolidRender(bool isServerLevel);
	virtual bool shouldRenderFace(ChunkRebuildData *level, int x, int y, int z, int face);

	bool fullSize() { return !isHalfSlab(id); }
	static bool isHalfSlab(int tileId);
};