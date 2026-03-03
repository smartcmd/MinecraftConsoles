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

	virtual int getContainerType();
	virtual unsigned int getContainerSize();
	virtual bool contains(shared_ptr<Container> c);
	virtual wstring getName();
	virtual wstring getCustomName();
	virtual bool hasCustomName();
	virtual shared_ptr<ItemInstance> getItem(unsigned int slot);
	virtual shared_ptr<ItemInstance> removeItem(unsigned int slot, int i);
	virtual shared_ptr<ItemInstance> removeItemNoUpdate(int slot);
	virtual void setItem(unsigned int slot, shared_ptr<ItemInstance> item);
	virtual int getMaxStackSize() const;
	virtual void setChanged();
	virtual bool stillValid(shared_ptr<Player> player);

	virtual void startOpen();
	virtual void stopOpen();
	virtual bool canPlaceItem(int slot, shared_ptr<ItemInstance> item);
};