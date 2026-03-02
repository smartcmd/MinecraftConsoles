#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "EnderChestTileEntity.h"

EnderChestTileEntity::EnderChestTileEntity()
{
	openness = oOpenness = 0.0f;
	openCount = 0;
	tickInterval = 0;
}

void EnderChestTileEntity::tick()
{
	TileEntity::tick();

	if (++tickInterval % 20 * 4 == 0)
	{
		level->tileEvent(x, y, z, Tile::enderChest_Id, ChestTile::EVENT_SET_OPEN_COUNT, openCount);
	}

	oOpenness = openness;

	float speed = 0.10f;
	if (openCount > 0 && openness == 0)
	{
		double xc = x + 0.5;
		double zc = z + 0.5;

		// 4J-PB - Seems the chest open volume is much louder than other sounds from user reports. We'll tone it down a bit		
		level->playSound(xc, y + 0.5, zc, eSoundType_RANDOM_CHEST_OPEN, 0.2f, level->random->nextFloat() * 0.1f + 0.9f);
	}
	if ((openCount == 0 && openness > 0) || (openCount > 0 && openness < 1))
	{
		float oldOpen = openness;
		if (openCount > 0) openness += speed;
		else openness -= speed;
		if (openness > 1)
		{
			openness = 1;
		}
		float lim = 0.5f;
		if (openness < lim && oldOpen >= lim)
		{
			double xc = x + 0.5;
			double zc = z + 0.5;

			// 4J-PB - Seems the chest open volume is much louder than other sounds from user reports. We'll tone it down a bit	
			level->playSound(xc, y + 0.5, zc, eSoundType_RANDOM_CHEST_CLOSE, 0.2f, level->random->nextFloat() * 0.1f + 0.9f);
		}
		if (openness < 0)
		{
			openness = 0;
		}
	}
}

void EnderChestTileEntity::triggerEvent(int b0, int b1)
{
	if (b0 == ChestTile::EVENT_SET_OPEN_COUNT)
	{
		openCount = b1;
	}
}

void EnderChestTileEntity::setRemoved()
{
	clearCache();
	TileEntity::setRemoved();
}

void EnderChestTileEntity::startOpen()
{
	openCount++;
	level->tileEvent(x, y, z, Tile::enderChest_Id, ChestTile::EVENT_SET_OPEN_COUNT, openCount);
}

void EnderChestTileEntity::stopOpen()
{
	openCount--;
	level->tileEvent(x, y, z, Tile::enderChest_Id, ChestTile::EVENT_SET_OPEN_COUNT, openCount);
}

bool EnderChestTileEntity::stillValid(shared_ptr<Player> player)
{
	if (level->getTileEntity(x, y, z) != shared_from_this()) return false;
	if (player->distanceToSqr(x + 0.5, y + 0.5, z + 0.5) > 8 * 8) return false;

	return true;
}

// 4J Added
shared_ptr<TileEntity> EnderChestTileEntity::clone()
{
	shared_ptr<EnderChestTileEntity> result = shared_ptr<EnderChestTileEntity>( new EnderChestTileEntity() );
	TileEntity::clone(result);

	return result;
}