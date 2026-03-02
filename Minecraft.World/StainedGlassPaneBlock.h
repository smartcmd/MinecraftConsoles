#pragma once

#include "ThinFenceTile.h"

class StainedGlassPaneBlock : public ThinFenceTile
{
	friend class ChunkRebuildData;
private:
	static const int ICONS_COUNT = 16;
	static Icon *ICONS[ICONS_COUNT];
	static Icon *EDGE_ICONS[ICONS_COUNT];

public:
	StainedGlassPaneBlock(int id);

	Icon *getIconTexture(int face, int data);
	Icon *getEdgeTexture(int data);
	Icon *getTexture(int face, int data);
	int getSpawnResourcesAuxValue(int data);
	static int getItemAuxValueForBlockData(int data);
	int getRenderLayer();
	void registerIcons(IconRegister *iconRegister);
};