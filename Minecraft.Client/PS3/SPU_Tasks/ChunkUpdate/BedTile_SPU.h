#pragma once
#include "Tile_SPU.h"



// TileRenderer not implemented, so minimum of stuff here
class BedTile_SPU : public Tile_SPU
{
public:
// 	static const int PART_FOOT = 0;
// 	static const int PART_HEAD = 1;
// 
//     static const int HEAD_PIECE_DATA = 0x8;
//     static const int OCCUPIED_DATA = 0x4;
// 
//     static int HEAD_DIRECTION_OFFSETS[4][2];

    BedTile_SPU(int id) : Tile_SPU(id) {}

    virtual Icon_SPU *getTexture(int face, int data) { return nullptr; }
	virtual int getRenderShape() { return Tile_SPU::SHAPE_BED; }
	virtual bool isSolidRender(bool isServerLevel = false) { return false; }
    virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, TileEntity* forceEntity = nullptr)	// 4J added forceData, forceEntity param
	{
		setShape();
	}
	void setShape() { 	Tile_SPU::setShape(0, 0, 0, 1, 9 / 16.0f, 1); }
};
