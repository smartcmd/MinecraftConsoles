#pragma once
using namespace std;

#include "Item.h"

class Player;
class Mob;
class Level;

class DyePowderItem : public Item
{
public:
	static const unsigned int COLOR_DESCS[];
	static const unsigned int COLOR_USE_DESCS[];
	static const wstring COLOR_TEXTURES[];
	static const int COLOR_RGB[];

	static const int BLACK;
	static const int RED;
	static const int GREEN;
	static const int BROWN;
	static const int BLUE;
	static const int PURPLE;
	static const int CYAN;
	static const int SILVER;
	static const int GRAY;
	static const int PINK;
	static const int LIME;
	static const int YELLOW;
	static const int LIGHT_BLUE;
	static const int MAGENTA;
	static const int ORANGE;
	static const int WHITE;

private:
	static const int DYE_POWDER_ITEM_TEXTURE_COUNT = 16;
	Icon **icons;

public:
	DyePowderItem(int id);

	virtual Icon *getIcon(int itemAuxValue);
	virtual unsigned int getDescriptionId(shared_ptr<ItemInstance> itemInstance);
	virtual unsigned int getUseDescriptionId(shared_ptr<ItemInstance> itemInstance);
	virtual bool useOn(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly=false);
	virtual bool interactEnemy(shared_ptr<ItemInstance> itemInstance, shared_ptr<Mob> mob);

	//@Override
    void registerIcons(IconRegister *iconRegister);
};
