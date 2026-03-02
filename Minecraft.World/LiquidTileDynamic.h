#pragma once
#include "LiquidTile.h"

class Random;

class LiquidTileDynamic : public LiquidTile
{
	friend class Tile;
private:
	// 4J Added
	typedef struct _LiquidTickData
	{
		Level *level;
		int x, y, z;
		Random *random;
		_LiquidTickData(Level *level, int x, int y, int z, Random *random)
			: level(level), x(x), y(y), z(z), random(random)
		{
		}
	} LiquidTickData;
	deque<LiquidTickData> m_tilesToTick; // For an iterative version of instatick
	bool m_iterativeInstatick;
protected:
	LiquidTileDynamic(int id, Material *material);
	virtual ~LiquidTileDynamic();
private:
	void setStatic(Level *level, int x, int y, int z);
	int maxCount;
public:
	virtual bool isPathfindable(LevelSource *level, int x, int y, int z);

private:
	// 4J Added
	void iterativeTick(Level *level, int x, int y, int z, Random *random);
	void mainTick(Level *level, int x, int y, int z, Random *random);
public:
	void tick(Level *level, int x, int y, int z, Random *random);
private:
	void trySpreadTo(Level *level, int x, int y, int z, int neighbor);

	bool *result;
	int *dist;

private:
	int getSlopeDistance(Level *level, int x, int y, int z, int pass, int from);
	bool *getSpread(Level *level, int x, int y, int z);
	bool isWaterBlocking(Level *level, int x, int y, int z);
protected:
	int getHighest(Level *level, int x, int y, int z, int current);
private:
	bool canSpreadTo(Level *level, int x, int y, int z);
public:
	void onPlace(Level *level, int x, int y, int z);
	bool canInstantlyTick();
};