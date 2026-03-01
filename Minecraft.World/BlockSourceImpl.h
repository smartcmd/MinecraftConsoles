#pragma once

#include "BlockSource.h"

class Level;

class BlockSourceImpl : public BlockSource
{
private:
	Level *world;
	int x;
	int y;
	int z;

public:
	BlockSourceImpl(Level *world, int x, int y, int z);

	Level *getWorld();
	double getX();
	double getY();
	double getZ();
	int getBlockX();
	int getBlockY();
	int getBlockZ();
	Tile *getType();
	int getData();
	Material *getMaterial();
	shared_ptr<TileEntity> getEntity();
};