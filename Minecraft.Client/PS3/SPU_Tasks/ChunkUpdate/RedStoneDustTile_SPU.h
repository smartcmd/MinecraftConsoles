#pragma once
#include "Tile_SPU.h"
#include "DiodeTile_SPU.h"

class RedStoneDustTile_SPU : public Tile_SPU
{
public:
	static const int TEXTURE_CROSS = 0;
	static const int TEXTURE_LINE = 1;
	static const int TEXTURE_CROSS_OVERLAY = 2;
	static const int TEXTURE_LINE_OVERLAY = 3;

	RedStoneDustTile_SPU(int id) : Tile_SPU(id) {}
    virtual bool isSolidRender(bool isServerLevel = false) { return false; }
    virtual bool isCubeShaped() { return false; }
	virtual int getRenderShape() { return Tile_SPU::SHAPE_RED_DUST; }
	virtual int getColor() const { return 0x800000; }// 4J Added 
	virtual int getColor(LevelSource *level, int x, int y, int z) { return 0x800000; }
	virtual int getColor(LevelSource *level, int x, int y, int z, int data) { return 0x800000; }	// 4J added
	static Icon_SPU *getTextureByName(int name)
	{
		switch(name)
		{
			case TEXTURE_CROSS: return &ms_pTileData->redStoneDust_iconCross;
			case TEXTURE_LINE: return &ms_pTileData->redStoneDust_iconLine;
			case TEXTURE_CROSS_OVERLAY: return &ms_pTileData->redStoneDust_iconCrossOver;
			case TEXTURE_LINE_OVERLAY: return &ms_pTileData->redStoneDust_iconLineOver;
		}
		return nullptr;
	}

	static bool shouldConnectTo(ChunkRebuildData *level, int x, int y, int z, int direction)
	{
		int t = level->getTile(x, y, z);
		if (t == Tile_SPU::redStoneDust_Id) return true;
		if (t == 0) return false;
		if (t == Tile_SPU::diode_off_Id || t == Tile_SPU::diode_on_Id)
		{
			int data = level->getData(x, y, z);
			return direction == (data & DiodeTile_SPU::DIRECTION_MASK) || direction == Direction::DIRECTION_OPPOSITE[data & DiodeTile_SPU::DIRECTION_MASK];
		}
		else if (TileRef_SPU(t)->isSignalSource() && direction != Direction::UNDEFINED) return true;

		return false;

	}

};