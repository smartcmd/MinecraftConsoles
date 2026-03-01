#pragma once

#include "AbstractContainerMenu.h"

class CraftingContainer;
class Container;

class CraftingMenu : public AbstractContainerMenu
{
	// 4J Stu Made these public for UI menus, perhaps should make friend class?
public:
	static const int RESULT_SLOT;
	static const int CRAFT_SLOT_START;
	static const int CRAFT_SLOT_END;
	static const int INV_SLOT_START;
	static const int INV_SLOT_END;
	static const int USE_ROW_SLOT_START;
	static const int USE_ROW_SLOT_END;

public:
	shared_ptr<CraftingContainer> craftSlots;
	shared_ptr<Container> resultSlots;

private:
	Level *level;
	int x, y, z;

public:
	CraftingMenu(shared_ptr<Inventory> inventory, Level *level, int xt, int yt, int zt);

	virtual void slotsChanged();// 4J used to take a shared_ptr<Container> but wasn't using it, so removed to simplify things
	virtual void removed(shared_ptr<Player> player);
	virtual bool stillValid(shared_ptr<Player> player);
	virtual shared_ptr<ItemInstance> quickMoveStack(shared_ptr<Player> player, int slotIndex);
	virtual bool canTakeItemForPickAll(shared_ptr<ItemInstance> carried, Slot *target);

	int getX() { return x; }
	int getY() { return y; }
	int getZ() { return z; }
};