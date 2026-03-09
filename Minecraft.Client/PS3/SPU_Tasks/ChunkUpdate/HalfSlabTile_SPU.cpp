#include "stdafx.h"
#include "HalfSlabTile_SPU.h"
#include "Facing_SPU.h"
#include "ChunkRebuildData.h"

void HalfSlabTile_SPU::updateShape(ChunkRebuildData *level, int x, int y, int z, int forceData /* = -1 */, TileEntity* forceEntity /* = nullptr */)
{
	if (fullSize()) 
	{
		setShape(0, 0, 0, 1, 1, 1);
	} 
	else 
	{
		bool upper = (level->getData(x, y, z) & TOP_SLOT_BIT) != 0;
		if (upper) 
		{
			setShape(0, 0.5f, 0, 1, 1, 1);
		} 
		else 
		{
			setShape(0, 0, 0, 1, 0.5f, 1);
		}
	}
}

void HalfSlabTile_SPU::updateDefaultShape() 
{
	if (fullSize()) 
	{
		setShape(0, 0, 0, 1, 1, 1);
	} 
	else 
	{
		setShape(0, 0, 0, 1, 0.5f, 1);
	}
}

bool HalfSlabTile_SPU::isSolidRender(bool isServerLevel) 
{
	return fullSize();
}

bool HalfSlabTile_SPU::shouldRenderFace(ChunkRebuildData *level, int x, int y, int z, int face) 
{
		if (fullSize()) return Tile_SPU::shouldRenderFace(level, x, y, z, face);

		if (face != Facing::UP && face != Facing::DOWN && !Tile_SPU::shouldRenderFace(level, x, y, z, face)) 
		{
			return false;
		}

		int ox = x, oy = y, oz = z;
		ox += Facing::STEP_X[Facing::OPPOSITE_FACING[face]];
		oy += Facing::STEP_Y[Facing::OPPOSITE_FACING[face]];
		oz += Facing::STEP_Z[Facing::OPPOSITE_FACING[face]];

		bool isUpper = (level->getData(ox, oy, oz) & TOP_SLOT_BIT) != 0;
		if (isUpper) 
		{
			if (face == Facing::DOWN) return true;
			if (face == Facing::UP && Tile_SPU::shouldRenderFace(level, x, y, z, face)) return true;
			return !(isHalfSlab(level->getTile(x, y, z)) && (level->getData(x, y, z) & TOP_SLOT_BIT) != 0);
		} 
		else 
		{
			if (face == Facing::UP) return true;
			if (face == Facing::DOWN && Tile_SPU::shouldRenderFace(level, x, y, z, face)) return true;
			return !(isHalfSlab(level->getTile(x, y, z)) && (level->getData(x, y, z) & TOP_SLOT_BIT) == 0);
		}
}

bool HalfSlabTile_SPU::isHalfSlab(int tileId) 
{
	return tileId == Tile_SPU::stoneSlabHalf_Id || tileId == Tile_SPU::woodSlabHalf_Id;
}


