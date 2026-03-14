#pragma once

#include "CropTile.h"

class CarrotTile : public CropTile
{
	friend class ChunkRebuildData;
private:
	Icon *icons[4];
public:
	CarrotTile(int id);

	Icon *getTexture(int face, int data);

protected:
	int getBaseSeedId();
	int getBasePlantId();
};