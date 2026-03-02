#pragma once

#include "AbstractContainerMenu.h"
#include "Slot.h"

class BeaconTileEntity;

class BeaconMenu : public AbstractContainerMenu
{
private:
	class PaymentSlot : public Slot
	{
	public:
		PaymentSlot(shared_ptr<Container> container, int slot, int x, int y);

		bool mayPlace(shared_ptr<ItemInstance> item);
		int getMaxStackSize() const;
	};

public:
	static const int PAYMENT_SLOT = 0;
	static const int INV_SLOT_START = PAYMENT_SLOT + 1;
	static const int INV_SLOT_END = INV_SLOT_START + 9 * 3;
	static const int USE_ROW_SLOT_START = INV_SLOT_END;
	static const int USE_ROW_SLOT_END = USE_ROW_SLOT_START + 9;

private:
	shared_ptr<BeaconTileEntity> beacon;
	PaymentSlot *paymentSlot;

	// copied values because container/client system is retarded
	int levels;
	int primaryPower;
	int secondaryPower;

public:
	BeaconMenu(shared_ptr<Container> inventory, shared_ptr<BeaconTileEntity> beacon);

	void addSlotListener(ContainerListener *listener);
	void setData(int id, int value);
	shared_ptr<BeaconTileEntity> getBeacon();
	bool stillValid(shared_ptr<Player> player);
	shared_ptr<ItemInstance> quickMoveStack(shared_ptr<Player> player, int slotIndex);
};