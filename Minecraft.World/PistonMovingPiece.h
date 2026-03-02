#include "EntityTile.h"

class PistonPieceEntity;

class PistonMovingPiece : public EntityTile
{
public:
	PistonMovingPiece(int id);

protected:
	virtual shared_ptr<TileEntity> newTileEntity(Level *level);

public:
	virtual void onPlace(Level *level, int x, int y, int z);
    virtual void onRemove(Level *level, int x, int y, int z, int id, int data);
    virtual bool mayPlace(Level *level, int x, int y, int z);
    virtual bool mayPlace(Level *level, int x, int y, int z, int face);
    virtual int getRenderShape();
    virtual bool isSolidRender(bool isServerLevel = false);
    virtual bool isCubeShaped();
    virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false); // 4J added soundOnly param;
    virtual int getResource(int data, Random *random, int playerBonusLevel);
    virtual void spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonus);
    virtual void neighborChanged(Level *level, int x, int y, int z, int type);
    static shared_ptr<TileEntity> newMovingPieceEntity(int block, int data, int facing, bool extending, bool isSourcePiston);
    virtual AABB *getAABB(Level *level, int x, int y, int z);
    virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());	// 4J added forceData, forceEntity param

    AABB *getAABB(Level *level, int x, int y, int z, int tile, float progress, int facing);

private:
	shared_ptr<PistonPieceEntity> getEntity(LevelSource *level, int x, int y, int z);

public:
	virtual int cloneTileId(Level *level, int x, int y, int z);
	void registerIcons(IconRegister *iconRegister);
};