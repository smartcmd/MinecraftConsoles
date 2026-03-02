#pragma once

#include "Slot.h"

class ResultSlot : public Slot
{
private:
	shared_ptr<Container> craftSlots;
	Player *player; // This can't be a shared_ptr, as we create a result slot in the inventorymenu in the Player ctor
	int removeCount;

public:
	ResultSlot(Player *player, shared_ptr<Container> craftSlots, shared_ptr<Container> container, int id, int x, int y);
	virtual ~ResultSlot() {}

	virtual bool mayPlace(shared_ptr<ItemInstance> item);
	virtual shared_ptr<ItemInstance> remove(int c);

protected:
	virtual void onQuickCraft(shared_ptr<ItemInstance> picked, int count);
	virtual void checkTakeAchievements(shared_ptr<ItemInstance> carried);

public:
	virtual void onTake(shared_ptr<Player> player, shared_ptr<ItemInstance> carried);
	virtual bool mayCombine(shared_ptr<ItemInstance> item); // 4J Added
};