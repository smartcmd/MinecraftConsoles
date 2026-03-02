#pragma once

#include "HeavyTile.h"

class FallingTile;

class AnvilTile : public HeavyTile
{
	friend class ChunkRebuildData;
	friend class Tile;
public:
	static const int PART_BASE = 0;
	static const int PART_JOINT = 1;
	static const int PART_COLUMN = 2;
	static const int PART_TOP = 3;

	static const int ANVIL_NAMES_LENGTH = 3;

	static const unsigned int ANVIL_NAMES[ANVIL_NAMES_LENGTH];

private:
	static wstring TEXTURE_DAMAGE_NAMES[ANVIL_NAMES_LENGTH];

public:
	int part;

private:
	Icon **icons;

protected:
	AnvilTile(int id);

public:
	bool isCubeShaped();
	bool isSolidRender(bool isServerLevel = false);
	Icon *getTexture(int face, int data);
	void registerIcons(IconRegister *iconRegister);
	void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance);
	bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false);
	int getRenderShape();
	int getSpawnResourcesAuxValue(int data);
	void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());

protected:
	void falling(shared_ptr<FallingTile> entity);

public:
	void onLand(Level *level, int xt, int yt, int zt, int data);
	bool shouldRenderFace(LevelSource *level, int x, int y, int z, int face);
};