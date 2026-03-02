#pragma once

#include "Tile.h"
#include "HalfSlabTile.h"

class Player;

class WoodSlabTile : HalfSlabTile
{	

	friend class Tile;
public:
	static const int TYPE_MASK = 7;
	static const int TOP_SLOT_BIT = 8;
	static const int SLAB_NAMES_LENGTH = 4;
	static const unsigned int SLAB_NAMES[SLAB_NAMES_LENGTH];

	WoodSlabTile(int id, bool fullSize);
	virtual Icon *getTexture(int face, int data);
	virtual int getResource(int data, Random *random, int playerBonusLevel);
	virtual int getAuxName(int auxValue); 

	virtual shared_ptr<ItemInstance> getSilkTouchItemInstance(int data);
	void registerIcons(IconRegister *iconRegister);
	
	// 4J added
	virtual unsigned int getDescriptionId(int iData = -1);
};