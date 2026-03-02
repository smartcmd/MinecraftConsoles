#pragma once
#include "BaseRailTile.h"

class Entity;
class Random;
class Level;
class ChunkRebuildData;

class DetectorRailTile : public BaseRailTile
{
	friend class ChunkRebuildData;
private:
	Icon **icons;

public:
	DetectorRailTile(int id);
	virtual int getTickDelay(Level *level);
	virtual bool isSignalSource();
	virtual void entityInside(Level *level, int x, int y, int z, shared_ptr<Entity> entity);
	virtual void tick(Level *level, int x, int y, int z, Random *random);
	virtual int getSignal(LevelSource *level, int x, int y, int z, int dir);
	virtual int getDirectSignal(LevelSource *level, int x, int y, int z, int facing);
	virtual void onPlace(Level *level, int x, int y, int z);
	virtual bool hasAnalogOutputSignal();
	virtual int getAnalogOutputSignal(Level *level, int x, int y, int z, int dir);

private:
	virtual void checkPressed(Level *level, int x, int y, int z, int currentData);
public:
	//@Override
	void registerIcons(IconRegister *iconRegister);

	//@Override
	Icon *getTexture(int face, int data);
};
