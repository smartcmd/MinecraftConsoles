#pragma once
#include "Tile.h"
#include "Definitions.h"

class Player;
class Random;
class Level;

class ButtonTile : public Tile
{
	friend class Tile;

private:
	bool sensitive;

protected:
	ButtonTile(int id, bool sensitive);

public:
	Icon *getTexture(int face, int data);
	virtual AABB *getAABB(Level *level, int x, int y, int z);
	virtual int getTickDelay(Level *level);
	virtual bool blocksLight();
	virtual bool isSolidRender(bool isServerLevel = false);
	virtual bool isCubeShaped();
	virtual bool mayPlace(Level *level, int x, int y, int z, int face);
	virtual bool mayPlace(Level *level, int x, int y, int z);
	virtual int getPlacedOnFaceDataValue(Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue);

private:
	int findFace(Level *level, int x, int y, int z);

public:
	virtual void neighborChanged(Level *level, int x, int y, int z, int type);

private:
	bool checkCanSurvive(Level *level, int x, int y, int z);

public:
	virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());	// 4J added forceData, forceEntity param

private:
	void updateShape(int data);

public:
	virtual void attack(Level *level, int x, int y, int z, shared_ptr<Player> player);
	virtual bool TestUse();
	virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false); // 4J added soundOnly param
	virtual void onRemove(Level *level, int x, int y, int z, int id, int data);
	virtual int getSignal(LevelSource *level, int x, int y, int z, int dir);
	virtual int getDirectSignal(LevelSource *level, int x, int y, int z, int dir);
	virtual bool isSignalSource();
	virtual void tick(Level *level, int x, int y, int z, Random *random);
	virtual void updateDefaultShape();
	void entityInside(Level *level, int x, int y, int z, shared_ptr<Entity> entity);

private:
	void checkPressed(Level *level, int x, int y, int z);
	void updateNeighbours(Level *level, int x, int y, int z, int dir);

public:
	void registerIcons(IconRegister *iconRegister);

	// 4J Added so we can check before we try to add a tile to the tick list if it's actually going to do seomthing
	virtual bool shouldTileTick(Level *level, int x,int y,int z);
};