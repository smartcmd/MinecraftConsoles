#include "stdafx.h"
#include "ThinFenceTile_SPU.h"
#include "ChunkRebuildData.h"

bool ThinFenceTile_SPU::isSolidRender(bool isServerLevel)
{
	return false;
}

int ThinFenceTile_SPU::getRenderShape()
{
	return Tile_SPU::SHAPE_IRON_FENCE;
}

bool ThinFenceTile_SPU::shouldRenderFace(ChunkRebuildData *level, int x, int y, int z, int face)
{
    int id = level->getTile(x, y, z);
    if (id == this->id) return false;
    return Tile_SPU::shouldRenderFace(level, x, y, z, face);
}

void ThinFenceTile_SPU::updateDefaultShape()
{
	setShape(0, 0, 0, 1, 1, 1);
}

void ThinFenceTile_SPU::updateShape(ChunkRebuildData *level, int x, int y, int z, int forceData, TileEntity* forceEntity) // 4J added forceData, forceEntity param
{
    float minX = 7.0f / 16.0f;
    float maxX = 9.0f / 16.0f;
    float minZ = 7.0f / 16.0f;
    float maxZ = 9.0f / 16.0f;

    bool n = attachsTo(level->getTile(x, y, z - 1));
    bool s = attachsTo(level->getTile(x, y, z + 1));
    bool w = attachsTo(level->getTile(x - 1, y, z));
    bool e = attachsTo(level->getTile(x + 1, y, z));

    if ((w && e) || (!w && !e && !n && !s))
	{
        minX = 0;
        maxX = 1;
    }
	else if (w && !e)
	{
        minX = 0;
    }
	else if (!w && e)
	{
        maxX = 1;
    }
    if ((n && s) || (!w && !e && !n && !s))
	{
        minZ = 0;
        maxZ = 1;
    }
	else if (n && !s)
	{
        minZ = 0;
    }
	else if (!n && s)
	{
        maxZ = 1;
    }
	setShape(minX, 0, minZ, maxX, 1, maxZ);
}

Icon_SPU *ThinFenceTile_SPU::getEdgeTexture()
{
	if(id == Tile_SPU::ironFence_Id)
		return &ms_pTileData->ironFence_EdgeTexture;
	if(id == Tile_SPU::thinGlass_Id)
		return &ms_pTileData->thinGlass_EdgeTexture;
#ifndef SN_TARGET_PS3_SPU
	assert(0);
#endif
	return nullptr;
}

bool ThinFenceTile_SPU::attachsTo(int tile)
{
	return ms_pTileData->solid[tile] || tile == id || tile == Tile_SPU::glass_Id;
}
