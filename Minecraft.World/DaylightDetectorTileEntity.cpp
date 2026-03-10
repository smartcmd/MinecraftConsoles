#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "DaylightDetectorTileEntity.h"

DaylightDetectorTileEntity::DaylightDetectorTileEntity()
{
}

void DaylightDetectorTileEntity::tick()
{
	if (level != nullptr && !level->isClientSide && (level->getGameTime() % SharedConstants::TICKS_PER_SECOND) == 0)
	{
		tile = getTile();
		if (tile != nullptr && dynamic_cast<DaylightDetectorTile *>(tile) != nullptr)
		{
			static_cast<DaylightDetectorTile *>(tile)->updateSignalStrength(level, x, y, z);
		}
	}
}

// 4J Added
shared_ptr<TileEntity> DaylightDetectorTileEntity::clone()
{
	shared_ptr<DaylightDetectorTileEntity> result = std::make_shared<DaylightDetectorTileEntity>();
	TileEntity::clone(result);

	return result;
}