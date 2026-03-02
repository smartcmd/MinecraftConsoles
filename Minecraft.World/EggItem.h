#pragma once
using namespace std;

#include "Item.h"

class Player;
class Level;

class EggItem : public Item
{
public:
	EggItem(int id);

	virtual shared_ptr<ItemInstance> use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player);
};
