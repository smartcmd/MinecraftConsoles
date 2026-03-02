#pragma once
using namespace std;

#include "Item.h"

class Player;
class Level;

class FishingRodItem : public Item
{
private:
	Icon *emptyIcon;

public:
	FishingRodItem(int id);

	virtual bool isHandEquipped();
	virtual bool isMirroredArt();
	virtual shared_ptr<ItemInstance> use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player);	

	void registerIcons(IconRegister *iconRegister);
	Icon *getEmptyIcon();
};
