#pragma once
#include "DirectionalTile.h"

class Player;
class Random;
class Level;

class DiodeTile : public DirectionalTile
{
	friend class Tile;
public:
    static const int DELAY_MASK = DIRECTION_INV_MASK;
    static const int DELAY_SHIFT = 2;

    static const double DELAY_RENDER_OFFSETS[4];
    static const int DELAYS[4];

private:
    bool on;

protected:
	DiodeTile(int id, bool on);
public:
    virtual void updateDefaultShape(); // 4J Added override
    virtual bool isCubeShaped();
    virtual bool mayPlace(Level *level, int x, int y, int z);
    virtual bool canSurvive(Level *level, int x, int y, int z);
    virtual void tick(Level *level, int x, int y, int z, Random *random);
    virtual Icon *getTexture(int face, int data);
	//@Override
	void registerIcons(IconRegister *iconRegister);
    virtual bool shouldRenderFace(LevelSource *level, int x, int y, int z, int face);
    virtual int getRenderShape();
    virtual bool getDirectSignal(Level *level, int x, int y, int z, int dir);
    virtual bool getSignal(LevelSource *level, int x, int y, int z, int facing);
    virtual void neighborChanged(Level *level, int x, int y, int z, int type);
private:
	virtual bool getSourceSignal(Level *level, int x, int y, int z, int data);
public:
	virtual bool TestUse();
	virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false); // 4J added soundOnly param
    virtual bool isSignalSource();
    virtual void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<Mob> by);
    virtual void onPlace(Level *level, int x, int y, int z);
	virtual void destroy(Level *level, int x, int y, int z, int data);
    virtual bool isSolidRender(bool isServerLevel = false);
    virtual int getResource(int data, Random *random, int playerBonusLevel);
    virtual void animateTick(Level *level, int xt, int yt, int zt, Random *random);
	virtual int cloneTileId(Level *level, int x, int y, int z);
};
