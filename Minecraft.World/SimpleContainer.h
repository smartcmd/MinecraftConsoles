#pragma once
using namespace std;

#include "Container.h"
#include "net.minecraft.world.ContainerListener.h"

class SimpleContainer : public Container
{
private:
	int name;
	wstring stringName;
	int size;
	ItemInstanceArray *items;
	vector<net_minecraft_world::ContainerListener *> *listeners;
	bool customName;

public:
	SimpleContainer(int name, wstring stringName, bool customName, int size);

	virtual void addListener(net_minecraft_world::ContainerListener *listener);
	virtual void removeListener(net_minecraft_world::ContainerListener *listener);
	virtual shared_ptr<ItemInstance> getItem(unsigned int slot);
	virtual shared_ptr<ItemInstance> removeItem(unsigned int slot, int count);
	virtual shared_ptr<ItemInstance> removeItemNoUpdate(int slot);
	virtual void setItem(unsigned int slot, shared_ptr<ItemInstance> item);
	virtual unsigned int getContainerSize();
	virtual wstring getName();
	virtual wstring getCustomName();
	virtual bool hasCustomName();
	virtual void setCustomName(const wstring &name);
	virtual int getMaxStackSize() const;
	virtual void setChanged();
	virtual bool stillValid(shared_ptr<Player> player);
	virtual void startOpen() { } // TODO Auto-generated method stub
	virtual void stopOpen() { } // TODO Auto-generated method stub
	virtual bool canPlaceItem(int slot, shared_ptr<ItemInstance> item);
};