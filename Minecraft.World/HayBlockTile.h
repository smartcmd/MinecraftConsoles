#pragma once

#include "RotatedPillarTile.h"

class HayBlockTile : public RotatedPillarTile
{
	friend class ChunkRebuildData;
public:
	HayBlockTile(int id);

	int getRenderShape();

protected:
	Icon *getTypeTexture(int type);

public:
	void registerIcons(IconRegister *iconRegister);
};