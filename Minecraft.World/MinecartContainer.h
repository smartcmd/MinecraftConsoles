#pragma once

#include "Minecart.h"
#include "Container.h"

class MinecartContainer : public Minecart, public virtual Container
{
private:
	ItemInstanceArray items;
	bool dropEquipment;

	void _init();

public:
	MinecartContainer(Level *level);
	MinecartContainer(Level *level, double x, double y, double z);

	virtual void destroy(DamageSource *source);
	virtual shared_ptr<ItemInstance> getItem(unsigned int slot);
	virtual shared_ptr<ItemInstance> removeItem(unsigned int slot, int count);
	virtual shared_ptr<ItemInstance> removeItemNoUpdate(int slot);
	virtual void setItem(unsigned int slot, shared_ptr<ItemInstance> item);
	virtual void setChanged();
	virtual bool stillValid(shared_ptr<Player> player);
	virtual void startOpen();
	virtual void stopOpen();
	virtual bool canPlaceItem(int slot, shared_ptr<ItemInstance> item);
	virtual wstring getName();
	virtual int getMaxStackSize() const;
	virtual void changeDimension(int i);
	virtual void remove();

protected:
	virtual void addAdditonalSaveData(CompoundTag *base);
	virtual void readAdditionalSaveData(CompoundTag *base);

public:
	virtual bool interact(shared_ptr<Player> player);

protected:
	virtual void applyNaturalSlowdown();

public:

	// 4J Stu - For container
	virtual bool hasCustomName() { return Minecart::hasCustomName(); }
	virtual wstring getCustomName() { return Minecart::getCustomName(); }
};