#pragma once

class EntityTile
{
public:
	virtual shared_ptr<TileEntity> newTileEntity(Level *level) = 0;
};