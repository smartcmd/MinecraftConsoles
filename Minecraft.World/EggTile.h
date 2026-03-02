#pragma once
#include "Tile.h"

class EggTile : public Tile
{
public:
	EggTile(int id);
    virtual void onPlace(Level *level, int x, int y, int z);
    virtual void neighborChanged(Level *level, int x, int y, int z, int type);
    virtual void tick(Level *level, int x, int y, int z, Random *random);
private:
	void checkSlide(Level *level, int x, int y, int z);
public:
	virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false); // 4J added soundOnly param
    virtual void attack(Level *level, int x, int y, int z, shared_ptr<Player> player);
private:
	void teleport(Level *level, int x, int y, int z);
public:
	virtual int getTickDelay();
    virtual bool blocksLight();
    virtual bool isSolidRender(bool isServerLevel = false);
    virtual bool isCubeShaped();
	virtual bool shouldRenderFace(LevelSource *level, int x, int y, int z, int face);
    virtual int getRenderShape();
	virtual int cloneTileId(Level *level, int x, int y, int z);

	// 4J Added
	static void generateTeleportParticles(Level *level,int xt,int yt, int zt,int deltas);
};