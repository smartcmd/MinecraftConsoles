#pragma once
#include "Tile_SPU.h"

class RailTile_SPU : public Tile_SPU
{

public:
	static const int RAIL_DATA_BIT = 8;
	static const int RAIL_DIRECTION_MASK = 7;

	RailTile_SPU(int id) : Tile_SPU(id) {}
    virtual bool isSolidRender(bool isServerLevel = false) { return false; }
    virtual void updateShape(ChunkRebuildData *level, int x, int y, int z, int forceData = -1, TileEntity* forceEntity = nullptr)	// 4J added forceData, forceEntity param
	{
		int data = level->getData(x, y, z);
		if (data >= 2 && data <= 5)
		{
			setShape(0, 0, 0, 1, 2 / 16.0f + 0.5f, 1);
		} else
		{
			setShape(0, 0, 0, 1, 2 / 16.0f, 1);
		}
	}
    virtual Icon_SPU *getTexture(int face, int data)
	{
		bool usesDataBit = false;
		Icon_SPU* iconTurn = &ms_pTileData->railTile_iconTurn;
		if(id == goldenRail_Id)
		{
			usesDataBit = true;
			iconTurn = &ms_pTileData->railTile_iconTurnGolden;
		}

		if (usesDataBit)
		{
// 			if (id == Tile::goldenRail_Id)
// 			{
				if ((data & RAIL_DATA_BIT) == 0)
				{
					return icon();
				}
				else
				{
					return iconTurn; // Actually the powered rail on version
				}
// 			}
		} else if (data >= 6) return iconTurn;
		return icon();

	}
    virtual int getRenderShape() { return Tile_SPU::SHAPE_RAIL; }
	bool isUsesDataBit()
	{
		if(id == goldenRail_Id || id == detectorRail_Id)
			return true;
		return false;
	}
};