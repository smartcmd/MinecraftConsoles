#include "stdafx.h"

#include "DropperTileEntity.h"

wstring DropperTileEntity::getName()
{
	return hasCustomName() ? name : app.GetString(IDS_CONTAINER_DROPPER);
}

// 4J Added
shared_ptr<TileEntity> DropperTileEntity::clone()
{
	shared_ptr<DropperTileEntity> result = shared_ptr<DropperTileEntity>( new DropperTileEntity() );
	TileEntity::clone(result);

	result->name = name;

	return result;
}