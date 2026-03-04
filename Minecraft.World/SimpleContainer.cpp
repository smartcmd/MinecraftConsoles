#include "stdafx.h"

#include "net.minecraft.world.ContainerListener.h"
#include "net.minecraft.world.item.h"

#include "SimpleContainer.h"

SimpleContainer::SimpleContainer(int name, wstring stringName, bool customName, int size)
{
	this->name = name;
	this->stringName = stringName;
	this->customName  = customName;
	this->size = size;
	items = new ItemInstanceArray( size );

	listeners = NULL;
}

void SimpleContainer::addListener(net_minecraft_world::ContainerListener *listener)
{
	if (listeners == NULL) listeners = new vector<net_minecraft_world::ContainerListener *>();
	listeners->push_back(listener);
}

void SimpleContainer::removeListener(net_minecraft_world::ContainerListener *listener)
{
	// 4J Java has a remove function on lists that will find the first occurence of
	// an object and remove it. We need to replicate that ourselves

	vector<net_minecraft_world::ContainerListener *>::iterator it = listeners->begin();
	vector<net_minecraft_world::ContainerListener *>::iterator itEnd = listeners->end();
	while( it != itEnd && *it != listener )
		it++;

	if( it != itEnd )
		listeners->erase( it );
}

shared_ptr<ItemInstance> SimpleContainer::getItem(unsigned int slot)
{
	return (*items)[slot];
}

shared_ptr<ItemInstance> SimpleContainer::removeItem(unsigned int slot, int count)
{
	if ((*items)[slot] != NULL) {
		if ((*items)[slot]->count <= count)
		{
			shared_ptr<ItemInstance> item = (*items)[slot];
			(*items)[slot] = nullptr;
			setChanged();
			return item;
		}
		else
		{
			shared_ptr<ItemInstance> i = (*items)[slot]->remove(count);
			if ((*items)[slot]->count == 0) (*items)[slot] = nullptr;
			setChanged();
			return i;
		}
	}
	return nullptr;
}

shared_ptr<ItemInstance> SimpleContainer::removeItemNoUpdate(int slot)
{
	if ((*items)[slot] != NULL)
	{
		shared_ptr<ItemInstance> item = (*items)[slot];
		(*items)[slot] = nullptr;
		return item;
	}
	return nullptr;
}

void SimpleContainer::setItem(unsigned int slot, shared_ptr<ItemInstance> item)
{
	(*items)[slot] = item;
	if (item != NULL && item->count > getMaxStackSize()) item->count = getMaxStackSize();
	setChanged();
}

unsigned int SimpleContainer::getContainerSize()
{
	return size;
}

wstring SimpleContainer::getName()
{
	return stringName.empty() ? app.GetString(name) : stringName;
}

wstring SimpleContainer::getCustomName()
{
	return hasCustomName() ? stringName : L"";
}

bool SimpleContainer::hasCustomName()
{
	return customName;
}

void SimpleContainer::setCustomName(const wstring &name)
{
	customName = true;
	this->stringName = name;
}

int SimpleContainer::getMaxStackSize() const
{
	return Container::LARGE_MAX_STACK_SIZE;
}

void SimpleContainer::setChanged()
{
	if (listeners != NULL) for (unsigned int i = 0; i < listeners->size(); i++)
	{
		listeners->at(i)->containerChanged();//shared_from_this());
	}
}

bool SimpleContainer::stillValid(shared_ptr<Player> player)
{
	return true;
}

bool SimpleContainer::canPlaceItem(int slot, shared_ptr<ItemInstance> item)
{
	return true;
}