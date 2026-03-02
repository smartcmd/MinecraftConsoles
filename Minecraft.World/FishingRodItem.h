#pragma once
using namespace std;

#include "Item.h"

class Player;
class Level;

class FishingRodItem : public Item
{
public:
	static const wstring TEXTURE_EMPTY;

private:
	Icon *emptyIcon;

public:
	FishingRodItem(int id);

	virtual bool isHandEquipped();
	virtual bool isMirroredArt();
	virtual shared_ptr<ItemInstance> use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player);	

	//@Override
	void registerIcons(IconRegister *iconRegister);
	Icon *getEmptyIcon();
};
