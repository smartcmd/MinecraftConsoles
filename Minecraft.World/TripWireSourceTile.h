#pragma once

#include "Tile.h"

class TripWireSourceTile : public Tile
{
public:
	using Tile::getTickDelay;

	static const int MASK_DIR = 0x3;
	static const int MASK_ATTACHED = 0x4;
	static const int MASK_POWERED = 0x8;
	static const int WIRE_DIST_MIN = 1;
	static const int WIRE_DIST_MAX = 2 + 40; // 2 hooks + x string

	TripWireSourceTile(int id);

	AABB *getAABB(Level *level, int x, int y, int z);
	bool blocksLight();
	bool isSolidRender(bool isServerLevel = false);
	bool isCubeShaped();
	int getRenderShape();
	int getTickDelay(Level *level);
	bool mayPlace(Level *level, int x, int y, int z, int face);
	bool mayPlace(Level *level, int x, int y, int z);
	int getPlacedOnFaceDataValue(Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue);
	void finalizePlacement(Level *level, int x, int y, int z, int data);
	void neighborChanged(Level *level, int x, int y, int z, int type);
	void calculateState(Level *level, int x, int y, int z, int id, int data, bool canUpdate, int wireSource, int wireSourceData);
	void tick(Level *level, int x, int y, int z, Random *random);

private:
	void playSound(Level *level, int x, int y, int z, bool attached, bool powered, bool wasAttached, bool wasPowered);
	void notifyNeighbors(Level *level, int x, int y, int z, int dir);
	bool checkCanSurvive(Level *level, int x, int y, int z);

public:
	void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());
	void onRemove(Level *level, int x, int y, int z, int id, int data);
	virtual bool getSignal(LevelSource *level, int x, int y, int z, int dir);
	virtual bool getDirectSignal(Level *level, int x, int y, int z, int dir);
	bool isSignalSource();
};
