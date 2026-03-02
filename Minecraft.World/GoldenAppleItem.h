#pragma once

#include "FoodItem.h"

class GoldenAppleItem : public FoodItem
{
public:
	using Item::getUseDescriptionId;

	GoldenAppleItem(int id, int nutrition, float saturationMod, bool isMeat);

	virtual bool isFoil(shared_ptr<ItemInstance> itemInstance);
	virtual const Rarity *getRarity(shared_ptr<ItemInstance> itemInstance);

	// 4J-JEV: Enchanted goldenapples and goldenapples each require their own tooltips.
	virtual unsigned int getUseDescriptionId(int iData /*= -1*/);
	virtual unsigned int getUseDescriptionId(shared_ptr<ItemInstance> instance);

protected:
	void addEatEffect(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player);
};
