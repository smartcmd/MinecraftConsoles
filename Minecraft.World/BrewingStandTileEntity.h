#pragma once
#include "TileEntity.h"
#include "WorldlyContainer.h"

class BrewingStandTileEntity : public TileEntity, public WorldlyContainer
{
public:
	eINSTANCEOF GetType() { return eTYPE_BREWINGSTANDTILEENTITY; }
	static TileEntity *create() { return new BrewingStandTileEntity(); }

	static const int INGREDIENT_SLOT = 3;

private:
	ItemInstanceArray items;
	static intArray SLOTS_FOR_UP;
	static intArray SLOTS_FOR_OTHER_FACES;

	int brewTime;
	int lastPotionCount;
	int ingredientId;
	wstring name;

public:
	BrewingStandTileEntity();
	~BrewingStandTileEntity();
	virtual wstring getName();
	virtual wstring getCustomName();
	virtual bool hasCustomName();
	virtual void setCustomName(const wstring &name);
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
	virtual int getMaxStackSize() const;
	virtual bool stillValid(shared_ptr<Player> player);
	virtual void startOpen();
	virtual void stopOpen();
	virtual bool canPlaceItem(int slot, shared_ptr<ItemInstance> item);
	virtual void setBrewTime(int value);
	virtual void setChanged() { TileEntity::setChanged(); } // 4J added
	int getPotionBits();
	virtual intArray getSlotsForFace(int face);
	virtual bool canPlaceItemThroughFace(int slot, shared_ptr<ItemInstance> item, int face);
	virtual bool canTakeItemThroughFace(int slot, shared_ptr<ItemInstance> item, int face);

	// 4J Added
	virtual shared_ptr<TileEntity> clone();
};