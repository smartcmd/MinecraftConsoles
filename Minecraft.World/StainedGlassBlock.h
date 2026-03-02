#pragma once

#include "HalfTransparentTile.h"

class StainedGlassBlock : public HalfTransparentTile
{
	friend class ChunkRebuildData;
private:
	static const int ICONS_LENGTH = 16;
	static Icon *ICONS[ICONS_LENGTH];

public:
	StainedGlassBlock(int id, Material *material);

	Icon *getTexture(int face, int data);
	int getSpawnResourcesAuxValue(int data);
	static int getItemAuxValueForBlockData(int data);
	int getRenderLayer();
	void registerIcons(IconRegister *iconRegister);
	int getResourceCount(Random *random);

protected:
	bool isSilkTouchable();

public:
	bool isCubeShaped();
};