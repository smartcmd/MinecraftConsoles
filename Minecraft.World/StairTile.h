#pragma once

#include "Tile.h"

class Mob;
class Player;

class StairTile : public Tile
{
	friend class Tile;

private:
	static const int DEAD_SPACE_COLUMN_COUNT = 2;
	static int DEAD_SPACES[8][DEAD_SPACE_COLUMN_COUNT];

public:
	static const int UPSIDEDOWN_BIT = 4;

	// the direction is the way going up (for normal non-upsidedown stairs)
	static const int DIR_EAST = 0;
	static const int DIR_WEST = 1;
	static const int DIR_SOUTH = 2;
	static const int DIR_NORTH = 3;

private:
	Tile *base;
	int basedata;
	bool isClipping;
	int clipStep;

protected:
	StairTile(int id, Tile *base, int basedata);

public:
	void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());	// 4J added forceData, forceEntity param
	bool isSolidRender(bool isServerLevel = false);
	bool isCubeShaped();
	int getRenderShape();
	void setBaseShape(LevelSource *level, int x, int y, int z);
	static bool isStairs(int id);

private:
	bool isLockAttached(LevelSource *level, int x, int y, int z, int data);

public:
	bool setStepShape(LevelSource *level, int x, int y, int z);
	bool setInnerPieceShape(LevelSource *level, int x, int y, int z);
	void addAABBs(Level *level, int x, int y, int z, AABB *box, AABBList *boxes, shared_ptr<Entity> source);


	/** DELEGATES: **/
public:
	virtual void addLights(Level *level, int x, int y, int z);
	virtual void animateTick(Level *level, int x, int y, int z, Random *random);
	virtual void attack(Level *level, int x, int y, int z, shared_ptr<Player> player);
	virtual void destroy(Level *level, int x, int y, int z, int data);
	virtual int getLightColor(LevelSource *level, int x, int y, int z, int tileId = -1);
	virtual float getBrightness(LevelSource *level, int x, int y, int z);
	virtual float getExplosionResistance(shared_ptr<Entity> source);
	virtual int getRenderLayer();
	virtual Icon *getTexture(int face, int data);
	virtual int getTickDelay(Level *level);
	virtual AABB *getTileAABB(Level *level, int x, int y, int z);
	virtual void handleEntityInside(Level *level, int x, int y, int z, shared_ptr<Entity> e, Vec3 *current);
	virtual bool mayPick();
	virtual bool mayPick(int data, bool liquid);
	virtual bool mayPlace(Level *level, int x, int y, int z);
	virtual void onPlace(Level *level, int x, int y, int z);
	virtual void onRemove(Level *level, int x, int y, int z, int id, int data);
	virtual void prepareRender(Level *level, int x, int y, int z);
	virtual void stepOn(Level *level, int x, int y, int z, shared_ptr<Entity> entity);
	virtual void tick(Level *level, int x, int y, int z, Random *random);
	virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false); // 4J added soundOnly param
	virtual void wasExploded(Level *level, int x, int y, int z, Explosion *explosion);
	virtual void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance);
	virtual int getPlacedOnFaceDataValue(Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue);
	virtual HitResult *clip(Level *level, int xt, int yt, int zt, Vec3 *a, Vec3 *b);
	virtual void registerIcons(IconRegister *iconRegister);
};
