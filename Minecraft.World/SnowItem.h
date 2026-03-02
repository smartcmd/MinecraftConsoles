#pragma once

#include "AuxDataTileItem.h"

class SnowItem : public AuxDataTileItem
{
public:
	SnowItem(int id, Tile *parentTile);

	bool useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly=false);
};