#pragma once

#include "AbstractContainerMenu.h"

class DispenserTileEntity;

class TrapMenu : public AbstractContainerMenu
{
private:
	static const int INV_SLOT_START = 9;
	static const int INV_SLOT_END = INV_SLOT_START + 9 * 3;
	static const int USE_ROW_SLOT_START = INV_SLOT_END;
	static const int USE_ROW_SLOT_END = USE_ROW_SLOT_START + 9;
private:
	shared_ptr<DispenserTileEntity> trap;

public:
	TrapMenu(shared_ptr<Container> inventory, shared_ptr<DispenserTileEntity> trap);

	virtual bool stillValid(shared_ptr<Player> player);
	virtual shared_ptr<ItemInstance> quickMoveStack(shared_ptr<Player> player, int slotIndex);
};