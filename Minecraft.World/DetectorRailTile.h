#pragma once
#include "RailTile.h"

class Entity;
class Random;
class Level;
class ChunkRebuildData;

class DetectorRailTile : public RailTile
{
	friend class ChunkRebuildData;
private:
	Icon **icons;

public:
	DetectorRailTile(int id);
	virtual int getTickDelay();
	virtual bool isSignalSource();
	virtual void entityInside(Level *level, int x, int y, int z, shared_ptr<Entity> entity);
	virtual void tick(Level *level, int x, int y, int z, Random *random);
	virtual bool getSignal(LevelSource *level, int x, int y, int z, int dir);
	virtual bool getDirectSignal(Level *level, int x, int y, int z, int facing);
private:
	virtual void checkPressed(Level *level, int x, int y, int z, int currentData);
public:
	//@Override
	void registerIcons(IconRegister *iconRegister);

	//@Override
	Icon *getTexture(int face, int data);
};
