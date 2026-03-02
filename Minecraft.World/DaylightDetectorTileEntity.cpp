#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "DaylightDetectorTileEntity.h"

DaylightDetectorTileEntity::DaylightDetectorTileEntity()
{
}

void DaylightDetectorTileEntity::tick()
{
	if (level != NULL && !level->isClientSide && (level->getGameTime() % SharedConstants::TICKS_PER_SECOND) == 0)
	{
		tile = getTile();
		if (tile != NULL && dynamic_cast<DaylightDetectorTile *>(tile) != NULL)
		{
			((DaylightDetectorTile *) tile)->updateSignalStrength(level, x, y, z);
		}
	}
}

// 4J Added
shared_ptr<TileEntity> DaylightDetectorTileEntity::clone()
{
	shared_ptr<DaylightDetectorTileEntity> result = shared_ptr<DaylightDetectorTileEntity>( new DaylightDetectorTileEntity() );
	TileEntity::clone(result);

	return result;
}