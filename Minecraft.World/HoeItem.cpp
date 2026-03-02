#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "ItemInstance.h"
#include "HoeItem.h"

HoeItem::HoeItem(int id, const Tier *tier) : Item(id)
{
	this->tier = tier;
	maxStackSize = 1;
	setMaxDamage(tier->getUses());
}

bool HoeItem::useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly)
{
	if (!player->mayBuild(x, y, z)) return false;

	// 4J-PB - Adding a test only version to allow tooltips to be displayed

	int targetType = level->getTile(x, y, z);
	//        Material above = level.getMaterial(x, y + 1, z);
	int above = level->getTile(x, y + 1, z);

	// 4J-PB - missing parentheses
	if (face != 0 && above == 0 && (targetType == Tile::grass_Id || targetType == Tile::dirt_Id)) 
	{
		if(!bTestUseOnOnly)
		{
			Tile *tile = Tile::farmland;
			level->playSound(x + 0.5f, y + 0.5f, z + 0.5f, tile->soundType->getStepSound(), (tile->soundType->getVolume() + 1) / 2, tile->soundType->getPitch() * 0.8f);

			if (level->isClientSide) return true;
			level->setTile(x, y, z, tile->id);
			instance->hurt(1, player);
		}
		return true;
	}

	return false;
}

bool HoeItem::isHandEquipped() 
{
	return true;
}

const Item::Tier *HoeItem::getTier()
{
	return tier;
}
