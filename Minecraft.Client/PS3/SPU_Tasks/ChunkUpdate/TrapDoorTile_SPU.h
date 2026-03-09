#pragma once

#include "Tile_SPU.h"


class TrapDoorTile_SPU : public Tile_SPU
{
public:
	TrapDoorTile_SPU(int id) : Tile_SPU(id) {}

/*
 * public int getTexture(int face, int data) { if (face == 0 || face == 1)
 * return tex; int dir = getDir(data); if ((dir == 0 || dir == 2) ^ (face <= 3))
 * { return tex; } int tt = (dir / 2 + ((face & 1) ^ dir)); tt += ((data & 4) /
 * 4); int texture = tex - (data & 8) * 2; if ((tt & 1) != 0) { texture =
 * -texture; } // if (getDir(data)==0 // tt-=((face+data&3)&1)^((data&4)>>2);
 * return texture; }
 */

	bool blocksLight() { return false; }
	bool isSolidRender(bool isServerLevel = false) { return false; }
	bool isCubeShaped() { return false; }
	int getRenderShape() { return Tile_SPU::SHAPE_BLOCK;}
	void updateShape(ChunkRebuildData *level, int x, int y, int z, int forceData = -1, TileEntity* forceEntity = nullptr)	// 4J added forceData, forceEntity param
	{
		setShape(level->getData(x, y, z));
	}

	void updateDefaultShape()
	{
		float r = 3 / 16.0f;
		setShape(0, 0.5f - r / 2, 0, 1, 0.5f + r / 2, 1);
	}

	bool isOpen(int data){ return (data & 4) != 0;	}

	using Tile_SPU::setShape;
	void setShape(int data)
	{
		float r = 3 / 16.0f;
		Tile_SPU::setShape(0, 0, 0, 1, r, 1);
		if (isOpen(data))
		{
			if ((data & 3) == 0) setShape(0, 0, 1 - r, 1, 1, 1);
			if ((data & 3) == 1) setShape(0, 0, 0, 1, 1, r);
			if ((data & 3) == 2) setShape(1 - r, 0, 0, 1, 1, 1);
			if ((data & 3) == 3) setShape(0, 0, 0, r, 1, 1);
		}
	}

};