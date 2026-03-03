#pragma once

#include "Container.h"

class ResultContainer : public Container
{
private:
	shared_ptr<ItemInstance> items[1];

public:
	// 4J Stu Added a ctor to init items
	ResultContainer();

	virtual unsigned int getContainerSize();
	virtual shared_ptr<ItemInstance> getItem(unsigned int slot);
	virtual wstring getName();
	virtual wstring getCustomName();
	virtual bool hasCustomName();
	virtual shared_ptr<ItemInstance> removeItem(unsigned int slot, int count);
	virtual shared_ptr<ItemInstance> removeItemNoUpdate(int slot);
	virtual void setItem(unsigned int slot, shared_ptr<ItemInstance> item);
	virtual int getMaxStackSize() const;
	virtual void setChanged();
	virtual bool stillValid(shared_ptr<Player> player);
	virtual void startOpen() { } // TODO Auto-generated method stub
	virtual void stopOpen() { } // TODO Auto-generated method stub
	virtual bool canPlaceItem(int slot, shared_ptr<ItemInstance> item);
};