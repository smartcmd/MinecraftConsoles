#pragma once

#include "AbstractContainerMenu.h"

class MerchantContainer;

class MerchantMenu : public AbstractContainerMenu
{
public:
	static const int PAYMENT1_SLOT = 0;
	static const int PAYMENT2_SLOT = 1;
	static const int RESULT_SLOT = 2;

	static const int INV_SLOT_START = RESULT_SLOT + 1;
	static const int INV_SLOT_END = INV_SLOT_START + 9 * 3;
	static const int USE_ROW_SLOT_START = INV_SLOT_END;
	static const int USE_ROW_SLOT_END = USE_ROW_SLOT_START + 9;

	static const int SELLSLOT1_X = 36;
	static const int SELLSLOT2_X = SELLSLOT1_X + 26;
	static const int BUYSLOT_X = 120;

	static const int ROW1_Y = 24;
	static const int ROW2_Y = 53;


private:
	shared_ptr<Merchant> trader;
	shared_ptr<MerchantContainer> tradeContainer;
	Level *level;

public:
	MerchantMenu(shared_ptr<Inventory> inventory, shared_ptr<Merchant> merchant, Level *level);

	shared_ptr<MerchantContainer> getTradeContainer();
	void addSlotListener(ContainerListener *listener);
	void broadcastChanges();
	void slotsChanged(); // 4J used to take a shared_ptr<Container> but wasn't using it, so removed to simplify things
	void setSelectionHint(int hint);
	void setData(int id, int value);
	bool stillValid(shared_ptr<Player> player);
	shared_ptr<ItemInstance> quickMoveStack(shared_ptr<Player> player, int slotIndex);
	void removed(shared_ptr<Player> player);

	shared_ptr<Merchant> getMerchant(); // 4J Added
};