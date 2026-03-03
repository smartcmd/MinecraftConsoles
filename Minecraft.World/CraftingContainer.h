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
	virtual wstring getName();
	virtual wstring getCustomName();
	virtual bool hasCustomName();
	virtual shared_ptr<ItemInstance> removeItemNoUpdate(int slot);
	virtual shared_ptr<ItemInstance> removeItem(unsigned int slot, int count);
	virtual void setItem(unsigned int slot, shared_ptr<ItemInstance> item);
	virtual int getMaxStackSize() const;
	virtual void setChanged();
	bool stillValid(shared_ptr<Player> player);

	void startOpen() { } // TODO Auto-generated method stub
	void stopOpen() { } // TODO Auto-generated method stub

	virtual bool canPlaceItem(int slot, shared_ptr<ItemInstance> item);
};