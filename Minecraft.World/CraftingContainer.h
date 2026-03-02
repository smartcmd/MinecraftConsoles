#pragma once
using namespace std;
#include "Container.h"

class AbstractContainerMenu;

class CraftingContainer : public Container
{
private:
	ItemInstanceArray *items;
	unsigned int width;
	AbstractContainerMenu *menu;

public:
	CraftingContainer(AbstractContainerMenu *menu, unsigned int w, unsigned int h);
	~CraftingContainer();

	virtual unsigned int getContainerSize();
	virtual shared_ptr<ItemInstance> getItem(unsigned int slot);
	shared_ptr<ItemInstance> getItem(unsigned int x, unsigned int y);
	virtual int getName();
	virtual shared_ptr<ItemInstance> removeItemNoUpdate(int slot);
	virtual shared_ptr<ItemInstance> removeItem(unsigned int slot, int count);
	virtual void setItem(unsigned int slot, shared_ptr<ItemInstance> item);
	virtual int getMaxStackSize();
	virtual void setChanged();
	bool stillValid(shared_ptr<Player> player);

	void startOpen() { } // TODO Auto-generated method stub
	void stopOpen() { } // TODO Auto-generated method stub

};