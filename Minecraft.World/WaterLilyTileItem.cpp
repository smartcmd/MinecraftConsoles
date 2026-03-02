#include "stdafx.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.material.h"
#include "net.minecraft.world.entity.player.h"
#include "WaterLilyTileItem.h"

WaterLilyTileItem::WaterLilyTileItem(int id) : ColoredTileItem(id, false)
{
}

bool WaterLilyTileItem::TestUse(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
	HitResult *hr = getPlayerPOVHitResult(level, player, true);
	if (hr == NULL) return false;

	if (hr->type == HitResult::TILE)
	{
		int xt = hr->x;
		int yt = hr->y;
		int zt = hr->z;
		if (!level->mayInteract(player, xt, yt, zt, 0))
		{
			delete hr;
			return false;
		}
		if (!player->mayUseItemAt(xt, yt, zt, hr->f, itemInstance))
		{			
			delete hr;
			return false;
		}
		
		delete hr;
		if (level->getMaterial(xt, yt, zt) == Material::water && level->getData(xt, yt, zt) == 0 && level->isEmptyTile(xt, yt + 1, zt))
		{
			return true;
		}
	}
	else
	{
		delete hr;
	}
	return false;
}

shared_ptr<ItemInstance> WaterLilyTileItem::use(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
	HitResult *hr = getPlayerPOVHitResult(level, player, true);
	if (hr == NULL) return itemInstance;

	if (hr->type == HitResult::TILE)
	{
		int xt = hr->x;
		int yt = hr->y;
		int zt = hr->z;
		if (!level->mayInteract(player, xt, yt, zt, 0))
		{
		delete hr;
			return itemInstance;
		}
		if (!player->mayUseItemAt(xt, yt, zt, hr->f, itemInstance))
		{			
			delete hr;
			return itemInstance;
		}
		
		delete hr;
		if (level->getMaterial(xt, yt, zt) == Material::water && level->getData(xt, yt, zt) == 0 && level->isEmptyTile(xt, yt + 1, zt))
		{
			level->setTileAndUpdate(xt, yt + 1, zt, Tile::waterLily->id);
			if (!player->abilities.instabuild)
			{
				itemInstance->count--;
			}
		}
	}
	else
	{
		delete hr;
	}
	return itemInstance;
}

int WaterLilyTileItem::getColor(int data, int spriteLayer)
{
	return Tile::waterLily->getColor(data);
}