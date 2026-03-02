#pragma once
#include "Tile.h"
#include "Direction.h"
class Level;
class LevelSource;

class VineTile : public Tile
{
public:
	static const int VINE_SOUTH = 1 << Direction::SOUTH;
	static const int VINE_NORTH = 1 << Direction::NORTH;
	static const int VINE_EAST = 1 << Direction::EAST;
	static const int VINE_WEST = 1 << Direction::WEST;

public:
	VineTile(int id);
    virtual void updateDefaultShape();
    virtual int getRenderShape();
    virtual bool isSolidRender(bool isServerLevel = false);
    virtual bool isCubeShaped();
    virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());	// 4J added forceData, forceEntity param
    virtual AABB *getAABB(Level *level, int x, int y, int z);
    virtual bool mayPlace(Level *level, int x, int y, int z, int face);
private:
	bool isAcceptableNeighbor(int id);
    bool updateSurvival(Level *level, int x, int y, int z);
public:
    virtual int getColor() const;
    virtual int getColor(int auxData);
	virtual int getColor(LevelSource *level, int x, int y, int z, int data);	// 4J added
    virtual int getColor(LevelSource *level, int x, int y, int z);
    virtual void neighborChanged(Level *level, int x, int y, int z, int type);
    virtual void tick(Level *level, int x, int y, int z, Random *random);
    virtual int getPlacedOnFaceDataValue(Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue);
    virtual int getResource(int data, Random *random, int playerBonusLevel);
    virtual int getResourceCount(Random *random);
    virtual void playerDestroy(Level *level, shared_ptr<Player> player, int x, int y, int z, int data);
};