#pragma once

#include "Tile.h"
class ChunkRebuildData;
class TntTile : public Tile
{
	friend class ChunkRebuildData;
private:
	Icon *iconTop;
	Icon *iconBottom;
public:
	static const int EXPLODE_BIT = 1;
	TntTile(int id);

	Icon *getTexture(int face, int data);
	virtual void onPlace(Level *level, int x, int y, int z);

	void neighborChanged(Level *level, int x, int y, int z, int type);

	int getResourceCount(Random *random);

	void wasExploded(Level *level, int x, int y, int z);

	void destroy(Level *level, int x, int y, int z, int data);

    bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false); // 4J added soundOnly param

	void entityInside(Level *level, int x, int y, int z, shared_ptr<Entity> entity);
	virtual shared_ptr<ItemInstance> getSilkTouchItemInstance(int data);
	void registerIcons(IconRegister *iconRegister);
};