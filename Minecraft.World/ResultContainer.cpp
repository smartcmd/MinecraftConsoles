#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "ResultContainer.h"

ResultContainer::ResultContainer() : Container()
{
	items = new ItemInstanceArray(1);
}

unsigned int ResultContainer::getContainerSize()
{
	return 1;
}

shared_ptr<ItemInstance> ResultContainer::getItem(unsigned int slot)
{
	return (*items)[0];
}

int ResultContainer::getName()
{
	return 0;
}

shared_ptr<ItemInstance> ResultContainer::removeItem(unsigned int slot, int count)
{
	if ((*items)[0] != NULL)
	{
		shared_ptr<ItemInstance> item = (*items)[0];
		(*items)[0] = nullptr;
		return item;
	}
	return nullptr;
}

shared_ptr<ItemInstance> ResultContainer::removeItemNoUpdate(int slot)
{
	if ((*items)[0] != NULL)
	{
		shared_ptr<ItemInstance> item = (*items)[0];
		(*items)[0] = nullptr;
		return item;
	}
	return nullptr;
}

void ResultContainer::setItem(unsigned int slot, shared_ptr<ItemInstance> item)
{
	(*items)[0] = item;
}

int ResultContainer::getMaxStackSize()
{
	return Container::LARGE_MAX_STACK_SIZE;
}

void ResultContainer::setChanged()
{
}

bool ResultContainer::stillValid(shared_ptr<Player> player)
{
	return true;
}