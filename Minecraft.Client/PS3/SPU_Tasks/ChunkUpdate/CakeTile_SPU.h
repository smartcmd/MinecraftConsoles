#pragma once
#include "Tile_SPU.h"
#include "ChunkRebuildData.h"
class CakeTile_SPU : public Tile_SPU
{
public:
	CakeTile_SPU(int id) : Tile_SPU(id) {}
    virtual void updateShape(ChunkRebuildData *level, int x, int y, int z, int forceData = -1, TileEntity* forceEntity = nullptr)	// 4J added forceData, forceEntity param
	{
		int d = level->getData(x, y, z);
		float r = 1 / 16.0f;
		float r2 = (1 + d * 2) / 16.0f;
		float h = 8 / 16.0f;
		this->setShape(r2, 0, r, 1 - r, h, 1 - r);
	}
    virtual void updateDefaultShape()
	{
		float r = 1 / 16.0f;
		float h = 8 / 16.0f;
		this->setShape(r, 0, r, 1 - r, h, 1 - r);
	}
    virtual Icon_SPU *getTexture(int face, int data)
	{
		if (face == Facing::UP) return &ms_pTileData->cakeTile_iconTop;
		if (face == Facing::DOWN) return &ms_pTileData->cakeTile_iconBottom;
		if (data > 0 && face == Facing::WEST) return &ms_pTileData->cakeTile_iconInner;
		return icon();
	}
    virtual bool isCubeShaped() { return false; }
    virtual bool isSolidRender(bool isServerLevel = false) { return false; }
};