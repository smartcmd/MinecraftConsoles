#pragma once

#include "Item.h"

class Player;
class Level;

class BowItem : public Item
{
public:
	static const wstring TEXTURE_PULL[];
	static const int MAX_DRAW_DURATION = 20 * 1;

private:
	static const int BOW_ICONS_COUNT = 3;
	Icon **icons;

public:
	BowItem(int id);

	virtual void releaseUsing(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player, int durationLeft);
	virtual shared_ptr<ItemInstance> useTimeDepleted(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player);
	virtual int getUseDuration(shared_ptr<ItemInstance> itemInstance);
	virtual UseAnim getUseAnimation(shared_ptr<ItemInstance> itemInstance);
	virtual shared_ptr<ItemInstance> use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player);
	virtual int getEnchantmentValue();

	//@Override
	void registerIcons(IconRegister *iconRegister);
	Icon *getDrawnIcon(int amount);
};