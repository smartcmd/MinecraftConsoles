#pragma once
#include "TileEntity.h"
#include "Container.h"

class BrewingStandTileEntity : public TileEntity, public Container
{
public:
	eINSTANCEOF GetType() { return eTYPE_BREWINGSTANDTILEENTITY; }
	static TileEntity *create() { return new BrewingStandTileEntity(); }

private:
	ItemInstanceArray items;
    static const int INGREDIENT_SLOT = 3;

    int brewTime;
    int lastPotionCount;
    int ingredientId;

public:
	BrewingStandTileEntity();
	~BrewingStandTileEntity();
	virtual int getName();
	virtual unsigned int getContainerSize();
	virtual void tick();

    int getBrewTime();

private:
	bool isBrewable();
    void doBrew();

	int applyIngredient(int currentBrew, shared_ptr<ItemInstance> ingredient);
   
public:
	virtual void load(CompoundTag *base);
    virtual void save(CompoundTag *base);
    virtual shared_ptr<ItemInstance> getItem(unsigned int slot);
    virtual shared_ptr<ItemInstance> removeItem(unsigned int slot, int i);
	virtual shared_ptr<ItemInstance> removeItemNoUpdate(int slot);
    virtual void setItem(unsigned int slot, shared_ptr<ItemInstance> item);
    virtual int getMaxStackSize();
    virtual bool stillValid(shared_ptr<Player> player);
    virtual void startOpen();
    virtual void stopOpen();
    virtual void setBrewTime(int value);
	virtual void setChanged() {} // 4J added
    int getPotionBits();

	// 4J Added
	virtual shared_ptr<TileEntity> clone();
};