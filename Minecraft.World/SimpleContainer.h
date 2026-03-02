#pragma once
using namespace std;

#include "Container.h"
#include "net.minecraft.world.ContainerListener.h"

class SimpleContainer : public Container
{
private:
	int name;
	int size;
	ItemInstanceArray *items;
	vector<net_minecraft_world::ContainerListener *> *listeners;

public:
	SimpleContainer(int name, int size);

	void addListener(net_minecraft_world::ContainerListener *listener);

	void removeListener(net_minecraft_world::ContainerListener *listener);

	shared_ptr<ItemInstance> getItem(unsigned int slot);

	shared_ptr<ItemInstance> removeItem(unsigned int slot, int count);
	shared_ptr<ItemInstance> removeItemNoUpdate(int slot);

	void setItem(unsigned int slot, shared_ptr<ItemInstance> item);

	unsigned int getContainerSize();

	int getName();

	int getMaxStackSize();

	void setChanged();

	bool stillValid(shared_ptr<Player> player);

	void startOpen() { } // TODO Auto-generated method stub
	void stopOpen() { } // TODO Auto-generated method stub

};