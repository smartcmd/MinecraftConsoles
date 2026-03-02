#pragma once

#include "AbstractContainerMenu.h"

class CraftingContainer;
class Container;

class InventoryMenu : public AbstractContainerMenu
{
private:
	Player *owner;

	// 4J Stu Made these public for UI menus, perhaps should make friend class?
public:
	static const int RESULT_SLOT;
	static const int CRAFT_SLOT_START;
	static const int CRAFT_SLOT_END;
	static const int ARMOR_SLOT_START;
	static const int ARMOR_SLOT_END;
	static const int INV_SLOT_START;
	static const int INV_SLOT_END;
	static const int USE_ROW_SLOT_START;
	static const int USE_ROW_SLOT_END;

public:
	shared_ptr<CraftingContainer> craftSlots;
	shared_ptr<Container> resultSlots;
	bool active;

	InventoryMenu(shared_ptr<Inventory> inventory, bool active, Player *player);

private:
	void _init(shared_ptr<Inventory> inventory, bool active);

public:
	virtual void slotsChanged();	// 4J used to take a shared_ptr<Container> but wasn't using it, so removed to simplify things
	virtual void removed(shared_ptr<Player> player);
	virtual bool stillValid(shared_ptr<Player> player);
	virtual shared_ptr<ItemInstance> quickMoveStack(shared_ptr<Player> player, int slotIndex);
	virtual bool mayCombine(Slot *slot, shared_ptr<ItemInstance> item);

	// 4J ADDED,
	virtual shared_ptr<ItemInstance> clicked(int slotIndex, int buttonNum, int clickType, shared_ptr<Player> player);
};
