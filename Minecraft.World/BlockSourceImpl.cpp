#include "stdafx.h"
#include "BlockSourceImpl.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.tile.entity.h"

BlockSourceImpl::BlockSourceImpl(Level *world, int x, int y, int z)
{
	this->world = world;
	this->x = x;
	this->y = y;
	this->z = z;
}

Level *BlockSourceImpl::getWorld()
{
	return world;
}

double BlockSourceImpl::getX()
{
	return x + 0.5;
}

double BlockSourceImpl::getY()
{
	return y + 0.5;
}

double BlockSourceImpl::getZ()
{
	return z + 0.5;
}

int BlockSourceImpl::getBlockX()
{
	return x;
}

int BlockSourceImpl::getBlockY()
{
	return y;
}

int BlockSourceImpl::getBlockZ()
{
	return z;
}

Tile *BlockSourceImpl::getType()
{
	return Tile::tiles[world->getTile(x, y, z)];
}

int BlockSourceImpl::getData()
{
	return world->getData(x, y, z);
}

Material *BlockSourceImpl::getMaterial()
{
	return world->getMaterial(x, y, z);
}

shared_ptr<TileEntity> BlockSourceImpl::getEntity()
{
	return world->getTileEntity(x, y, z);
}