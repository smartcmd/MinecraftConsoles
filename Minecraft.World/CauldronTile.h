#pragma once
#include "Tile.h"

class IconRegister;

class CauldronTile : public Tile
{
public:
	static const wstring TEXTURE_INSIDE;
	static const wstring TEXTURE_BOTTOM;

private:
	Icon *iconInner;
	Icon *iconTop;
	Icon *iconBottom;

public:
	CauldronTile(int id);
	using Tile::getTexture;
	virtual Icon *getTexture(int face, int data);
	//@Override
	void registerIcons(IconRegister *iconRegister);
	static Icon *getTexture(const wstring &name);
	virtual void addAABBs(Level *level, int x, int y, int z, AABB *box, AABBList *boxes, shared_ptr<Entity> source);
	virtual void updateDefaultShape();
	virtual bool isSolidRender(bool isServerLevel = false);
	virtual int getRenderShape();
	virtual bool isCubeShaped();
	virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false); // 4J added soundOnly param
	virtual void handleRain(Level *level, int x, int y, int z);
	virtual int getResource(int data, Random *random, int playerBonusLevel);
	virtual int cloneTileId(Level *level, int x, int y, int z);
	virtual bool hasAnalogOutputSignal();
	virtual int getAnalogOutputSignal(Level *level, int x, int y, int z, int dir);
	static int getFillLevel(int data);
};
