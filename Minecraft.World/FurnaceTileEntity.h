#pragma once
using namespace std;

#include "FurnaceTile.h"
#include "TileEntity.h"
#include "WorldlyContainer.h"

class Player;
class Level;

class FurnaceTileEntity : public TileEntity, public WorldlyContainer
{
public:
	eINSTANCEOF GetType() { return eTYPE_FURNACETILEENTITY; }
	static TileEntity *create() { return new FurnaceTileEntity(); }

	using TileEntity::setChanged;

	static const int SLOT_INPUT = 0;
	static const int SLOT_FUEL = 1;
	static const int SLOT_RESULT = 2;

private:
	static const intArray SLOTS_FOR_UP;
	static const intArray SLOTS_FOR_DOWN;
	static const intArray SLOTS_FOR_SIDES;

	static const int BURN_INTERVAL;
	ItemInstanceArray items;

	// 4J-JEV: Added for 'Renewable Energy' achievement.
	// Should be true iff characoal was consumed whilst cooking the current stack.
	bool m_charcoalUsed;

public:
	int litTime;
	int litDuration;
	int tickCount;

private:

	wstring name;

public:
	// 4J Stu - Need a ctor to initialise member variables
	FurnaceTileEntity();
	virtual ~FurnaceTileEntity();

	virtual unsigned int getContainerSize();
	virtual shared_ptr<ItemInstance> getItem(unsigned int slot);
	virtual shared_ptr<ItemInstance> removeItem(unsigned int slot, int count);
	virtual shared_ptr<ItemInstance> removeItemNoUpdate(int slot);
	virtual void setItem(unsigned int slot, shared_ptr<ItemInstance> item);
	virtual wstring getName();
	virtual wstring getCustomName();
	virtual bool hasCustomName();
	virtual void setCustomName(const wstring &name);
	virtual void load(CompoundTag *base);
	virtual void save(CompoundTag *base);
	virtual int getMaxStackSize() const;
	int getBurnProgress(int max);
	int getLitProgress(int max);
	bool isLit();
	virtual void tick();

private:
	bool canBurn();

public:
	void burn();

	static int getBurnDuration(shared_ptr<ItemInstance> itemInstance);
	static bool isFuel(shared_ptr<ItemInstance> item);

public:
	virtual bool stillValid(shared_ptr<Player> player);
	virtual void setChanged();

	void startOpen();
	void stopOpen();

	virtual bool canPlaceItem(int slot, shared_ptr<ItemInstance> item);
	virtual intArray getSlotsForFace(int face);
	virtual bool canPlaceItemThroughFace(int slot, shared_ptr<ItemInstance> item, int face);
	virtual bool canTakeItemThroughFace(int slot, shared_ptr<ItemInstance> item, int face);

	// 4J Added
	virtual shared_ptr<TileEntity> clone();

	// 4J-JEV: Added for 'Renewable Energy' achievement.
	bool wasCharcoalUsed() { return m_charcoalUsed; }
};