#include "stdafx.h"
#include "TheEndPortalTileEntity.h"

// 4J Added
shared_ptr<TileEntity> TheEndPortalTileEntity::clone()
{
	shared_ptr<TheEndPortalTileEntity> result = shared_ptr<TheEndPortalTileEntity>( new TheEndPortalTileEntity() );
	TileEntity::clone(result);
	return result;
}