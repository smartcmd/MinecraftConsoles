#pragma once

#include "LocatableSource.h"

class Tile;
class Material;
class TileEntity;

class BlockSource : public LocatableSource
{
public:
	/**
	* @return The X coordinate for the middle of the block
	*/
	virtual double getX() = 0;

	/**
	* @return The Y coordinate for the middle of the block
	*/
	virtual double getY() = 0;

	/**
	* @return The Z coordinate for the middle of the block
	*/
	virtual double getZ() = 0;

	virtual int getBlockX() = 0;
	virtual int getBlockY() = 0;
	virtual int getBlockZ() = 0;

	virtual Tile *getType() = 0;
	virtual int getData() = 0;
	virtual Material *getMaterial() = 0;

	virtual shared_ptr<TileEntity> getEntity() = 0;
};