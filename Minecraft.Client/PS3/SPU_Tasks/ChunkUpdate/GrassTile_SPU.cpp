#include "stdafx.h"
#include "GrassTile_SPU.h"
// #include "net.minecraft.world.level.h"
// #include "net.minecraft.world.level.biome.h"
// #include "net.minecraft.h"
// #include "net.minecraft.world.h"

#include "Facing_SPU.h"
#include "ChunkRebuildData.h"


Icon_SPU *GrassTile_SPU::getTexture(int face, int data)
{
    if (face == Facing::UP) return &ms_pTileData->grass_iconTop;
    if (face == Facing::DOWN) return TileRef_SPU(dirt_Id)->getTexture(face);
    return icon();
}

Icon_SPU *GrassTile_SPU::getTexture(ChunkRebuildData *level, int x, int y, int z, int face)
{
    if (face == Facing::UP) return &ms_pTileData->grass_iconTop;
    if (face == Facing::DOWN) return TileRef_SPU(dirt_Id)->getTexture(face);
     Material_SPU *above = level->getMaterial(x, y + 1, z);
     if (above->getID() == Material_SPU::topSnow_Id || above->getID() == Material_SPU::snow_Id) 
 		return &ms_pTileData->grass_iconSnowSide;
     else 
		return icon();
}


int GrassTile_SPU::getColor(ChunkRebuildData *level, int x, int y, int z)
{
	return getColor( level, x, y, z, level->getData( x, y, z ) );
}

// 4J - changed interface to have data passed in, and put existing interface as wrapper above
int GrassTile_SPU::getColor(ChunkRebuildData *level, int x, int y, int z, int data)
{
	//return level->getBiomeSource()->getBiome(x, z)->getGrassColor(level, x, y, z);

	int totalRed = 0;
	int totalGreen = 0;
	int totalBlue = 0;

	for (int oz = -1; oz <= 1; oz++)
	{
		for (int ox = -1; ox <= 1; ox++)
		{
			int grassColor = level->getGrassColor(x + ox, z + oz);
			totalRed += (grassColor & 0xff0000) >> 16;
			totalGreen += (grassColor & 0xff00) >> 8;
			totalBlue += (grassColor & 0xff);
		}
	}

	//        return level.getBiomeSource().getBiome(x, z).getGrassColor(level, x, y, z);
	return (((totalRed / 9) & 0xFF) << 16) | (((totalGreen / 9) & 0xFF) << 8) | (((totalBlue / 9) & 0xFF));
}


Icon_SPU *GrassTile_SPU::getSideTextureOverlay()
{
	return &ms_pTileData->grass_iconSideOverlay;
}

