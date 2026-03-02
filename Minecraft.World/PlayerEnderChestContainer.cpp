#include "stdafx.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "ContainerOpenPacket.h"
#include "PlayerEnderChestContainer.h"

PlayerEnderChestContainer::PlayerEnderChestContainer() : SimpleContainer(IDS_TILE_ENDERCHEST, L"", false, 9 * 3)
{
	activeChest = nullptr;
}

int PlayerEnderChestContainer::getContainerType()
{
	return ContainerOpenPacket::ENDER_CHEST;
}

void PlayerEnderChestContainer::setActiveChest(shared_ptr<EnderChestTileEntity> activeChest)
{
	this->activeChest = activeChest;
}

void PlayerEnderChestContainer::setItemsByTag(ListTag<CompoundTag> *enderItemsList)
{
	for (int i = 0; i < getContainerSize(); i++)
	{
		setItem(i, nullptr);
	}
	for (int i = 0; i < enderItemsList->size(); i++)
	{
		CompoundTag *tag = enderItemsList->get(i);
		int slot = tag->getByte(L"Slot") & 0xff;
		if (slot >= 0 && slot < getContainerSize()) setItem(slot, ItemInstance::fromTag(tag));
	}
}

ListTag<CompoundTag> *PlayerEnderChestContainer::createTag()
{
	ListTag<CompoundTag> *items = new ListTag<CompoundTag>(L"EnderItems");
	for (int i = 0; i < getContainerSize(); i++)
	{
		shared_ptr<ItemInstance> item = getItem(i);
		if (item != NULL)
		{
			CompoundTag *tag = new CompoundTag();
			tag->putByte(L"Slot", (byte) i);
			item->save(tag);
			items->add(tag);
		}
	}
	return items;
}

bool PlayerEnderChestContainer::stillValid(shared_ptr<Player> player)
{
	if (activeChest != NULL && !activeChest->stillValid(player))
	{
		return false;
	}
	return SimpleContainer::stillValid(player);
}

void PlayerEnderChestContainer::startOpen()
{
	if (activeChest != NULL)
	{
		activeChest->startOpen();
	}
	SimpleContainer::startOpen();
}

void PlayerEnderChestContainer::stopOpen()
{
	if (activeChest)
	{
		activeChest->stopOpen();
	}
	SimpleContainer::stopOpen();
	activeChest = nullptr;
}

bool PlayerEnderChestContainer::canPlaceItem(int slot, shared_ptr<ItemInstance> item)
{
	return true;
}