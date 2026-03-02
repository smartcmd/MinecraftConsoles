#pragma once

#include "Item.h"

class ItemInstance;
class Player;
class Level;

class FireChargeItem : public Item
{
private:
	Icon *m_dragonFireballIcon;
public:
	FireChargeItem(int id);

	virtual bool useOn(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly);

	virtual Icon *getIcon(int itemAuxValue);
	virtual void registerIcons(IconRegister *iconRegister);
};