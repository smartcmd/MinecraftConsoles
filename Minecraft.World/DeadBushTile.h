#pragma once
#include "Bush.h"

class Random;

class DeadBushTile : public Bush
{
	friend class Tile;
protected:
	DeadBushTile(int id);
    virtual bool mayPlaceOn(int tile);
public:
    virtual void updateDefaultShape(); // 4J Added override
    virtual int getResource(int data, Random *random, int playerBonusLevel);
	virtual void playerDestroy(Level *level, shared_ptr<Player> player, int x, int y, int z, int data);
};
