#pragma once

using namespace std;

#include "com.mojang.nbt.h"
#include "TileEntity.h"
#include "Container.h"

class Player;
class Random;
class Level;
class CompoundTag;

class DispenserTileEntity: public TileEntity, public Container
{
public:
	eINSTANCEOF GetType() { return eTYPE_DISPENSERTILEENTITY; }
	static TileEntity *create() { return new DispenserTileEntity(); }


	using TileEntity::setChanged;

private:
	ItemInstanceArray items;
	Random *random;

protected:
	wstring name;

public:
	DispenserTileEntity();
	virtual ~DispenserTileEntity();

	virtual unsigned int getContainerSize();
	virtual shared_ptr<ItemInstance> getItem(unsigned int slot);
	virtual shared_ptr<ItemInstance> removeItem(unsigned int slot, int count);
	virtual shared_ptr<ItemInstance> removeItemNoUpdate(int slot);
	virtual bool removeProjectile(int itemId);
	virtual int getRandomSlot();
	virtual void setItem(unsigned int slot, shared_ptr<ItemInstance> item);
	virtual int addItem(shared_ptr<ItemInstance> item);
	virtual wstring getName();
	virtual wstring getCustomName();
	virtual void setCustomName(const wstring &name);
	virtual bool hasCustomName();
	virtual void load(CompoundTag *base);
	virtual void save(CompoundTag *base);
	virtual int getMaxStackSize() const;
	virtual bool stillValid(shared_ptr<Player> player);
	virtual void setChanged();

	virtual void startOpen();
	virtual void stopOpen();
	virtual bool canPlaceItem(int slot, shared_ptr<ItemInstance> item);

	// 4J Added
	virtual shared_ptr<TileEntity> clone();
	void AddItemBack(shared_ptr<ItemInstance>item, unsigned int slot);
};