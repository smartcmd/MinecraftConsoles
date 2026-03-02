#pragma once

#include "DirectionalTile.h"

class CocoaTile : public DirectionalTile
{
public:	
	static const int COCOA_TEXTURES_LENGTH = 3;
	static const wstring TEXTURE_AGES[];

private:
	Icon **icons;

public:
	using Tile::setPlacedBy;

	CocoaTile(int id);

	virtual Icon *getTexture(int face, int data);
	virtual Icon *getTextureForAge(int age);
	virtual void tick(Level *level, int x, int y, int z, Random *random);
	virtual bool canSurvive(Level *level, int x, int y, int z);
	virtual int getRenderShape();
	virtual bool isCubeShaped();
	virtual bool isSolidRender(bool isServerLevel = false);
	virtual AABB *getAABB(Level *level, int x, int y, int z);
	virtual AABB *getTileAABB(Level *level, int x, int y, int z);
	virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());
	virtual void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<Mob> by, shared_ptr<ItemInstance> itemInstance);
	virtual int getPlacedOnFaceDataValue(Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue);
	virtual void neighborChanged(Level *level, int x, int y, int z, int type);
	static int getAge(int data);
	using DirectionalTile::spawnResources;
	virtual void spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonusLevel);
	virtual int cloneTileId(Level *level, int x, int y, int z);
	virtual int cloneTileData(Level *level, int x, int y, int z);
	virtual void registerIcons(IconRegister *iconRegister);
};
