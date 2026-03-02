#pragma once
#include "Tile.h"
#include "Definitions.h"

class Player;
class HitResult;
class ChunkRebuildData;

class DoorTile : public Tile
{
	friend class Tile;
	friend class ChunkRebuildData;
public:
	static const int UPPER_BIT = 8;
	static const int C_DIR_MASK = 3;
	static const int C_OPEN_MASK = 4;
	static const int C_LOWER_DATA_MASK = 7;
	static const int C_IS_UPPER_MASK = 8;
	static const int C_RIGHT_HINGE_MASK = 16;

private:
	static const int DOOR_TILE_TEXTURE_COUNT = 4;
	static const wstring TEXTURES[];
	int texBase;
	Icon **icons;

protected:
	DoorTile(int id, Material *material);
public:
	virtual Icon *getTexture(int face, int data);
	//@Override
	Icon *getTexture(LevelSource *level, int x, int y, int z, int face);
	//@Override
	void registerIcons(IconRegister *iconRegister);
	virtual bool blocksLight();
	virtual bool isSolidRender(bool isServerLevel = false);
	virtual bool isCubeShaped();
	virtual int getRenderShape();
	virtual AABB *getTileAABB(Level *level, int x, int y, int z);
	virtual AABB *getAABB(Level *level, int x, int y, int z);
	virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());	// 4J added forceData, forceEntity param
	int getDir(LevelSource *level, int x, int y, int z);
	bool isOpen(LevelSource *level, int x, int y, int z);
private:
	using Tile::setShape;
	virtual void setShape(int compositeData);
public:
	virtual void attack(Level *level, int x, int y, int z, shared_ptr<Player> player);
	virtual bool TestUse();
	virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false); // 4J added soundOnly param
	void setOpen(Level *level, int x, int y, int z, bool shouldOpen);
	virtual void neighborChanged(Level *level, int x, int y, int z, int type);
	virtual int getResource(int data, Random *random, int playerBonusLevel);
	virtual HitResult *clip(Level *level, int xt, int yt, int zt, Vec3 *a, Vec3 *b);
	virtual bool mayPlace(Level *level, int x, int y, int z);
	static bool isOpen(int data);
	virtual int getPistonPushReaction();
	int getCompositeData(LevelSource *level, int x, int y, int z);
	virtual int cloneTileId(Level *level, int x, int y, int z);
};
