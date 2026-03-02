#pragma once

#include "BaseEntityTile.h"
#include "ChestTile.h"

class EnderChestTile : public BaseEntityTile
{
public:
	static const int EVENT_SET_OPEN_COUNT = ChestTile::EVENT_SET_OPEN_COUNT;

	EnderChestTile(int id);
	virtual void updateDefaultShape(); // 4J Added override

	bool isSolidRender(bool isServerLevel = false);
	bool isCubeShaped();
	int getRenderShape();
	int getResource(int data, Random *random, int playerBonusLevel);
	int getResourceCount(Random *random);

protected:
	bool isSilkTouchable();

public:
	void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance);
	bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false);
	shared_ptr<TileEntity> newTileEntity(Level *level);
	void animateTick(Level *level, int xt, int yt, int zt, Random *random);
	virtual void registerIcons(IconRegister *iconRegister);
};
