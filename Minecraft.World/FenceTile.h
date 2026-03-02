#pragma once
#include "Tile.h"
#include "Definitions.h"

class FenceTile : public Tile
{
private:
	wstring texture;

public:
	FenceTile(int id, const wstring &texture, Material *material);
	virtual void addAABBs(Level *level, int x, int y, int z, AABB *box, AABBList *boxes, shared_ptr<Entity> source);
	virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>()); // 4J added forceData, forceEntity param
	virtual bool isSolidRender(bool isServerLevel = false);
	virtual bool isCubeShaped();
	virtual bool isPathfindable(LevelSource *level, int x, int y, int z);
	virtual bool shouldRenderFace(LevelSource *level, int x, int y, int z, int face);
	virtual int getRenderShape();
	virtual bool connectsTo(LevelSource *level, int x, int y, int z);
	static bool isFence(int tile);
	virtual void registerIcons(IconRegister *iconRegister);
	virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false);
};