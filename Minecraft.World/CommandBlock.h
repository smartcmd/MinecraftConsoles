#pragma once

#include "BaseEntityTile.h"

class CommandBlock : public BaseEntityTile
{
private:
	static const int TRIGGER_BIT = 1;

public:
	CommandBlock(int id);

	virtual shared_ptr<TileEntity> newTileEntity(Level *level);
	virtual void neighborChanged(Level *level, int x, int y, int z, int type);
	virtual void tick(Level *level, int x, int y, int z, Random *random);
	virtual int getTickDelay(Level *level);
	virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false);
	virtual bool hasAnalogOutputSignal();
	virtual int getAnalogOutputSignal(Level *level, int x, int y, int z, int dir);
	virtual void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance);
	virtual int getResourceCount(Random *random);
};