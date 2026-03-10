#pragma once
#include "Tile_SPU.h"


class ButtonTile_SPU : public Tile_SPU
{

public:
	ButtonTile_SPU(int id) : Tile_SPU(id) {}

	Icon_SPU *getTexture(int face, int data)
	{
		if(id == Tile_SPU::button_wood_Id) 
			return TileRef_SPU(wood_Id)->getTexture(Facing::UP);
		else 
			return TileRef_SPU(rock_Id)->getTexture(Facing::UP);
	}
    virtual bool blocksLight() { return false; }
    virtual bool isSolidRender(bool isServerLevel = false) { return false; }
	virtual bool isCubeShaped() { return false; }
	virtual void updateShape(ChunkRebuildData *level, int x, int y, int z, int forceData = -1, TileEntity* forceEntity = nullptr)	// 4J added forceData, forceEntity param
	{
		int data = level->getData(x, y, z);
		int dir = data & 7;
		bool pressed = (data & 8) > 0;

		float h0 = 6 / 16.0f;
		float h1 = 10 / 16.0f;
		float r = 3 / 16.0f;
		float d = 2 / 16.0f;
		if (pressed) d = 1 / 16.0f;

		if (dir == 1)
		{
			setShape(0, h0, 0.5f - r, d, h1, 0.5f + r);
		} else if (dir == 2)
		{
			setShape(1 - d, h0, 0.5f - r, 1, h1, 0.5f + r);
		} else if (dir == 3)
		{
			setShape(0.5f - r, h0, 0, 0.5f + r, h1, d);
		} else if (dir == 4)
		{
			setShape(0.5f - r, h0, 1 - d, 0.5f + r, h1, 1);
		}
	}
    virtual void updateDefaultShape()
	{
		float x = 3 / 16.0f;
		float y = 2 / 16.0f;
		float z = 2 / 16.0f;
		setShape(0.5f - x, 0.5f - y, 0.5f - z, 0.5f + x, 0.5f + y, 0.5f + z);
	}
};