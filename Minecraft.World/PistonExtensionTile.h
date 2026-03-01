#pragma once
#include "Tile.h"

class PistonExtensionTile : public Tile
{
public:
	// i'm reusing this block for the sticky pistons
	static const int STICKY_BIT = 8;

private:
	Icon *overrideTopTexture;

public:
	PistonExtensionTile(int id);
	virtual void setOverrideTopTexture(Icon *overrideTopTexture);
	virtual void clearOverrideTopTexture();
	virtual void playerWillDestroy(Level *level, int x, int y, int z, int data, shared_ptr<Player> player);
	virtual void onRemove(Level *level, int x, int y, int z, int id, int data);
	virtual Icon *getTexture(int face, int data);
	virtual void registerIcons(IconRegister *iconRegister);
	virtual int getRenderShape();
	virtual bool isSolidRender(bool isServerLevel = false);
	virtual bool isCubeShaped();
	virtual bool mayPlace(Level *level, int x, int y, int z);
	virtual bool mayPlace(Level *level, int x, int y, int z, int face);
	virtual int getResourceCount(Random *random);
	virtual void addAABBs(Level *level, int x, int y, int z, AABB *box, AABBList *boxes, shared_ptr<Entity> source);
	virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());	// 4J added forceData, forceEntity param
	virtual void neighborChanged(Level *level, int x, int y, int z, int type);
	static int getFacing(int data);
	virtual int cloneTileId(Level *level, int x, int y, int z);
};