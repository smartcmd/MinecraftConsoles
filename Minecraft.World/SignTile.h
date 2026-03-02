#pragma once

#include "EntityTile.h"
#include "TileEntity.h"

#include "stdafx.h"
#include "Material.h"

class SignTile : public EntityTile
{
	friend class Tile;
private:
	eINSTANCEOF clas;
	bool onGround;

protected:
	SignTile(int id, eINSTANCEOF clas, bool onGround);

public:
	Icon *getTexture(int face, int data);
    virtual void updateDefaultShape();
	AABB *getAABB(Level *level, int x, int y, int z);
	AABB *getTileAABB(Level *level, int x, int y, int z);
	void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());	// 4J added forceData, forceEntity param
	int getRenderShape();
	bool isCubeShaped();
	virtual bool isPathfindable(LevelSource *level, int x, int y, int z);
	bool isSolidRender(bool isServerLevel = false);

protected:
	shared_ptr<TileEntity> newTileEntity(Level *level);

public:
	int getResource(int data, Random *random, int playerBonusLevel);
	void neighborChanged(Level *level, int x, int y, int z, int type);
	int cloneTileId(Level *level, int x, int y, int z);
	void registerIcons(IconRegister *iconRegister);
};