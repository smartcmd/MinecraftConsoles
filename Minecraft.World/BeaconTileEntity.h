#pragma once
#include "TileEntity.h"
#include "Container.h"

class BeaconTileEntity : public TileEntity, public Container
{
public:
	eINSTANCEOF GetType() { return eTYPE_BEACONTILEENTITY; }
	static TileEntity *create() { return new BeaconTileEntity(); }
	// 4J Added
	virtual shared_ptr<TileEntity> clone();

private:
	static const int SCALE_TIME = SharedConstants::TICKS_PER_SECOND * 2;

public:
	static const int BEACON_EFFECTS_TIERS = 4;
	static const int BEACON_EFFECTS_EFFECTS = 3;
	static MobEffect *BEACON_EFFECTS[BEACON_EFFECTS_TIERS][BEACON_EFFECTS_EFFECTS];

	static void staticCtor();

private:
	__int64 clientSideRenderTick;
	float clientSideRenderScale;

	bool isActive;
	int levels;

	int primaryPower;
	int secondaryPower;

	shared_ptr<ItemInstance> paymentItem;
	wstring name;

public:
	BeaconTileEntity();

	void tick();

private:
	void applyEffects();
	void updateShape();

public:
	float getAndUpdateClientSideScale();
	int getPrimaryPower();
	int getSecondaryPower();
	int getLevels();
	// client-side method used by GUI
	void setLevels(int levels);
	void setPrimaryPower(int primaryPower);
	void setSecondaryPower(int secondaryPower);
	shared_ptr<Packet> getUpdatePacket();
	double getViewDistance();
	void load(CompoundTag *tag);
	void save(CompoundTag *tag);
	unsigned int getContainerSize();
	shared_ptr<ItemInstance> getItem(unsigned int slot);
	shared_ptr<ItemInstance> removeItem(unsigned int slot, int count);
	shared_ptr<ItemInstance> removeItemNoUpdate(int slot);
	void setItem(unsigned int slot, shared_ptr<ItemInstance> item);
	wstring getName();
	wstring getCustomName();
	bool hasCustomName();
	void setCustomName(const wstring &name);
	int getMaxStackSize() const;
	bool stillValid(shared_ptr<Player> player);
	void startOpen();
	void stopOpen();
	bool canPlaceItem(int slot, shared_ptr<ItemInstance> item);

	// 4J Stu - For container
	virtual void setChanged() { TileEntity::setChanged(); }
};