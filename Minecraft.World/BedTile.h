#pragma once
#include "DirectionalTile.h"

class Pos;
class Player;
class Random;
class Level;

class BedTile : public DirectionalTile
{
private:
	static const int PART_FOOT = 0;
	static const int PART_HEAD = 1;

	Icon **iconEnd;
	Icon **iconSide;
	Icon **iconTop;

public:
	static const int HEAD_PIECE_DATA = 0x8;
	static const int OCCUPIED_DATA = 0x4;

	static int HEAD_DIRECTION_OFFSETS[4][2];

	BedTile(int id);

	virtual void updateDefaultShape();
	virtual bool TestUse(Level *level, int x, int y, int z, shared_ptr<Player> player);
	virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false); // 4J added soundOnly param
	virtual Icon *getTexture(int face, int data);
	//@Override
	void registerIcons(IconRegister *iconRegister);
	virtual int getRenderShape();
	virtual bool isCubeShaped();
	virtual bool isSolidRender(bool isServerLevel = false);
	virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());	// 4J added forceData, forceEntity param
	virtual void neighborChanged(Level *level, int x, int y, int z, int type);
	virtual int getResource(int data, Random *random,int playerBonusLevel);

private:
	using Tile::setShape;
	void setShape();

public:
	static bool isHeadPiece(int data);
	static bool isOccupied(int data);
	static void setOccupied(Level *level, int x, int y, int z, bool occupied);
	static Pos *findStandUpPosition(Level *level, int x, int y, int z, int skipCount);

	virtual void spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonus);
	virtual int getPistonPushReaction();
	virtual int cloneTileId(Level *level, int x, int y, int z);
	virtual void playerWillDestroy(Level *level, int x, int y, int z, int data, shared_ptr<Player> player);
};
