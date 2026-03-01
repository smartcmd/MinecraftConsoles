#pragma once
#include "Tile.h"
#include "TilePos.h"
#include "Definitions.h"

class Random;
class HitResult;
class ChunkRebuildData;

using namespace std;

class BaseRailTile : public Tile
{
	friend class Tile;
	friend class ChunkRebuildData;
public:
	static const int DIR_FLAT_Z = 0;
	static const int DIR_FLAT_X = 1;
	// the data bit is used by boosters and detectors, so they can't turn
	static const int RAIL_DATA_BIT = 8;
	static const int RAIL_DIRECTION_MASK = 7;

private:
	Icon *iconTurn;

protected:
	bool usesDataBit;

	class Rail
	{
		friend class BaseRailTile;
		friend class RailTile;
	private:
		Level *level;
		int x, y, z;
		bool usesDataBit;
		vector<TilePos *> connections;
		bool m_bValidRail; // 4J added

	public:
		Rail(Level *level, int x, int y, int z);
		~Rail();
	private:
		void updateConnections(int direction);
		void removeSoftConnections();
		bool hasRail(int x, int y, int z);
		Rail *getRail(TilePos *p);
		bool connectsTo(Rail *rail);
		bool hasConnection(int x, int y, int z);

	protected:
		int countPotentialConnections();

	private:
		bool canConnectTo(Rail *rail);
		void connectTo(Rail *rail);
		bool hasNeighborRail(int x, int y, int z);
	public:
		void place(bool hasSignal, bool first);
	};
public:
	static bool isRail(Level *level, int x, int y, int z);
	static bool isRail(int id);
protected:
	BaseRailTile(int id, bool usesDataBit);
public:
	using Tile::getResourceCount;

	bool isUsesDataBit();
	virtual AABB *getAABB(Level *level, int x, int y, int z);
	virtual bool blocksLight();
	virtual bool isSolidRender(bool isServerLevel = false);
	virtual HitResult *clip(Level *level, int xt, int yt, int zt, Vec3 *a, Vec3 *b);
	virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());	// 4J added forceData, forceEntity param
	virtual bool isCubeShaped();
	virtual int getRenderShape();
	virtual int getResourceCount(Random random);
	virtual bool mayPlace(Level *level, int x, int y, int z);
	virtual void onPlace(Level *level, int x, int y, int z);
	virtual void neighborChanged(Level *level, int x, int y, int z, int type);

protected:
	virtual void updateState(Level *level, int x, int y, int z, int data, int dir, int type);
	virtual void updateDir(Level *level, int x, int y, int z, bool first);

public:
	int getPistonPushReaction();
	void onRemove(Level *level, int x, int y, int z, int id, int data);
};
