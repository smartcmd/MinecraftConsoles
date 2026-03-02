#pragma once
#include "Tile.h"

class LeverTile : public Tile
{
	friend class Tile;
protected:
	LeverTile(int id);
public:
    virtual AABB *getAABB(Level *level, int x, int y, int z);
    virtual bool blocksLight();
    virtual bool isSolidRender(bool isServerLevel = false);
    virtual bool isCubeShaped();
    virtual int getRenderShape();
    virtual bool mayPlace(Level *level, int x, int y, int z, int face);
    virtual bool mayPlace(Level *level, int x, int y, int z);
    virtual int getPlacedOnFaceDataValue(Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue);
	static int getLeverFacing(int facing);
    virtual void neighborChanged(Level *level, int x, int y, int z, int type);
private:
	virtual bool checkCanSurvive(Level *level, int x, int y, int z);
public:
	virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());	// 4J added forceData, forceEntity param
    virtual void attack(Level *level, int x, int y, int z, shared_ptr<Player> player);
	virtual bool TestUse();
    virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false); // 4J added soundOnly param
    virtual void onRemove(Level *level, int x, int y, int z, int id, int data);
    virtual bool getSignal(LevelSource *level, int x, int y, int z, int dir);
    virtual bool getDirectSignal(Level *level, int x, int y, int z, int dir);
    virtual bool isSignalSource();
};
