#pragma once

#include "AbstractContainerMenu.h"

class Container;

class ContainerMenu : public AbstractContainerMenu
{
private:
	shared_ptr<Container> container;
	int containerRows;

public:
	ContainerMenu(shared_ptr<Container> inventory, shared_ptr<Container> container);

	virtual bool stillValid(shared_ptr<Player> player);
	virtual shared_ptr<ItemInstance> quickMoveStack(shared_ptr<Player> player, int slotIndex);
	virtual void removed(shared_ptr<Player> player);
	virtual shared_ptr<Container> getContainer();

	// 4J ADDED,
	virtual shared_ptr<ItemInstance> clicked(int slotIndex, int buttonNum, int clickType, shared_ptr<Player> player, bool looped = false);
};
