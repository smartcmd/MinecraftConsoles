#pragma once
#include "EntityTile.h"

class IconRegister;
class ChunkRebuildData;

class BrewingStandTile : public EntityTile
{
	friend ChunkRebuildData;
public:
	static const wstring TEXTURE_BASE;
private:
	Random *random;
	Icon *iconBase;

public:
	BrewingStandTile(int id);
	~BrewingStandTile();
    virtual bool isSolidRender(bool isServerLevel = false);
    virtual int getRenderShape();
    virtual shared_ptr<TileEntity> newTileEntity(Level *level);
    virtual bool isCubeShaped();
    virtual void addAABBs(Level *level, int x, int y, int z, AABB *box, AABBList *boxes, shared_ptr<Entity> source);
    virtual void updateDefaultShape();
    virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false); // 4J added soundOnly param
    virtual void animateTick(Level *level, int xt, int yt, int zt, Random *random);
    virtual void onRemove(Level *level, int x, int y, int z, int id, int data);
    virtual int getResource(int data, Random *random, int playerBonusLevel);
	virtual int cloneTileId(Level *level, int x, int y, int z);
	void registerIcons(IconRegister *iconRegister);
	Icon *getBaseTexture();
};