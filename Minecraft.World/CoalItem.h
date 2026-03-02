#pragma once
using namespace std;

#include "Item.h"

class ItemInstance;

class CoalItem : public Item
{
private:
	Icon *charcoalIcon;

public:
	static const int STONE_COAL = 0;
	static const int CHAR_COAL = 1;

	CoalItem(int id);

	virtual unsigned int getDescriptionId(shared_ptr<ItemInstance> instance); 

	Icon *getIcon(int auxValue);
	void registerIcons(IconRegister *iconRegister);
};