#pragma once

#include "Tile.h"


class HalfSlabTile : public Tile 
{


public:
 	static const int TYPE_MASK = 7;
 	static const int TOP_SLOT_BIT = 8;

protected:
	bool fullSize;

public:
	HalfSlabTile(int id, bool fullSize, Material *material);
	virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());	// 4J added forceData, forceEntity param
	virtual void updateDefaultShape();
	virtual void addAABBs(Level *level, int x, int y, int z, AABB *box, AABBList *boxes, shared_ptr<Entity> source);
	virtual bool isSolidRender(bool isServerLevel);
	virtual int getPlacedOnFaceDataValue(Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue);
	virtual int getResourceCount(Random *random);
	virtual int getSpawnResourcesAuxValue(int data);
	virtual bool isCubeShaped();
	virtual bool shouldRenderFace(LevelSource *level, int x, int y, int z, int face);
private:
	static bool isHalfSlab(int tileId);
public:
	virtual int getAuxName(int auxValue) = 0;

};