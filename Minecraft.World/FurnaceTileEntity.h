#pragma once
using namespace std;

#include "FurnaceTile.h"
#include "TileEntity.h"
#include "Container.h"

class Player;
class Level;

class FurnaceTileEntity : public TileEntity, public Container
{
public:
	eINSTANCEOF GetType() { return eTYPE_FURNACETILEENTITY; }
	static TileEntity *create() { return new FurnaceTileEntity(); }

using TileEntity::setChanged;

private:
	static const int BURN_INTERVAL;
	ItemInstanceArray *items;

	enum {
		INPUT_SLOT = 0,
		FUEL_SLOT,
		RESULT_SLOT,
	};

	// 4J-JEV: Added for 'Renewable Energy' achievement.
	// Should be true iff characoal was consumed whilst cooking the current stack.
	bool m_charcoalUsed;

public:
	int litTime;
	int litDuration;
	int tickCount;

public:
	// 4J Stu - Need a ctor to initialise member variables
	FurnaceTileEntity();
	virtual ~FurnaceTileEntity();

	virtual unsigned int getContainerSize();
	virtual shared_ptr<ItemInstance> getItem(unsigned int slot);
	virtual shared_ptr<ItemInstance> removeItem(unsigned int slot, int count);
	virtual shared_ptr<ItemInstance> removeItemNoUpdate(int slot);
	virtual void setItem(unsigned int slot, shared_ptr<ItemInstance> item);
	virtual int getName();
	virtual void load(CompoundTag *base);
	virtual void save(CompoundTag *base);
	virtual int getMaxStackSize();
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

	// 4J Added
	virtual shared_ptr<TileEntity> clone();

	// 4J-JEV: Added for 'Renewable Energy' achievement.
	bool wasCharcoalUsed() { return m_charcoalUsed; }
};