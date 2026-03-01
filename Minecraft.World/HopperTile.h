#pragma once

#include "BaseEntityTile.h"

class HopperTileEntity;

class HopperTile : public BaseEntityTile
{
	friend class ChunkRebuildData;
private:
	static const int MASK_TOGGLE = 0x8;
	static const int MASK_ATTACHED = 0x7;

public :
	static const wstring TEXTURE_OUTSIDE;
	static const wstring TEXTURE_INSIDE;

private:
	Random random;

private:
	Icon *hopperIcon;
	Icon *hopperTopIcon;
	Icon *hopperInnerIcon;

public:
	HopperTile(int id);

	virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());
	virtual void addAABBs(Level *level, int x, int y, int z, AABB *box, AABBList *boxes, shared_ptr<Entity> source);
	virtual int getPlacedOnFaceDataValue(Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue);
	virtual shared_ptr<TileEntity> newTileEntity(Level *level);
	virtual void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance);
	virtual void onPlace(Level *level, int x, int y, int z);
	virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false);
	virtual void neighborChanged(Level *level, int x, int y, int z, int type);

private:
	virtual void checkPoweredState(Level *level, int x, int y, int z);

public:
	virtual void onRemove(Level *level, int x, int y, int z, int id, int data);
	virtual int getRenderShape();
	virtual bool isCubeShaped();
	virtual bool isSolidRender(bool isServerLevel = false);
	virtual bool shouldRenderFace(LevelSource *level, int x, int y, int z, int face);
	virtual Icon *getTexture(int face, int data);
	static int getAttachedFace(int data);
	static bool isTurnedOn(int data);
	virtual bool hasAnalogOutputSignal();
	virtual int getAnalogOutputSignal(Level *level, int x, int y, int z, int dir);
	virtual void registerIcons(IconRegister *iconRegister);
	static Icon *getTexture(const wstring &name);
	virtual wstring getTileItemIconName();
	static shared_ptr<HopperTileEntity> getHopper(LevelSource *level, int x, int y, int z);
};