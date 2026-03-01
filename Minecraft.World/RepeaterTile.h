#pragma once

#include "DiodeTile.h"

class RepeaterTile : public DiodeTile
{
public:
	static const int DELAY_MASK = DIRECTION_INV_MASK;
	static const int DELAY_SHIFT = 2;

	static const double DELAY_RENDER_OFFSETS[4];

private:
	static const int DELAYS[4];
	
public:
	RepeaterTile(int id, bool on);

	virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false);

protected:
	virtual int getTurnOnDelay(int data);
	virtual DiodeTile *getOnTile();
	virtual DiodeTile *getOffTile();

public:
	virtual int getResource(int data, Random *random, int playerBonusLevel);
	virtual int cloneTileId(Level *level, int x, int y, int z);
	virtual int getRenderShape();
	virtual bool isLocked(LevelSource *level, int x, int y, int z, int data);

protected:
	virtual bool isAlternateInput(int tile);

public:
	void animateTick(Level *level, int xt, int yt, int zt, Random *random);
	void onRemove(Level *level, int x, int y, int z, int id, int data);

	virtual bool TestUse();
};