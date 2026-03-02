#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "AbstractContainerMenu.h"
#include "CraftingContainer.h"

CraftingContainer::~CraftingContainer()
{

}

CraftingContainer::CraftingContainer(AbstractContainerMenu *menu, unsigned int w, unsigned int h)
{
	unsigned int size = w * h;
	items = new ItemInstanceArray(size);
	this->menu = menu;
	this->width = w;
}

unsigned int CraftingContainer::getContainerSize()
{
	return items->length;
}

shared_ptr<ItemInstance> CraftingContainer::getItem(unsigned int slot)
{
	if (slot >= getContainerSize())
	{
		return nullptr;
	}
	return (*items)[slot];
}

shared_ptr<ItemInstance> CraftingContainer::getItem(unsigned int x, unsigned int y)
{
	if (x < 0 || x >= width)
	{
		return nullptr;
	}
	unsigned int pos = x + y * width;
	return getItem(pos);
}

int CraftingContainer::getName()
{
	return 0;
}

shared_ptr<ItemInstance> CraftingContainer::removeItemNoUpdate(int slot)
{
	if ((*items)[slot] != NULL)
	{
		shared_ptr<ItemInstance> item = (*items)[slot];
		(*items)[slot] = nullptr;
		return item;
	}
	return nullptr;
}

shared_ptr<ItemInstance> CraftingContainer::removeItem(unsigned int slot, int count)
{
	if ((*items)[slot] != NULL)
	{
		if ((*items)[slot]->count <= count)
		{
			shared_ptr<ItemInstance> item = (*items)[slot];
			(*items)[slot] = nullptr;
			menu->slotsChanged();	// 4J - used to take pointer to this, but wasn't using it so removed
			return item;
		}
		else
		{
			shared_ptr<ItemInstance> i = (*items)[slot]->remove(count);
			if ((*items)[slot]->count == 0) (*items)[slot] = nullptr;
			menu->slotsChanged();		// 4J - used to take pointer to this, but wasn't using it so removed
			return i;
		}
	}
	return nullptr;
}

void CraftingContainer::setItem(unsigned int slot, shared_ptr<ItemInstance> item)
{
	(*items)[slot] = item;
	if(menu) menu->slotsChanged();
}

int CraftingContainer::getMaxStackSize()
{
	return Container::LARGE_MAX_STACK_SIZE;
}

void CraftingContainer::setChanged()
{
}

bool CraftingContainer::stillValid(shared_ptr<Player> player)
{
	return true;
}