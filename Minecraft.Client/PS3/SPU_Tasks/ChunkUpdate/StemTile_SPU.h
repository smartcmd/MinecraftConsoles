#pragma once
#include "Bush_SPU.h"

class StemTile_SPU : public Bush_SPU
{
private:
public:
	StemTile_SPU(int id) : Bush_SPU(id) {}

public:
	int getColor(int data) const
	{
		//int r = data * 32;
		//int g = 255 - data * 8;
		//int b = data * 4;
		//return r << 16 | g << 8 | b;

		int colour = 0;

		unsigned int minColour = ms_pTileData->stemTile_minColour;
		unsigned int maxColour = ms_pTileData->stemTile_maxColour;

		byte redComponent = ((minColour>>16)&0xFF) + (( (maxColour>>16)&0xFF - (minColour>>16)&0xFF)*( data/7.0f));
		byte greenComponent = ((minColour>>8)&0xFF) + (( (maxColour>>8)&0xFF - (minColour>>8)&0xFF)*( data/7.0f));
		byte blueComponent = ((minColour)&0xFF) + (( (maxColour)&0xFF - (minColour)&0xFF)*( data/7.0f));

		colour = redComponent<<16 | greenComponent<<8 | blueComponent;
		return colour;
	}

	virtual int getColor(ChunkRebuildData *level, int x, int y, int z) { 	return getColor(level->getData(x, y, z)); }
    virtual void updateDefaultShape()
	{    
		float ss = 0.125f;
		this->setShape(0.5f - ss, 0, 0.5f - ss, 0.5f + ss, 0.25f, 0.5f + ss);
	}

    virtual void updateShape(ChunkRebuildData *level, int x, int y, int z, int forceData = -1, TileEntity* forceEntity = nullptr)	// 4J added forceData, forceEntity param
	{
		ms_pTileData->yy1[id] = (level->getData(x, y, z) * 2 + 2) / 16.0f;
		float ss = 0.125f;
		this->setShape(0.5f - ss, 0, 0.5f - ss, 0.5f + ss, (float) ms_pTileData->yy1[id], 0.5f + ss);
	}

	int getConnectDir(ChunkRebuildData *level, int x, int y, int z)
	{
		int fruitID = pumpkin_Id;
		if(id == melonStem_Id)
			fruitID = melon_Id;

		int d = level->getData(x, y, z);
		if (d < 7) return -1;
		if (level->getTile(x - 1, y, z) == fruitID) return 0;
		if (level->getTile(x + 1, y, z) == fruitID) return 1;
		if (level->getTile(x, y, z - 1) == fruitID) return 2;
		if (level->getTile(x, y, z + 1) == fruitID) return 3;
		return -1;
	}

	virtual int getRenderShape() { return Tile_SPU::SHAPE_STEM; }
	Icon_SPU *getAngledTexture() { 	return &ms_pTileData->stemTile_iconAngled; }
};
