#pragma once

#include "DiodeTile.h"
#include "EntityTile.h"

class ComparatorTileEntity;

class ComparatorTile : public DiodeTile, public EntityTile
{
private:
	static const int BIT_OUTPUT_SUBTRACT = 0x4;
	static const int BIT_IS_LIT = 0x8;

public:
	ComparatorTile(int id, bool on);

	virtual int getResource(int data, Random *random, int playerBonusLevel);
	virtual int cloneTileId(Level *level, int x, int y, int z);

protected:
	virtual int getTurnOnDelay(int data);
	virtual DiodeTile *getOnTile();
	virtual DiodeTile *getOffTile();

public:
	virtual int getRenderShape();
	virtual Icon *getTexture(int face, int data);

protected:
	virtual bool isOn(int data);
	virtual int getOutputSignal(LevelSource *levelSource, int x, int y, int z, int data);

private:
	virtual int calculateOutputSignal(Level *level, int x, int y, int z, int data);

public:
	virtual bool isReversedOutputSignal(int data);

protected:
	virtual bool shouldTurnOn(Level *level, int x, int y, int z, int data);
	virtual int getInputSignal(Level *level, int x, int y, int z, int data);
	virtual shared_ptr<ComparatorTileEntity> getComparator(LevelSource *level, int x, int y, int z);

public:
	virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false);

protected:
	virtual void checkTickOnNeighbor(Level *level, int x, int y, int z, int type);

private:
	virtual void refreshOutputState(Level *level, int x, int y, int z, Random *random);

public:
	virtual void tick(Level *level, int x, int y, int z, Random *random);
	virtual void onPlace(Level *level, int x, int y, int z);
	virtual void onRemove(Level *level, int x, int y, int z, int id, int data);
	virtual bool triggerEvent(Level *level, int x, int y, int z, int b0, int b1);
	virtual shared_ptr<TileEntity> newTileEntity(Level *level);
	virtual bool TestUse();
};