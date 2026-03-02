#pragma once

#include "Slot.h"

class RepairMenu;

class RepairResultSlot : public Slot
{
private:
	RepairMenu *m_menu;
	int xt, yt, zt;

public:
	RepairResultSlot(RepairMenu *menu, int xt, int yt, int zt, shared_ptr<Container> container, int slot, int x, int y);

	bool mayPlace(shared_ptr<ItemInstance> item);
	bool mayPickup(shared_ptr<Player> player);
	void onTake(shared_ptr<Player> player, shared_ptr<ItemInstance> carried);
	virtual bool mayCombine(shared_ptr<ItemInstance> item); // 4J Added
};