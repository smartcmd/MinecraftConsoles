#pragma once
#include "Tile.h"
#include "Definitions.h"
using namespace std;

class Random;
class Level;
class ChunkRebuildData;

class RedStoneDustTile : public Tile
{
	friend ChunkRebuildData;
public:
	static const wstring TEXTURE_CROSS;
	static const wstring TEXTURE_LINE;
	static const wstring TEXTURE_CROSS_OVERLAY;
	static const wstring TEXTURE_LINE_OVERLAY;
private:
	bool shouldSignal;
	unordered_set<TilePos, TilePosKeyHash, TilePosKeyEq> toUpdate;
	Icon *iconCross;
	Icon *iconLine;
	Icon *iconCrossOver;
	Icon *iconLineOver;

public:
	RedStoneDustTile(int id);
	virtual void updateDefaultShape(); // 4J Added override
	virtual AABB *getAABB(Level *level, int x, int y, int z);
	virtual bool isSolidRender(bool isServerLevel = false);
	virtual bool isCubeShaped();
	virtual int getRenderShape();
	virtual int getColor() const; // 4J Added
	virtual int getColor(LevelSource *level, int x, int y, int z);
	virtual int getColor(LevelSource *level, int x, int y, int z, int data);	// 4J added
	virtual bool mayPlace(Level *level, int x, int y, int z);
private:
	void updatePowerStrength(Level *level, int x, int y, int z);
	void updatePowerStrength(Level *level, int x, int y, int z, int xFrom, int yFrom, int zFrom);
	void checkCornerChangeAt(Level *level, int x, int y, int z);
public:
	virtual void onPlace(Level *level, int x, int y, int z);
	virtual void onRemove(Level *level, int x, int y, int z, int id, int data);
private:
	int checkTarget(Level *level, int x, int y, int z, int target);
public:
	virtual void neighborChanged(Level *level, int x, int y, int z, int type);
	virtual int getResource(int data, Random *random, int playerBonusLevel);
	virtual int getDirectSignal(LevelSource *level, int x, int y, int z, int dir);
	virtual int getSignal(LevelSource *level, int x, int y, int z, int dir);

	virtual bool isSignalSource();
	virtual void animateTick(Level *level, int x, int y, int z, Random *random);

	static bool shouldConnectTo(LevelSource *level, int x, int y, int z, int direction);
	static bool shouldReceivePowerFrom(LevelSource *level, int x, int y, int z, int direction);
	virtual int cloneTileId(Level *level, int x, int y, int z);

	void registerIcons(IconRegister *iconRegister);
	static Icon *getTexture(const wstring &name);
};
