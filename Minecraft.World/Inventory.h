#pragma once
using namespace std;
#include "Container.h"
#include "ListTag.h"
#include "ItemInstance.h"

class Player;
class CompoundTag;

class Inventory : public Container
{
public:
	static const int POP_TIME_DURATION ;
	static const int MAX_INVENTORY_STACK_SIZE;

private:
	static const int INVENTORY_SIZE;
	static const int SELECTION_SIZE;

public:
	ItemInstanceArray items;
	ItemInstanceArray armor;

	int selected;
	Player *player; // This is owned by shared_ptrs, but we are owned by it

private:
	shared_ptr<ItemInstance> heldItem;
	shared_ptr<ItemInstance> carried;

public:
	bool changed;

	Inventory(Player *player);
	~Inventory();

	shared_ptr<ItemInstance> getSelected();
	// 4J-PB - Added for the in-game tooltips
	bool IsHeldItem();
	static int getSelectionSize();

private:
	int getSlot(int tileId);
	int getSlot(int tileId, int data);

	int getSlotWithRemainingSpace(shared_ptr<ItemInstance> item);

public:
	int getFreeSlot();
	void grabTexture(int id, int data, bool checkData, bool mayReplace);
	void swapPaint(int wheel);
	int clearInventory(int id, int data);
	void replaceSlot(Item *item, int data);

private:
	int addResource(shared_ptr<ItemInstance> itemInstance);

public:
	void tick();
	bool removeResource(int type);

	// 4J-PB added to get the right resource from the inventory for removal
	bool removeResource(int type,int iAuxVal);
	void removeResources(shared_ptr<ItemInstance> item); // 4J Added for trading

	// 4J-Stu added to the get the item that would be affected by the removeResource functions
	shared_ptr<ItemInstance> getResourceItem(int type);
	shared_ptr<ItemInstance> getResourceItem(int type,int iAuxVal);

	bool hasResource(int type);
	void swapSlots(int from, int to);
	bool add(shared_ptr<ItemInstance> item);
	shared_ptr<ItemInstance> removeItem(unsigned int slot, int count);
	virtual shared_ptr<ItemInstance> removeItemNoUpdate(int slot);
	void setItem(unsigned int slot, shared_ptr<ItemInstance> item);
	float getDestroySpeed(Tile *tile);
	ListTag<CompoundTag> *save(ListTag<CompoundTag> *listTag);
	void load(ListTag<CompoundTag> *inventoryList);
	unsigned int getContainerSize();
	shared_ptr<ItemInstance> getItem(unsigned int slot);
	wstring getName();
	wstring getCustomName();
	bool hasCustomName();
	int getMaxStackSize() const;
	bool canDestroy(Tile *tile);
	shared_ptr<ItemInstance> getArmor(int layer);
	int getArmorValue();
	void hurtArmor(float dmg);
	void dropAll();
	void setChanged();
	bool isSame(shared_ptr<Inventory> copy);

private:
	bool isSame(shared_ptr<ItemInstance> a, shared_ptr<ItemInstance> b);

public:
	shared_ptr<Inventory> copy();
	void setCarried(shared_ptr<ItemInstance> carried);
	shared_ptr<ItemInstance> getCarried();
	bool stillValid(shared_ptr<Player> player);
	bool contains(shared_ptr<ItemInstance> itemInstance);
	virtual void startOpen();
	virtual void stopOpen();
	bool canPlaceItem(int slot, shared_ptr<ItemInstance> item);
	void replaceWith(shared_ptr<Inventory> other);

	int countMatches(shared_ptr<ItemInstance> itemInstance); // 4J Added
};