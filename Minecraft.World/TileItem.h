#pragma once
using namespace std;

#include "Item.h"

class Player;
class Level;

class TileItem : public Item
{
public: static const int _class = 0;
using Item::getColor;

private: 
	int tileId;
	Icon *itemIcon;

public:
	TileItem(int id); 
	
	virtual int getTileId();

	//@Override
	int getIconType();

	//@Override
	Icon *getIcon(int auxValue);

	virtual bool useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly=false);
	virtual unsigned int getDescriptionId(shared_ptr<ItemInstance> instance);
	virtual unsigned int getDescriptionId(int iData = -1);

	// 4J Added
	virtual int getColor(int itemAuxValue, int spriteLayer);

	// 4J Added
	virtual unsigned int getUseDescriptionId(shared_ptr<ItemInstance> instance);
	virtual unsigned int getUseDescriptionId();

	virtual bool mayPlace(Level *level, int x, int y, int z, int face, shared_ptr<Player> player, shared_ptr<ItemInstance> item);

	//@Override
    virtual void registerIcons(IconRegister *iconRegister);
};
