#pragma once

#include "Slot.h"

class MerchantContainer;
class Player;
class Merchant;

class MerchantResultSlot : public Slot
{
private:
	shared_ptr<MerchantContainer> slots;
	Player *player;
	int removeCount;
	shared_ptr<Merchant> merchant;

public:
	MerchantResultSlot(Player *player, shared_ptr<Merchant> merchant, shared_ptr<MerchantContainer> slots, int id, int x, int y);

	bool mayPlace(shared_ptr<ItemInstance> item);
	shared_ptr<ItemInstance> remove(int c);

protected:
	void onQuickCraft(shared_ptr<ItemInstance> picked, int count);
	void checkTakeAchievements(shared_ptr<ItemInstance> carried);

public:
	void onTake(shared_ptr<Player> player, shared_ptr<ItemInstance> carried);
	virtual bool mayCombine(shared_ptr<ItemInstance> item); // 4J Added

private:
	bool removePaymentItemsIfMatching(MerchantRecipe *activeRecipe, shared_ptr<ItemInstance> a, shared_ptr<ItemInstance> b);
};