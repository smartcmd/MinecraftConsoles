#pragma once
#include "DirectionalTile.h"

class FenceGateTile : public DirectionalTile
{
private:
    static const int OPEN_BIT = 4;

public:
	FenceGateTile(int id);
    Icon *getTexture(int face, int data);
    virtual bool mayPlace(Level *level, int x, int y, int z);
    virtual AABB *getAABB(Level *level, int x, int y, int z);
	virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>()); // 4J added forceData, forceEntity param // Brought forward from 1.2.3
    virtual bool blocksLight();
    virtual bool isSolidRender(bool isServerLevel = false);
    virtual bool isCubeShaped();
	virtual bool isPathfindable(LevelSource *level, int x, int y, int z);
	virtual bool shouldRenderFace(LevelSource *level, int x, int y, int z, int face);
    virtual int getRenderShape();
    virtual void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<Mob> by);
    virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false); // 4J added soundOnly param
	virtual void neighborChanged(Level *level, int x, int y, int z, int type);
    static bool isOpen(int data);
	void registerIcons(IconRegister *iconRegister);
};
