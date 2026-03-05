#include "stdafx.h"
#include "LiquidTile_SPU.h"
#include "Facing_SPU.h"
#include "ChunkRebuildData.h"

#ifdef SN_TARGET_PS3_SPU
#include "..\Common\spu_assert.h"
#endif

// const wstring LiquidTile::TEXTURE_LAVA_STILL = L"lava";
// const wstring LiquidTile::TEXTURE_WATER_STILL = L"water";
// const wstring LiquidTile::TEXTURE_WATER_FLOW = L"water_flow";
// const wstring LiquidTile::TEXTURE_LAVA_FLOW = L"lava_flow";

#define MATH_PI (3.141592654f)


int LiquidTile_SPU::getColor(ChunkRebuildData *level, int x, int y, int z)
{

	return getColor(level, x, y, z, 0);
}

int LiquidTile_SPU::getColor(ChunkRebuildData *level, int x, int y, int z, int d)
{
 	// MGH - TODO
	if (getMaterial()->getID() == Material_SPU::water_Id)
	{
		//            Biome b = level.getBiomeSource().getBiome(x, z);
		//            return b.waterColor;

		int totalRed = 0;
		int totalGreen = 0;
		int totalBlue = 0;

		for (int oz = -1; oz <= 1; oz++)
		{
			for (int ox = -1; ox <= 1; ox++)
			{
				int waterColor = level->getWaterColor(x + ox, z + oz);

				totalRed += (waterColor & 0xff0000) >> 16;
				totalGreen += (waterColor & 0xff00) >> 8;
				totalBlue += (waterColor & 0xff);
			}
		}

		//            return level.getBiomeSource().getBiome(x, z).getGrassColor(level, x, y, z);
		return (((totalRed / 9) & 0xFF) << 16) | (((totalGreen / 9) & 0xFF) << 8) | (((totalBlue / 9) & 0xFF));
	}
	return 0xffffff;
}

float LiquidTile_SPU::getHeight(int d)
{
    // if (d == 0) d++;
    if (d >= 8) d = 0;
	return (d + 1) / 9.0f;
}

Icon_SPU *LiquidTile_SPU::getTexture(int face, int data)
{
	if (face == Facing::DOWN || face == Facing::UP)
	{
		if(id == water_Id || id == calmWater_Id)
			return &ms_pTileData->liquidTile_iconWaterStill;
		else //(id == lava_Id || id == calmLava_Id)
			return &ms_pTileData->liquidTile_iconLavaStill;
	}
	else
	{
		if(id == water_Id || id == calmWater_Id)
			return &ms_pTileData->liquidTile_iconWaterFlow;
		else //(id == lava_Id || id == calmLava_Id)
			return &ms_pTileData->liquidTile_iconLavaFlow;
    }
}

int LiquidTile_SPU::getDepth(ChunkRebuildData *level, int x, int y, int z)
{
	if (level->getMaterial(x, y, z)->getID() != getMaterial()->getID()) return -1;
	else return -1;;
}

int LiquidTile_SPU::getRenderedDepth(ChunkRebuildData *level, int x, int y, int z)
{
	if (level->getMaterial(x, y, z)->getID() != getMaterial()->getID()) return -1;
	int d = level->getData(x, y, z);
	if (d >= 8) d = 0;
	return d;
}


bool LiquidTile_SPU::isSolidRender(bool isServerLevel)
{
	return false;
}

bool LiquidTile_SPU::isSolidFace(ChunkRebuildData *level, int x, int y, int z, int face)
{
    Material_SPU *m = level->getMaterial(x, y, z);
    if (m->getID() == this->getMaterial()->getID()) return false;
	if (face == Facing::UP) return true;
    if (m->getID() == Material_SPU::ice_Id) return false;
    
    return Tile_SPU::isSolidFace(level, x, y, z, face);
}

bool LiquidTile_SPU::shouldRenderFace(ChunkRebuildData *level, int x, int y, int z, int face)
{
    Material_SPU *m = level->getMaterial(x, y, z);
    if (m->getID() == this->getMaterial()->getID()) return false;
	if (face == Facing::UP) return true;
    if (m->getID() == Material_SPU::ice_Id) return false;
    return Tile_SPU::shouldRenderFace(level, x, y, z, face);
}

int LiquidTile_SPU::getRenderShape()
{
	return Tile_SPU::SHAPE_WATER;
}


Vec3_SPU LiquidTile_SPU::getFlow(ChunkRebuildData *level, int x, int y, int z)
{
    Vec3_SPU flow = Vec3_SPU(0,0,0);
    int mid = getRenderedDepth(level, x, y, z);
    for (int d = 0; d < 4; d++)
	{

        int xt = x;
        int yt = y;
        int zt = z;

        if (d == 0) xt--;
        if (d == 1) zt--;
        if (d == 2) xt++;
        if (d == 3) zt++;

        int t = getRenderedDepth(level, xt, yt, zt);
        if (t < 0)
		{
            if (!level->getMaterial(xt, yt, zt)->blocksMotion())
			{
                t = getRenderedDepth(level, xt, yt - 1, zt);
                if (t >= 0)
				{
                    int dir = t - (mid - 8);
                    flow = flow.add((xt - x) * dir, (yt - y) * dir, (zt - z) * dir);
                }
            }
        } else
		{
            if (t >= 0)
			{
                int dir = t - mid;
                flow = flow.add((xt - x) * dir, (yt - y) * dir, (zt - z) * dir);
            }
        }

    }
    if (level->getData(x, y, z) >= 8)
	{
        bool ok = false;
        if (ok || isSolidFace(level, x, y, z - 1, 2)) ok = true;
        if (ok || isSolidFace(level, x, y, z + 1, 3)) ok = true;
        if (ok || isSolidFace(level, x - 1, y, z, 4)) ok = true;
        if (ok || isSolidFace(level, x + 1, y, z, 5)) ok = true;
        if (ok || isSolidFace(level, x, y + 1, z - 1, 2)) ok = true;
        if (ok || isSolidFace(level, x, y + 1, z + 1, 3)) ok = true;
        if (ok || isSolidFace(level, x - 1, y + 1, z, 4)) ok = true;
        if (ok || isSolidFace(level, x + 1, y + 1, z, 5)) ok = true;
        if (ok) flow = flow.normalize().add(0, -6, 0);
    }
    flow = flow.normalize();
    return flow;
}


// 4J - change brought forward from 1.8.2
int LiquidTile_SPU::getLightColor(ChunkRebuildData *level, int x, int y, int z)
{
	int a = level->getLightColor(x, y, z, 0);
	int b = level->getLightColor(x, y + 1, z, 0);

	int aa = a & 0xff;
	int ba = b & 0xff;
	int ab = (a >> 16) & 0xff;
	int bb = (b >> 16) & 0xff;

	return (aa > ba ? aa : ba) | ((ab > bb ? ab : bb) << 16);
}

float LiquidTile_SPU::getBrightness(ChunkRebuildData *level, int x, int y, int z)
{
    float a = level->getBrightness(x, y, z);
    float b = level->getBrightness(x, y + 1, z);
    return a > b ? a : b;
}


int LiquidTile_SPU::getRenderLayer()
{
	return getMaterial()->getID() == Material_SPU::water_Id ? 1 : 0;
}


double LiquidTile_SPU::getSlopeAngle(ChunkRebuildData *level, int x, int y, int z, Material_SPU *m)
{
    Vec3_SPU flow = Vec3_SPU(0,0,0);
    if (m->getID() == Material_SPU::water_Id)
	{
		TileRef_SPU tRef(Tile_SPU::water_Id);
		flow = static_cast<LiquidTile_SPU *>(tRef.getPtr())->getFlow(level, x, y, z);
	}
    if (m->getID() == Material_SPU::lava_Id)
	{
		TileRef_SPU tRef(Tile_SPU::lava_Id);
		flow = static_cast<LiquidTile_SPU *>(tRef.getPtr())->getFlow(level, x, y, z);
	}
    if (flow.x == 0 && flow.z == 0) return -1000;
    return atan2(flow.z, flow.x) - MATH_PI / 2;
}

