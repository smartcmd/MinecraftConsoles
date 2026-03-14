#pragma once

#include "CropTile.h"

class PotatoTile : public CropTile
{
	friend class ChunkRebuildData;
private:
	Icon *icons[4];

public:
	PotatoTile(int id);

	Icon *getTexture(int face, int data);

protected:
	int getBaseSeedId();
	int getBasePlantId();

public:
	void spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonus);
};