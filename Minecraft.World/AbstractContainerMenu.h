#pragma once
using namespace std;

#include "Player.h"
#include "net.minecraft.world.inventory.ContainerListener.h"
using net_minecraft_world_inventory::ContainerListener;

class Inventory;
class Slot;
class Item;
class ItemInstance;
class Container;

class AbstractContainerMenu
{
public:
	static const int SLOT_CLICKED_OUTSIDE = -999;

	static const int CLICK_PICKUP = 0;
	static const int CLICK_QUICK_MOVE = 1;
	static const int CLICK_SWAP = 2;
	static const int CLICK_CLONE = 3;
	static const int CLICK_THROW = 4;
	static const int CLICK_QUICK_CRAFT = 5;
	static const int CLICK_PICKUP_ALL = 6;

	static const int QUICKCRAFT_TYPE_CHARITABLE = 0;
	static const int QUICKCRAFT_TYPE_GREEDY = 1;
	static const int QUICKCRAFT_HEADER_START = 0;
	static const int QUICKCRAFT_HEADER_CONTINUE = 1;
	static const int QUICKCRAFT_HEADER_END = 2;

	// 4J Stu - Added these to fix problem with items picked up while in the creative menu replacing slots in the creative menu
	static const int CONTAINER_ID_CARRIED = -1;
	static const int CONTAINER_ID_INVENTORY = 0;
	static const int CONTAINER_ID_CREATIVE = -2;

	vector<shared_ptr<ItemInstance> > lastSlots;
	vector<Slot *> slots;
	int containerId;

private:
	short changeUid;

	int quickcraftType;
	int quickcraftStatus;
	unordered_set<Slot *> quickcraftSlots;

private:
	bool m_bNeedsRendered; // 4J added

protected:
	vector<ContainerListener *> containerListeners;

	// 4J Stu - The java does not have ctor here (being an abstract) but we need one to initialise the member variables
	// TODO Make sure all derived classes also call this
	AbstractContainerMenu();

	Slot *addSlot(Slot *slot);

public:
	virtual ~AbstractContainerMenu();
	virtual void addSlotListener(ContainerListener *listener);
	virtual void removeSlotListener(ContainerListener *listener);
	virtual vector<shared_ptr<ItemInstance> > *getItems();
	virtual void sendData(int id, int value);
	virtual void broadcastChanges();
	virtual bool needsRendered();
	virtual bool clickMenuButton(shared_ptr<Player> player, int buttonId);
	virtual Slot *getSlotFor(shared_ptr<Container> c, int index);
	virtual Slot *getSlot(int index);
	virtual shared_ptr<ItemInstance> quickMoveStack(shared_ptr<Player> player, int slotIndex);
	virtual shared_ptr<ItemInstance> clicked(int slotIndex, int buttonNum, int clickType, shared_ptr<Player> player, bool looped = false); // 4J added looped param
	virtual bool mayCombine(Slot *slot, shared_ptr<ItemInstance> item);
	virtual bool canTakeItemForPickAll(shared_ptr<ItemInstance> carried, Slot *target);

protected:
	virtual void loopClick(int slotIndex, int buttonNum, bool quickKeyHeld, shared_ptr<Player> player);

public:
	virtual void removed(shared_ptr<Player> player);
	virtual void slotsChanged();// 4J used to take a shared_ptr<Container> container but wasn't using it, so removed to simplify things
	bool isPauseScreen();
	void setItem(unsigned int slot, shared_ptr<ItemInstance> item);
	void setAll(ItemInstanceArray *items);
	virtual void setData(int id, int value);
	short backup(shared_ptr<Inventory> inventory);

private:
	unordered_set<shared_ptr<Player> , PlayerKeyHash, PlayerKeyEq> unSynchedPlayers;

public:
	bool isSynched(shared_ptr<Player> player);
	void setSynched(shared_ptr<Player> player, bool synched);
	virtual bool stillValid(shared_ptr<Player> player) = 0;

	// 4J Stu Added for UI
	unsigned int getSize() { return (unsigned int)slots.size(); }


protected:
	// 4J Stu - Changes to return bool brought forward from 1.2
	bool moveItemStackTo(shared_ptr<ItemInstance> itemStack, int startSlot, int endSlot, bool backwards);

public:
	virtual bool isOverrideResultClick(int slotNum, int buttonNum);

	static int getQuickcraftType(int mask);
	static int getQuickcraftHeader(int mask);
	static int getQuickcraftMask(int header, int type);
	static bool isValidQuickcraftType(int type);

protected:
	void resetQuickCraft();

public:
	static bool canItemQuickReplace(Slot *slot, shared_ptr<ItemInstance> item, bool ignoreSize);
	static void getQuickCraftSlotCount(unordered_set<Slot *> *quickCraftSlots, int quickCraftingType, shared_ptr<ItemInstance> item, int carry);
	bool canDragTo(Slot *slot);
	static int getRedstoneSignalFromContainer(shared_ptr<Container> container);

	// 4J Added
	virtual bool isValidIngredient(shared_ptr<ItemInstance> item, int slotId);
};
