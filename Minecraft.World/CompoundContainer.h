#pragma once
using namespace std;

#include "Container.h"

class Player;

class CompoundContainer : public Container
{
private:
	int name;
	shared_ptr<Container> c1, c2;

public:
	CompoundContainer(int name, shared_ptr<Container> c1, shared_ptr<Container> c2);

	unsigned int getContainerSize();

	int getName();

	shared_ptr<ItemInstance> getItem(unsigned int slot);

	shared_ptr<ItemInstance> removeItem(unsigned int slot, int i);
	shared_ptr<ItemInstance> removeItemNoUpdate(int slot);

	void setItem(unsigned int slot, shared_ptr<ItemInstance> item);

	int getMaxStackSize();

	void setChanged();

	bool stillValid(shared_ptr<Player> player);

	virtual void startOpen();
	virtual void stopOpen();
};