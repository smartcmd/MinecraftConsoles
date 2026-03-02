#pragma once
using namespace std;

#include "Item.h"

class Player;

class HoeItem : public Item
{
protected:
	const Tier *tier;
public:
	HoeItem(int id, const Tier *tier);

	virtual bool useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly=false);
	virtual bool isHandEquipped();

	const Tier *getTier();
};