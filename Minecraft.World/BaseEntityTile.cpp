#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.h"
#include "TileEntity.h"
#include "BaseEntityTile.h"

BaseEntityTile::BaseEntityTile(int id, Material *material, bool isSolidRender /*= true*/) : Tile(id, material, isSolidRender)
{
	_isEntityTile = true;
}

void BaseEntityTile::onPlace(Level *level, int x, int y, int z)
{
	Tile::onPlace(level, x, y, z);
	//level->setTileEntity(x, y, z, newTileEntity(level));
}

void BaseEntityTile::onRemove(Level *level, int x, int y, int z, int id, int data)
{
	Tile::onRemove(level, x, y, z, id, data);
	level->removeTileEntity(x, y, z);
}

bool BaseEntityTile::triggerEvent(Level *level, int x, int y, int z, int b0, int b1)
{
	Tile::triggerEvent(level, x, y, z, b0, b1);
	shared_ptr<TileEntity> te = level->getTileEntity(x, y, z);
	if (te != nullptr)
	{
		return te->triggerEvent(b0, b1);
	}
	return false;
}