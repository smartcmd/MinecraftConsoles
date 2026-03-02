#pragma once
#include "Tile.h"
#include "Definitions.h"

class Random;

class LadderTile : public Tile
{
	friend class Tile;
protected:
	LadderTile(int id);
public:
	virtual AABB *getAABB(Level *level, int x, int y, int z);
    virtual AABB *getTileAABB(Level *level, int x, int y, int z);
	virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>()); // 4J added forceData, forceEntity param
	using Tile::setShape;
	virtual void setShape(int data);
    virtual bool blocksLight();
    virtual bool isSolidRender(bool isServerLevel = false);
    virtual bool isCubeShaped();
    virtual int getRenderShape();
    virtual bool mayPlace(Level *level, int x, int y, int z);
    virtual int getPlacedOnFaceDataValue(Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue);
    virtual void neighborChanged(Level *level, int x, int y, int z, int type);
    virtual int getResourceCount(Random* random);
};