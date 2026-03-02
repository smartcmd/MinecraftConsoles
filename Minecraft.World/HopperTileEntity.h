#pragma once

#include "TileEntity.h"
#include "Hopper.h"

class HopperTileEntity : public TileEntity, public Hopper
{
public:
	eINSTANCEOF GetType() { return eTYPE_HOPPERTILEENTITY; }
	static TileEntity *create() { return new HopperTileEntity(); }
	// 4J Added
	virtual shared_ptr<TileEntity> clone();

public:
	static const int MOVE_ITEM_SPEED = 8;

private:
	ItemInstanceArray items;
	wstring name;
	int cooldownTime ;

public:
	HopperTileEntity();
	~HopperTileEntity();

	virtual void load(CompoundTag *base);
	virtual void save(CompoundTag *base);
	virtual void setChanged();
	virtual unsigned int getContainerSize();
	virtual shared_ptr<ItemInstance> getItem(unsigned int slot);
	virtual shared_ptr<ItemInstance> removeItem(unsigned int slot, int count);
	virtual shared_ptr<ItemInstance> removeItemNoUpdate(int slot);
	virtual void setItem(unsigned int slot, shared_ptr<ItemInstance> item);
	virtual wstring getName();
	virtual wstring getCustomName();
	virtual bool hasCustomName();
	virtual void setCustomName(const wstring &name);
	virtual int getMaxStackSize() const;
	virtual bool stillValid(shared_ptr<Player> player);
	virtual void startOpen();
	virtual void stopOpen();
	virtual bool canPlaceItem(int slot, shared_ptr<ItemInstance> item);
	virtual void tick();
	virtual bool tryMoveItems();

private:
	virtual bool ejectItems();

public:
	static bool suckInItems(Hopper *hopper);

private:
	static bool tryTakeInItemFromSlot(Hopper *hopper, Container *container, int slot, int face);

public:
	static bool addItem(Container *container, shared_ptr<ItemEntity> item);
	static shared_ptr<ItemInstance> addItem(Container *container, shared_ptr<ItemInstance> item, int face);

private:
	static bool canPlaceItemInContainer(Container *container, shared_ptr<ItemInstance> item, int slot, int face);
	static bool canTakeItemFromContainer(Container *container, shared_ptr<ItemInstance> item, int slot, int face);
	static shared_ptr<ItemInstance> tryMoveInItem(Container *container, shared_ptr<ItemInstance> item, int slot, int face);
	virtual shared_ptr<Container> getAttachedContainer();

public:
	static shared_ptr<Container> getSourceContainer(Hopper *hopper);
	static shared_ptr<ItemEntity> getItemAt(Level *level, double xt, double yt, double zt);
	static shared_ptr<Container> getContainerAt(Level *level, double x, double y, double z);

private:
	static bool canMergeItems(shared_ptr<ItemInstance> a, shared_ptr<ItemInstance> b);

public:
	virtual Level *getLevel();
	virtual double getLevelX();
	virtual double getLevelY();
	virtual double getLevelZ();
	virtual void setCooldown(int time);
	virtual bool isOnCooldown();
};