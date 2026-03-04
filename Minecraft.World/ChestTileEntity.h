#pragma once

using namespace std;

#include "TileEntity.h"
#include "Container.h"

#include "ListTag.h"

class Player;
class CompoundTag;

class ChestTileEntity : public TileEntity, public Container
{
public:
	eINSTANCEOF GetType() { return eTYPE_CHESTTILEENTITY; }
	static TileEntity *create() { return new ChestTileEntity(); }

	int getContainerType(); // 4J-Added;

	using TileEntity::setChanged;

private:
	void _init(bool isBonusChest);

public:
	ChestTileEntity(bool isBonusChest = false);		// 4J added param
	ChestTileEntity(int type, bool isBonusChest = false);		// 4J added param
	virtual ~ChestTileEntity();

private:
	ItemInstanceArray *items;

public:
	bool isBonusChest;					// 4J added
	bool hasCheckedNeighbors;
	weak_ptr<ChestTileEntity> n;
	weak_ptr<ChestTileEntity> e;
	weak_ptr<ChestTileEntity> w;
	weak_ptr<ChestTileEntity> s;

	float openness, oOpenness;
	int openCount;
private:
	int tickInterval;

	int type;
	wstring name;

public:
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
	virtual bool stillValid(shared_ptr<Player> player);
	virtual void setChanged();
	virtual void clearCache();

private:
	virtual void heyImYourNeighbor(shared_ptr<ChestTileEntity> neighbor, int from);

public:
	virtual void checkNeighbors();

private:
	bool isSameChest(int x, int y, int z);

public:
	virtual void tick();
	virtual bool triggerEvent(int b0, int b1);
	virtual void startOpen();
	virtual void stopOpen();
	virtual bool canPlaceItem(int slot, shared_ptr<ItemInstance> item);
	virtual void setRemoved();
	virtual int getType();

	// 4J Added
	virtual shared_ptr<TileEntity> clone();
};