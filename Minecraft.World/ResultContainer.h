#pragma once

#include "Container.h"

class ResultContainer : public Container
{
private:
	ItemInstanceArray *items;

public:
	// 4J Stu Added a ctor to init items
	ResultContainer();

	virtual unsigned int getContainerSize();
	virtual shared_ptr<ItemInstance> getItem(unsigned int slot);
	virtual int getName();
	virtual shared_ptr<ItemInstance> removeItem(unsigned int slot, int count);
	virtual shared_ptr<ItemInstance> removeItemNoUpdate(int slot);
	virtual void setItem(unsigned int slot, shared_ptr<ItemInstance> item);
	virtual int getMaxStackSize();
	virtual void setChanged();
	virtual bool stillValid(shared_ptr<Player> player);

	void startOpen() { } // TODO Auto-generated method stub
	void stopOpen() { } // TODO Auto-generated method stub
};