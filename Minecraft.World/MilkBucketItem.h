#pragma once

#include "Item.h"

class MilkBucketItem : public Item
{
private:
	static const int DRINK_DURATION = (int) (20 * 1.6);

public:
	MilkBucketItem(int id);

	virtual shared_ptr<ItemInstance> useTimeDepleted(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player);
	virtual int getUseDuration(shared_ptr<ItemInstance> itemInstance);
	virtual UseAnim getUseAnimation(shared_ptr<ItemInstance> itemInstance);
	virtual shared_ptr<ItemInstance> use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player);
};