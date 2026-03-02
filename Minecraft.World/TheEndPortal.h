#pragma once
#include "EntityTile.h"

class TheEndPortal : public EntityTile
{
public:
	static DWORD tlsIdx;
	// 4J - was just a static but implemented with TLS for our version
    static bool allowAnywhere();	
	static void allowAnywhere(bool set);	

	TheEndPortal(int id, Material *material);

    virtual shared_ptr<TileEntity> newTileEntity(Level *level);
    virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());	// 4J added forceData, forceEntity param
    virtual bool shouldRenderFace(LevelSource *level, int x, int y, int z, int face);
    virtual void addAABBs(Level *level, int x, int y, int z, AABB *box, AABBList *boxes, shared_ptr<Entity> source);
    virtual bool isSolidRender(bool isServerLevel = false);
    virtual bool isCubeShaped();
    virtual int getResourceCount(Random *random);
    virtual void entityInside(Level *level, int x, int y, int z, shared_ptr<Entity> entity);
    virtual void animateTick(Level *level, int xt, int yt, int zt, Random *random);
    virtual int getRenderShape();
    virtual void onPlace(Level *level, int x, int y, int z);
	virtual int cloneTileId(Level *level, int x, int y, int z);
	void registerIcons(IconRegister *iconRegister);
};