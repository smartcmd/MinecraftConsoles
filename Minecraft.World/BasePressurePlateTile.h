#pragma once

#include "Tile.h"

class BasePressurePlateTile : public Tile
{
private:
	wstring texture;

protected:
	BasePressurePlateTile(int id, const wstring &tex, Material *material);

public:
	virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());

protected:
	virtual void updateShape(int data);

public:
	virtual int getTickDelay(Level *level);
	virtual AABB *getAABB(Level *level, int x, int y, int z);
	virtual bool isSolidRender(bool isServerLevel = false);
	virtual bool blocksLight();
	virtual bool isCubeShaped();
	virtual bool isPathfindable(LevelSource *level, int x, int y, int z);
	virtual bool mayPlace(Level *level, int x, int y, int z);
	virtual void neighborChanged(Level *level, int x, int y, int z, int type);
	virtual void tick(Level *level, int x, int y, int z, Random *random);
	virtual void entityInside(Level *level, int x, int y, int z, shared_ptr<Entity> entity);

protected:
	virtual void checkPressed(Level *level, int x, int y, int z, int oldSignal);
	virtual AABB *getSensitiveAABB(int x, int y, int z);

public:
	virtual void onRemove(Level *level, int x, int y, int z, int id, int data);

protected:
	virtual void updateNeighbours(Level *level, int x, int y, int z);

public:
	virtual int getSignal(LevelSource *level, int x, int y, int z, int dir);
	virtual int getDirectSignal(LevelSource *level, int x, int y, int z, int dir);
	virtual bool isSignalSource();
	virtual void updateDefaultShape();
	virtual int getPistonPushReaction();

protected:
	virtual int getSignalStrength(Level *level, int x, int y, int z) = 0;
	virtual int getSignalForData(int data) = 0;
	virtual int getDataForSignal(int signal) = 0;

public:
	virtual void registerIcons(IconRegister *iconRegister);
};