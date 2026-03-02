#pragma once

#include "Tile.h"

class RotatedPillarTile : public Tile
{
public:
	static const int MASK_TYPE = 0x3;
	static const int MASK_FACING = 0xC;
	static const int FACING_Y = 0 << 2;
	static const int FACING_X = 1 << 2;
	static const int FACING_Z = 2 << 2;

protected:
	Icon *iconTop;

	RotatedPillarTile(int id, Material *material);

public:
	virtual int getRenderShape();
	virtual int getPlacedOnFaceDataValue(Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue);
	virtual Icon *getTexture(int face, int data);

protected:
	virtual Icon *getTypeTexture(int type) = 0;

	virtual Icon *getTopTexture(int type);

public:
	virtual int getSpawnResourcesAuxValue(int data);
	virtual int getType(int data);

protected:
	virtual shared_ptr<ItemInstance> getSilkTouchItemInstance(int data);
};