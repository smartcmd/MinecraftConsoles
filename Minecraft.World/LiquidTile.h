#pragma once
#include "Tile.h"
#include "Definitions.h"

class Random;
class ChunkRebuildData;

class LiquidTile : public Tile
{
	friend class ChunkRebuildData;
public:
	static const wstring TEXTURE_LAVA_STILL;
	static const wstring TEXTURE_WATER_STILL;
	static const wstring TEXTURE_WATER_FLOW;
	static const wstring TEXTURE_LAVA_FLOW;

private:
	Icon *icons[2];

protected:
	LiquidTile(int id, Material *material);
public:
	virtual bool isPathfindable(LevelSource *level, int x, int y, int z);
	virtual int getColor() const;
    virtual int getColor(LevelSource *level, int x, int y, int z);
	virtual int getColor(LevelSource *level, int x, int y, int z, int data); // 4J added
    static float getHeight(int d);
    virtual Icon *getTexture(int face, int data);
protected:
	virtual int getDepth(Level *level, int x, int y, int z);
    virtual int getRenderedDepth(LevelSource *level, int x, int y, int z);
public:
	virtual bool isCubeShaped();
	virtual bool isSolidRender(bool isServerLevel = false);
	virtual bool mayPick(int data, bool liquid);
	virtual bool isSolidFace(LevelSource *level, int x, int y, int z, int face);
	virtual bool shouldRenderFace(LevelSource *level, int x, int y, int z, int face);
	virtual AABB *getAABB(Level *level, int x, int y, int z);
	virtual int getRenderShape();
	virtual int getResource(int data, Random *random, int playerBonusLevel);
	virtual int getResourceCount(Random *random);
private:
	virtual Vec3 *getFlow(LevelSource *level, int x, int y, int z);
public:
	virtual void handleEntityInside(Level *level, int x, int y, int z, shared_ptr<Entity> e, Vec3 *current);
	virtual int getTickDelay();
	virtual int getLightColor(LevelSource *level, int x, int y, int z,  int tileId=-1);	// 4J - brought forward from 1.8.2
	virtual float getBrightness(LevelSource *level, int x, int y, int z);
	virtual int getRenderLayer();
	virtual void animateTick(Level *level, int x, int y, int z, Random *random);
	static double getSlopeAngle(LevelSource *level, int x, int y, int z, Material *m);
	virtual void onPlace(Level *level, int x, int y, int z);
	virtual void neighborChanged(Level *level, int x, int y, int z, int type);
private:
	virtual void updateLiquid(Level *level, int x, int y, int z);
protected:
	virtual void fizz(Level *level, int x, int y, int z);

public:
	void registerIcons(IconRegister *iconRegister);
	static Icon *getTexture(const wstring &name);
};