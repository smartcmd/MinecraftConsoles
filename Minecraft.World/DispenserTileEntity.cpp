#include "stdafx.h"
#include "com.mojang.nbt.h"
#include "TileEntity.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "DispenserTileEntity.h"



DispenserTileEntity::DispenserTileEntity() : TileEntity()
{
	items = ItemInstanceArray(9);
	random = new Random();
	name = L"";
}

DispenserTileEntity::~DispenserTileEntity()
{
	delete[] items.data;

	delete random;
}

unsigned int DispenserTileEntity::getContainerSize()
{
	return 9;
}

shared_ptr<ItemInstance> DispenserTileEntity::getItem(unsigned int slot)
{
	return items[slot];
}

shared_ptr<ItemInstance> DispenserTileEntity::removeItem(unsigned int slot, int count)
{
	if (items[slot] != NULL)
	{
		if (items[slot]->count <= count)
		{
			shared_ptr<ItemInstance> item = items[slot];
			items[slot] = nullptr;
			setChanged();
			// 4J Stu - Fix for duplication glitch
			if(item->count <= 0) return nullptr;
			return item;
		}
		else
		{
			shared_ptr<ItemInstance> i = items[slot]->remove(count);
			if (items[slot]->count == 0) items[slot] = nullptr;
			setChanged();
			// 4J Stu - Fix for duplication glitch
			if(i->count <= 0) return nullptr;
			return i;
		}
	}
	return nullptr;
}

shared_ptr<ItemInstance> DispenserTileEntity::removeItemNoUpdate(int slot)
{
	if (items[slot] != NULL)
	{
		shared_ptr<ItemInstance> item = items[slot];
		items[slot] = nullptr;
		return item;
	}
	return nullptr;
}

// 4J-PB added for spawn eggs not being useable due to limits, so add them in again
void DispenserTileEntity::AddItemBack(shared_ptr<ItemInstance>item, unsigned int slot)
{
	if (items[slot] != NULL)
	{
		// just increment the count of the items
		if(item->id==items[slot]->id)
		{
			items[slot]->count++;
			setChanged();
		}
	}
	else
	{
		items[slot] = item;
		if (item != NULL && item->count > getMaxStackSize()) item->count = getMaxStackSize();
		setChanged();
	}
}
/**
* Removes an item with the given id and returns true if one was found.
*
* @param itemId
* @return
*/
bool DispenserTileEntity::removeProjectile(int itemId)
{
	for (unsigned int i = 0; i < items.length; i++)
	{
		if (items[i] != NULL && items[i]->id == itemId)
		{
			shared_ptr<ItemInstance> removedItem = removeItem(i, 1);
			return removedItem != NULL;
		}
	}
	return false;
}

int DispenserTileEntity::getRandomSlot()
{
	int replaceSlot = -1;
	int replaceOdds = 1;
	for (unsigned int i = 0; i < items.length; i++)
	{
		if (items[i] != NULL && random->nextInt(replaceOdds++) == 0)
		{
			replaceSlot = i;
		}
	}

	return replaceSlot;
}

void DispenserTileEntity::setItem(unsigned int slot, shared_ptr<ItemInstance> item)
{
	items[slot] = item;
	if (item != NULL && item->count > getMaxStackSize()) item->count = getMaxStackSize();
	setChanged();
}

int DispenserTileEntity::addItem(shared_ptr<ItemInstance> item)
{
	for (int i = 0; i < items.length; i++)
	{
		if (items[i] == NULL || items[i]->id == 0)
		{
			setItem(i, item);
			return i;
		}
	}

	return -1;
}

wstring DispenserTileEntity::getName()
{
	return hasCustomName() ? name : app.GetString(IDS_TILE_DISPENSER);
}

wstring DispenserTileEntity::getCustomName()
{
	return hasCustomName() ? name : L"";
}

void DispenserTileEntity::setCustomName(const wstring &name)
{
	this->name = name;
}

bool DispenserTileEntity::hasCustomName()
{
	return !name.empty();
}

void DispenserTileEntity::load(CompoundTag *base)
{
	TileEntity::load(base);
	ListTag<CompoundTag> *inventoryList = (ListTag<CompoundTag> *) base->getList(L"Items");
	delete [] items.data;
	items = ItemInstanceArray(getContainerSize());
	for (int i = 0; i < inventoryList->size(); i++)
	{
		CompoundTag *tag = inventoryList->get(i);
		unsigned int slot = tag->getByte(L"Slot") & 0xff;
		if (slot >= 0 && slot < items.length) items[slot] = ItemInstance::fromTag(tag);
	}
	if (base->contains(L"CustomName")) name = base->getString(L"CustomName");
}

void DispenserTileEntity::save(CompoundTag *base)
{
	TileEntity::save(base);
	ListTag<CompoundTag> *listTag = new ListTag<CompoundTag>;

	for (unsigned int i = 0; i < items.length; i++)
	{
		if (items[i] != NULL)
		{
			CompoundTag *tag = new CompoundTag();
			tag->putByte(L"Slot", (byte) i);
			items[i]->save(tag);
			listTag->add(tag);
		}
	}
	base->put(L"Items", listTag);
	if (hasCustomName()) base->putString(L"CustomName", name);
}

int DispenserTileEntity::getMaxStackSize() const
{
	return Container::LARGE_MAX_STACK_SIZE;
}

bool DispenserTileEntity::stillValid(shared_ptr<Player> player)
{
	if (level->getTileEntity(x, y, z) != shared_from_this() ) return false;
	if (player->distanceToSqr(x + 0.5, y + 0.5, z + 0.5) > 8 * 8) return false;
	return true;
}

void DispenserTileEntity::setChanged()
{
	return TileEntity::setChanged();
}

void DispenserTileEntity::startOpen()
{
}

void DispenserTileEntity::stopOpen()
{
}

bool DispenserTileEntity::canPlaceItem(int slot, shared_ptr<ItemInstance> item)
{
	return true;
}

// 4J Added
shared_ptr<TileEntity> DispenserTileEntity::clone()
{
	shared_ptr<DispenserTileEntity> result = shared_ptr<DispenserTileEntity>( new DispenserTileEntity() );
	TileEntity::clone(result);

	for (unsigned int i = 0; i < items.length; i++)
	{
		if (items[i] != NULL)
		{
			result->items[i] = ItemInstance::clone(items[i]);
		}
	}
	return result;
}