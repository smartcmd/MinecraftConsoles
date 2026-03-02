#pragma once
using namespace std;

#include "TileItem.h"
#include "HalfSlabTile.h"

class StoneSlabTileItem : public TileItem
{
private:
	bool isFull;
	HalfSlabTile *halfTile;
	HalfSlabTile *fullTile;
public:
	StoneSlabTileItem(int id, HalfSlabTile *halfTile, HalfSlabTile *fullTile, bool full);

	virtual Icon *getIcon(int itemAuxValue);
	virtual int getLevelDataForAuxValue(int auxValue);
	virtual unsigned int getDescriptionId(shared_ptr<ItemInstance> instance);
	virtual bool useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly=false);

	virtual bool mayPlace(Level *level, int x, int y, int z, int face,shared_ptr<Player> player, shared_ptr<ItemInstance> item);
private:
	bool tryConvertTargetTile(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, bool bTestUseOnOnly);
};