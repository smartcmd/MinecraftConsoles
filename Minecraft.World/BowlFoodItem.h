#pragma once

#include "FoodItem.h"

class Player;
class Level;

class BowlFoodItem : public FoodItem
{
public:
	BowlFoodItem(int id, int nutrition);

	shared_ptr<ItemInstance> useTimeDepleted(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player);
};