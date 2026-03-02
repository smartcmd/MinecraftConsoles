#pragma once
#include "Tile.h"
#include "Definitions.h"

class Random;

class PressurePlateTile : public Tile
{
	friend class Tile;
private:
	wstring texture;
public:
    enum Sensitivity
	{
        everything,
		mobs,
		players
    };

private:
	Sensitivity sensitivity;
protected:
	PressurePlateTile(int id, const wstring &tex, Material *material, Sensitivity sensitivity);
public:
    virtual int getTickDelay();
    virtual AABB *getAABB(Level *level, int x, int y, int z);
    virtual bool isSolidRender(bool isServerLevel = false);
    virtual bool blocksLight();
    virtual bool isCubeShaped();
	virtual bool isPathfindable(LevelSource *level, int x, int y, int z);
    virtual bool mayPlace(Level *level, int x, int y, int z);
    virtual void neighborChanged(Level *level, int x, int y, int z, int type);
    virtual void tick(Level *level, int x, int y, int z, Random *random);
    virtual void entityInside(Level *level, int x, int y, int z, shared_ptr<Entity> entity);
private:
	virtual void checkPressed(Level *level, int x, int y, int z);
public:
    virtual void onRemove(Level *level, int x, int y, int z, int id, int data);
    virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());	// 4J added forceData, forceEntity param
    virtual bool getSignal(LevelSource *level, int x, int y, int z, int dir);
    virtual bool getDirectSignal(Level *level, int x, int y, int z, int dir);
    virtual bool isSignalSource();
    virtual void updateDefaultShape();
	virtual int getPistonPushReaction();
	void registerIcons(IconRegister *iconRegister);
	
	// 4J Added so we can check before we try to add a tile to the tick list if it's actually going to do seomthing
	virtual bool shouldTileTick(Level *level, int x,int y,int z);
};
