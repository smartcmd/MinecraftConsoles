#pragma once

#include "BaseRailTile.h"

class PoweredRailTile : public BaseRailTile
{
	friend class ChunkRebuildData;
protected:
	Icon *iconPowered;
	
public:
	PoweredRailTile(int id);

	virtual Icon *getTexture(int face, int data);
	virtual void registerIcons(IconRegister *iconRegister);

protected:
	virtual bool findPoweredRailSignal(Level *level, int x, int y, int z, int data, bool forward, int searchDepth);
	virtual bool isSameRailWithPower(Level *level, int x, int y, int z, bool forward, int searchDepth, int dir);
	virtual void updateState(Level *level, int x, int y, int z, int data, int dir, int type);
};