#pragma once
#include "Tile.h"

class TheEndPortalFrameTile : public Tile
{
public:
	static const int EYE_BIT = 4;
	static const wstring TEXTURE_EYE;

private:
	Icon *iconTop;
	Icon *iconEye;

public:
	TheEndPortalFrameTile(int id);
	virtual Icon *getTexture(int face, int data);
	void registerIcons(IconRegister *iconRegister);
	Icon *getEye();
	virtual bool isSolidRender(bool isServerLevel = false);
	virtual int getRenderShape();
	virtual void updateDefaultShape();
	virtual void addAABBs(Level *level, int x, int y, int z, AABB *box, AABBList *boxes, shared_ptr<Entity> source);
	static bool hasEye(int data);
	virtual int getResource(int data, Random *random, int playerBonusLevel);
	virtual void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance);
	virtual bool hasAnalogOutputSignal();
	virtual int getAnalogOutputSignal(Level *level, int x, int y, int z, int dir);
};