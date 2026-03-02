#pragma once

#include "BaseEntityTile.h"

class BeaconTile : public BaseEntityTile
{
public:
	BeaconTile(int id);

	shared_ptr<TileEntity> newTileEntity(Level *level);
	bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false);
	bool isSolidRender(bool isServerLevel = false);
	bool isCubeShaped();
	bool blocksLight();
	int getRenderShape();
	void registerIcons(IconRegister *iconRegister);
	void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance);
	virtual bool TestUse();
};