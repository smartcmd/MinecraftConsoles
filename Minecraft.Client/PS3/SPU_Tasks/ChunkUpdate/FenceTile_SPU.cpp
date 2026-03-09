#include "stdafx.h"
#include "FenceTile_SPU.h"
#include "ChunkRebuildData.h"



void FenceTile_SPU::updateShape(ChunkRebuildData *level, int x, int y, int z, int forceData, TileEntity* forceEntity) // 4J added forceData, forceEntity param
{
    bool n = connectsTo(level, x, y, z - 1);
    bool s = connectsTo(level, x, y, z + 1);
    bool w = connectsTo(level, x - 1, y, z);
    bool e = connectsTo(level, x + 1, y, z);

    float west = 6.0f / 16.0f;
    float east = 10.0f / 16.0f;
    float north = 6.0f / 16.0f;
    float south = 10.0f / 16.0f;

    if (n)
	{
        north = 0;
    }
    if (s)
	{
        south = 1;
    }
    if (w)
	{
        west = 0;
    }
    if (e)
	{
        east = 1;
    }

    setShape(west, 0, north, east, 1.0f, south);
}

bool FenceTile_SPU::blocksLight()
{
	return false;
}

bool FenceTile_SPU::isSolidRender(bool isServerLevel)
{
	return false;
}

int FenceTile_SPU::getRenderShape()
{
	return Tile_SPU::SHAPE_FENCE;
}

bool FenceTile_SPU::connectsTo(ChunkRebuildData *level, int x, int y, int z)
{
    int tile = level->getTile(x, y, z);
    if (tile == id || tile == Tile_SPU::fenceGate_Id)
	{
        return true;
    }
    TileRef_SPU tileInstance(tile);
    if (tileInstance.getPtr() != nullptr)
	{
        if (tileInstance->getMaterial()->isSolidBlocking() && tileInstance->isCubeShaped())
		{
            return tileInstance->getMaterial()->getID() != Material_SPU::vegetable_Id;
        }
    }
    return false;
}

bool FenceTile_SPU::shouldRenderFace(ChunkRebuildData *level, int x, int y, int z, int face)
{
	return true;
}