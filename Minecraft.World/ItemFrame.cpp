#include "stdafx.h"

#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.saveddata.h"
#include "com.mojang.nbt.h"
#include "ItemFrame.h"




// 4J - added for common ctor code
void ItemFrame::_init()
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();
}

ItemFrame::ItemFrame(Level *level) : HangingEntity( level )
{
	_init();
}

ItemFrame::ItemFrame(Level *level, int xTile, int yTile, int zTile, int dir) : HangingEntity( level, xTile, yTile, zTile, dir )
{
	_init();
	setDir(dir);
}

void ItemFrame::defineSynchedData() 
{
	getEntityData()->defineNULL(DATA_ITEM, NULL);
	getEntityData()->define(DATA_ROTATION, (byte) 0);
}

void ItemFrame::dropItem() 
{
	spawnAtLocation(shared_ptr<ItemInstance>(new ItemInstance(Item::frame)), 0.0f);
	shared_ptr<ItemInstance> item = getItem();
	if (item != NULL) 
	{
		shared_ptr<MapItemSavedData> data = Item::map->getSavedData(item, level);
		data->removeItemFrameDecoration(item);

		shared_ptr<ItemInstance> itemToDrop = item->copy();
		itemToDrop->setFramed(nullptr);
		spawnAtLocation(itemToDrop, 0.0f);
	}
}

shared_ptr<ItemInstance> ItemFrame::getItem() 
{
	return getEntityData()->getItemInstance(DATA_ITEM);
}

void ItemFrame::setItem(shared_ptr<ItemInstance> item) 
{
	if(item != NULL)
	{
		item = item->copy();
		item->count = 1;

		item->setFramed(dynamic_pointer_cast<ItemFrame>( shared_from_this() ));
	}
	getEntityData()->set(DATA_ITEM, item);
	getEntityData()->markDirty(DATA_ITEM);
}

int ItemFrame::getRotation() 
{
	return getEntityData()->getByte(DATA_ROTATION);
}

void ItemFrame::setRotation(int rotation) 
{
	getEntityData()->set(DATA_ROTATION, (byte) (rotation % 4));
}

void ItemFrame::addAdditonalSaveData(CompoundTag *tag) 
{
	if (getItem() != NULL) 
	{
		tag->putCompound(L"Item", getItem()->save(new CompoundTag()));
		tag->putByte(L"ItemRotation", (byte) getRotation());
		//tag->putFloat(L"ItemDropChance", dropChance);
	}
	HangingEntity::addAdditonalSaveData(tag);
}

void ItemFrame::readAdditionalSaveData(CompoundTag *tag) 
{
	CompoundTag *itemTag = tag->getCompound(L"Item");
	if (itemTag != NULL && !itemTag->isEmpty()) 
	{
		setItem(ItemInstance::fromTag(itemTag));
		setRotation(tag->getByte(L"ItemRotation"));

		//if (tag->contains(L"ItemDropChance")) dropChance = tag->getFloat(L"ItemDropChance");
	}
	HangingEntity::readAdditionalSaveData(tag);
}

bool ItemFrame::interact(shared_ptr<Player> player) 
{
	if(!player->isAllowedToInteract(shared_from_this()))
	{
		return false;
	}

	if (getItem() == NULL) 
	{
		shared_ptr<ItemInstance> item = player->getCarriedItem();

		if (item != NULL) 
		{
			if (!level->isClientSide)//isClientSide) 
			{
				setItem(item);

				if (!player->abilities.instabuild) 
				{
 					if (--item->count <= 0) 
 					{
 						player->inventory->setItem(player->inventory->selected, nullptr);
 					}
				}
			}
		}
	} 
	else 
	{
		if (!level->isClientSide)//isClientSide) 
		{
			setRotation(getRotation() + 1);
		}
	}

	return true;
}
