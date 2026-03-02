#pragma once

#include "Slot.h"

class FurnaceResultSlot : public Slot
{
private:
	shared_ptr<Player> player;
	int removeCount;

public:
	FurnaceResultSlot(shared_ptr<Player> player, shared_ptr<Container> container, int slot, int x, int y);
	virtual ~FurnaceResultSlot() {}

	virtual bool mayPlace(shared_ptr<ItemInstance> item);
	virtual shared_ptr<ItemInstance> remove(int c);
	virtual void onTake(shared_ptr<Player> player, shared_ptr<ItemInstance> carried);
	virtual bool mayCombine(shared_ptr<ItemInstance> item); // 4J Added

protected:
	virtual void onQuickCraft(shared_ptr<ItemInstance> picked, int count);
	virtual void checkTakeAchievements(shared_ptr<ItemInstance> carried);
};