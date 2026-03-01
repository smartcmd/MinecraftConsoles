#pragma once
#include "Tile.h"
#include "Definitions.h"

class ThinFenceTile : public Tile
{
private:
	wstring edgeTexture;
	bool dropsResources;
	wstring texture;
	Icon *iconSide;

public:
	ThinFenceTile(int id, const wstring &tex, const wstring &edgeTex, Material *material, bool dropsResources);
	virtual int getResource(int data, Random *random, int playerBonusLevel); 
	virtual bool isSolidRender(bool isServerLevel = false);
    virtual bool isCubeShaped();
    virtual int getRenderShape();
    virtual bool shouldRenderFace(LevelSource *level, int x, int y, int z, int face);
    virtual void addAABBs(Level *level, int x, int y, int z, AABB *box, AABBList *boxes, shared_ptr<Entity> source);
    virtual void updateDefaultShape();
    virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());	// 4J added forceData, forceEntity param
    virtual Icon *getEdgeTexture();
    bool attachsTo(int tile);

protected:
	bool isSilkTouchable();
	shared_ptr<ItemInstance> getSilkTouchItemInstance(int data);

public:
	virtual void registerIcons(IconRegister *iconRegister);
};
