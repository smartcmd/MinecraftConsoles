#pragma once
#include "Tile.h"
#include "Definitions.h"

class Random;
class HitResult;

class TorchTile : public Tile
{
	friend class Tile;
protected:
	TorchTile(int id);
public:
	virtual AABB *getAABB(Level *level, int x, int y, int z);
    virtual AABB *getTileAABB(Level *level, int x, int y, int z);
	virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>()); // 4J added forceData, forceEntity param
	using Tile::setShape;
	virtual void setShape(int data);
    virtual bool isSolidRender(bool isServerLevel = false);
    virtual bool isCubeShaped();
    virtual int getRenderShape();
	bool isConnection(Level *level, int x, int y, int z);
    virtual bool mayPlace(Level *level, int x, int y, int z);
    virtual int getPlacedOnFaceDataValue(Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue);
    virtual void tick(Level *level, int x, int y, int z, Random *random);
    virtual void onPlace(Level *level, int x, int y, int z);
    virtual void neighborChanged(Level *level, int x, int y, int z, int type);
private:
	virtual bool checkCanSurvive(Level *level, int x, int y, int z);
public:
	virtual HitResult *clip(Level *level, int x, int y, int z, Vec3 *a, Vec3 *b);
    virtual void animateTick(Level *level, int xt, int yt, int zt, Random *random);
	
	// 4J Added so we can check before we try to add a tile to the tick list if it's actually going to do seomthing
	virtual bool shouldTileTick(Level *level, int x,int y,int z);
};
