#pragma once
#include "BaseEntityTile.h"
#include "net.minecraft.core.h"

class Player;
class Mob;
class ChunkRebuildData;

class DispenserTile : public BaseEntityTile
{
	friend class Tile;
	friend class ChunkRebuildData;

public:
	static const int FACING_MASK = 0x7;
	static const int TRIGGER_BIT = 8;
	static BehaviorRegistry REGISTRY;

protected:
	Random *random;

	Icon *iconTop;
	Icon *iconFront;
	Icon *iconFrontVertical;

protected:
	DispenserTile(int id);

public:
	virtual int getTickDelay(Level *level);
	virtual void onPlace(Level *level, int x, int y, int z);

private:
	void recalcLockDir(Level *level, int x, int y, int z);

public:
	virtual Icon *getTexture(int face, int data);
	virtual void registerIcons(IconRegister *iconRegister);
	virtual bool TestUse();
	virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false); // 4J added soundOnly param

protected:
	virtual void dispenseFrom(Level *level, int x, int y, int z);
	virtual DispenseItemBehavior *getDispenseMethod(shared_ptr<ItemInstance> item);

public:
	virtual void neighborChanged(Level *level, int x, int y, int z, int type);
	virtual void tick(Level *level, int x, int y, int z, Random *random);
	virtual shared_ptr<TileEntity> newTileEntity(Level *level);
	virtual void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance);
	virtual void onRemove(Level *level, int x, int y, int z, int id, int data);

	static Position *getDispensePosition(BlockSource *source);
	static FacingEnum *getFacing(int data);
	virtual bool hasAnalogOutputSignal();
	virtual int getAnalogOutputSignal(Level *level, int x, int y, int z, int dir);
};