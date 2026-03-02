#pragma once

#include "Item.h"

class SkullItem : public Item
{
private:
	static const int SKULL_COUNT = 5;

	static const unsigned int NAMES[SKULL_COUNT];

public:
	static wstring ICON_NAMES[SKULL_COUNT];

private:
	Icon *icons[SKULL_COUNT];

public:
	SkullItem(int id);

	bool useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly=false);
	bool mayPlace(Level *level, int x, int y, int z, int face, shared_ptr<Player> player, shared_ptr<ItemInstance> item);
	Icon *getIcon(int itemAuxValue);
	int getLevelDataForAuxValue(int auxValue);
	virtual unsigned int getDescriptionId(int iData = -1);
	unsigned int getDescriptionId(shared_ptr<ItemInstance> instance);
	wstring getHoverName(shared_ptr<ItemInstance> itemInstance);
	void registerIcons(IconRegister *iconRegister);
};