#pragma once
#include "DirectionalTile.h"

class Player;
class Random;
class Level;

class DiodeTile : public DirectionalTile
{
	friend class Tile;
protected:
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
	virtual bool shouldRenderFace(LevelSource *level, int x, int y, int z, int face);
	virtual int getRenderShape();

protected:
	virtual bool isOn(int data);

public:
	virtual int getDirectSignal(LevelSource *level, int x, int y, int z, int dir);
	virtual int getSignal(LevelSource *level, int x, int y, int z, int facing);
	virtual void neighborChanged(Level *level, int x, int y, int z, int type);

protected:
	virtual void checkTickOnNeighbor(Level *level, int x, int y, int z, int type);

public:
	virtual bool isLocked(LevelSource *level, int x, int y, int z, int data);

protected:
	virtual bool shouldTurnOn(Level *level, int x, int y, int z, int data);
	virtual int getInputSignal(Level *level, int x, int y, int z, int data);
	virtual int getAlternateSignal(LevelSource *level, int x, int y, int z, int data);
	virtual int getAlternateSignalAt(LevelSource *level, int x, int y, int z, int facing);

public:
	virtual bool isSignalSource();
	virtual void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance);
	virtual void onPlace(Level *level, int x, int y, int z);

protected:
	virtual void updateNeighborsInFront(Level *level, int x, int y, int z);

public:
	virtual void destroy(Level *level, int x, int y, int z, int data);
	virtual bool isSolidRender(bool isServerLevel = false);
	virtual int getResource(int data, Random *random, int playerBonusLevel) = 0;
	virtual int cloneTileId(Level *level, int x, int y, int z) = 0;

protected:
	virtual bool isAlternateInput(int tile);
	virtual int getOutputSignal(LevelSource *level, int x, int y, int z, int data);

public:
	static bool isDiode(int id);
	virtual bool isSameDiode(int id);
	virtual bool shouldPrioritize(Level *level, int x, int y, int z, int data);

protected:
	virtual int getTurnOffDelay(int data);

	virtual int getTurnOnDelay(int data) = 0;
	virtual DiodeTile *getOnTile() = 0;
	virtual DiodeTile *getOffTile() = 0;

public:
	virtual bool isMatching(int id);
};
