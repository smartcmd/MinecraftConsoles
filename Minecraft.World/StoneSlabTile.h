#pragma once
using namespace std;

#include "HalfSlabTile.h"

class ChunkRebuildData;

class StoneSlabTile : public HalfSlabTile
{
	friend ChunkRebuildData;
public:
	static const int STONE_SLAB = 0;
	static const int SAND_SLAB = 1;
	static const int WOOD_SLAB = 2;
	static const int COBBLESTONE_SLAB = 3;
	static const int BRICK_SLAB = 4;
	static const int SMOOTHBRICK_SLAB = 5;
	static const int NETHERBRICK_SLAB = 6;
	static const int QUARTZ_SLAB = 7;

	static const int SLAB_NAMES_LENGTH = 8;

	static const unsigned int SLAB_NAMES[SLAB_NAMES_LENGTH];

private:
	Icon *iconSide;

public:
	StoneSlabTile(int id, bool fullSize);

	virtual Icon *getTexture(int face, int data);

	void registerIcons(IconRegister *iconRegister);
	virtual int getResource(int data, Random *random, int playerBonusLevel);
	virtual unsigned int getDescriptionId(int iData = -1);
	virtual int getAuxName(int auxValue);
protected:
	virtual shared_ptr<ItemInstance> getSilkTouchItemInstance(int data);
};