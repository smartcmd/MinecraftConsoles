#pragma once

#include "MetalTile.h"

class PoweredMetalTile : public MetalTile
{
public:
	PoweredMetalTile(int id);

	virtual bool isSignalSource();
	virtual int getSignal(LevelSource *level, int x, int y, int z, int dir);
};