#pragma once

#include "AbstractContainerMenu.h"

class CraftingContainer;
class Container;

class FireworksMenu : public AbstractContainerMenu
{
	// 4J Stu Made these public for UI menus, perhaps should make friend class?
public:
	static const int RESULT_SLOT = 0;
	static const int CRAFT_SLOT_START = 1;
	static const int CRAFT_SLOT_END = CRAFT_SLOT_START + 9;
	static const int INV_SLOT_START = CRAFT_SLOT_END;
	static const int INV_SLOT_END = INV_SLOT_START + (9*3);
	static const int USE_ROW_SLOT_START = INV_SLOT_END;
	static const int USE_ROW_SLOT_END = USE_ROW_SLOT_START + 9;

public:
	shared_ptr<CraftingContainer> craftSlots;
	shared_ptr<Container> resultSlots;

private:
	Level *level;
	int x, y, z;

	bool m_canMakeFireworks;
	bool m_canMakeCharge;
	bool m_canMakeFade;

public:
	FireworksMenu(shared_ptr<Inventory> inventory, Level *level, int xt, int yt, int zt);

	virtual void slotsChanged();// 4J used to take a shared_ptr<Container> but wasn't using it, so removed to simplify things
	virtual void removed(shared_ptr<Player> player);
	virtual bool stillValid(shared_ptr<Player> player);
	virtual shared_ptr<ItemInstance> quickMoveStack(shared_ptr<Player> player, int slotIndex);
	virtual bool canTakeItemForPickAll(shared_ptr<ItemInstance> carried, Slot *target);

	// 4J Added
	virtual bool isValidIngredient(shared_ptr<ItemInstance> item, int slotId);
};