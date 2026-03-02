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
	ItemInstanceArray *items;
	Random *random;

public:
	DispenserTileEntity();
	virtual ~DispenserTileEntity();

	virtual unsigned int getContainerSize();
	virtual shared_ptr<ItemInstance> getItem(unsigned int slot);
	virtual shared_ptr<ItemInstance> removeItem(unsigned int slot, int count);
	shared_ptr<ItemInstance> removeItemNoUpdate(int slot);
	bool removeProjectile(int itemId);
	int getRandomSlot();
	virtual void setItem(unsigned int slot, shared_ptr<ItemInstance> item);
	virtual int addItem(shared_ptr<ItemInstance> item);
	virtual int getName();
	virtual void load(CompoundTag *base);
	virtual void save(CompoundTag *base);
	virtual int getMaxStackSize();
	virtual bool stillValid(shared_ptr<Player> player);
	virtual void setChanged();

    void startOpen();
    void stopOpen();

	// 4J Added
	virtual shared_ptr<TileEntity> clone();
	void AddItemBack(shared_ptr<ItemInstance>item, unsigned int slot); 
};