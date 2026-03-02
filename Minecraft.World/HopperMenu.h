#pragma once

#include "AbstractContainerMenu.h"

class HopperMenu : public AbstractContainerMenu
{
private:
	shared_ptr<Container> hopper;

public:
	static const int CONTENTS_SLOT_START = 0;
	static const int INV_SLOT_START = CONTENTS_SLOT_START + 5;
	static const int INV_SLOT_END = INV_SLOT_START + 9 * 3;
	static const int USE_ROW_SLOT_START = INV_SLOT_END;
	static const int USE_ROW_SLOT_END = USE_ROW_SLOT_START + 9;

public:
	HopperMenu(shared_ptr<Container> inventory, shared_ptr<Container> hopper);

	bool stillValid(shared_ptr<Player> player);
	shared_ptr<ItemInstance> quickMoveStack(shared_ptr<Player> player, int slotIndex);
	void removed(shared_ptr<Player> player);
	shared_ptr<Container> getContainer();
};