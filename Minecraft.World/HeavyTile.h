#pragma once
#include "Tile.h"

class Random;
class Level;
class FallingTile;

class HeavyTile : public Tile
{
public:
	static bool instaFall;

    HeavyTile(int type, bool isSolidRender = true);
    HeavyTile(int type, Material *material, bool isSolidRender = true);
    virtual void onPlace(Level *level, int x, int y, int z);
    virtual void neighborChanged(Level *level, int x, int y, int z, int type);
    virtual void tick(Level *level, int x, int y, int z, Random *random);
private:
	void checkSlide(Level *level, int x, int y, int z);
protected:
	virtual void falling(shared_ptr<FallingTile> entity);
public:
	virtual int getTickDelay();
    static bool isFree(Level *level, int x, int y, int z);
	virtual void onLand(Level *level, int xt, int yt, int zt, int data);
};
