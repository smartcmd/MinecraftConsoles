#pragma once

#include "AbstractContainerMenu.h"
#include "Slot.h"

class BrewingStandTileEntity;
class Inventory;
class Player;
class Container;

class BrewingStandMenu : public AbstractContainerMenu
{
	// 4J Stu - Made public so that we can access these from the XUI menus
public:
	static const int INGREDIENT_SLOT = 3;
	static const int BOTTLE_SLOT_START = 0;
	static const int BOTTLE_SLOT_END = 2;
	static const int INV_SLOT_START = INGREDIENT_SLOT + 1;
	static const int INV_SLOT_END = INV_SLOT_START + 9 * 3;
	static const int USE_ROW_SLOT_START = INV_SLOT_END;
	static const int USE_ROW_SLOT_END = USE_ROW_SLOT_START + 9;

private:
	shared_ptr<BrewingStandTileEntity> brewingStand;
	Slot *ingredientSlot;

public:
	BrewingStandMenu(shared_ptr<Inventory> inventory, shared_ptr<BrewingStandTileEntity> brewingStand);

private:
	int tc;

public:
	virtual void addSlotListener(ContainerListener *listener);
	virtual void broadcastChanges();
	virtual void setData(int id, int value);
	virtual bool stillValid(shared_ptr<Player> player);
	virtual shared_ptr<ItemInstance> quickMoveStack(shared_ptr<Player> player, int slotIndex);

private:
	class PotionSlot : public Slot
	{
	private:
		shared_ptr<Player> player;

	public:
		PotionSlot(shared_ptr<Player> player, shared_ptr<Container> container, int slot, int x, int y);

		virtual bool mayPlace(shared_ptr<ItemInstance> item);
		virtual int getMaxStackSize();
		virtual void onTake(shared_ptr<Player> player, shared_ptr<ItemInstance> carried);
		static bool mayPlaceItem(shared_ptr<ItemInstance> item);
		virtual bool mayCombine(shared_ptr<ItemInstance> item); // 4J Added
	};

	class IngredientsSlot : public Slot
	{
	public:
		IngredientsSlot(shared_ptr<Container> container, int slot, int x, int y);

		virtual bool mayPlace(shared_ptr<ItemInstance> item);
		virtual int getMaxStackSize();
		virtual bool mayCombine(shared_ptr<ItemInstance> item); // 4J Added
	};
};