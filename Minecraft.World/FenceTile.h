#pragma once
#include "Tile.h"
#include "Definitions.h"

class FenceTile : public Tile
{
private:
	wstring texture;

public:
	FenceTile(int id, const wstring &texture, Material *material);
	virtual AABB *getAABB(Level *level, int x, int y, int z);
	virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>()); // 4J added forceData, forceEntity param
	virtual bool blocksLight();
	virtual bool isSolidRender(bool isServerLevel = false);
	virtual bool isCubeShaped();
	virtual bool isPathfindable(LevelSource *level, int x, int y, int z);
	virtual bool shouldRenderFace(LevelSource *level, int x, int y, int z, int face);
	virtual int getRenderShape();
	bool connectsTo(LevelSource *level, int x, int y, int z);
	static bool isFence(int tile);
	void registerIcons(IconRegister *iconRegister);
};