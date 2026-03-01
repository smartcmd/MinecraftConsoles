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

	virtual Icon *getTexture(int face, int data);
	virtual void onPlace(Level *level, int x, int y, int z);
	virtual void neighborChanged(Level *level, int x, int y, int z, int type);
	virtual int getResourceCount(Random *random);
	virtual void wasExploded(Level *level, int x, int y, int z, Explosion *explosion);
	virtual void destroy(Level *level, int x, int y, int z, int data);
	virtual void destroy(Level *level, int x, int y, int z, int data, shared_ptr<LivingEntity> source);
	virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false); // 4J added soundOnly param
	virtual void entityInside(Level *level, int x, int y, int z, shared_ptr<Entity> entity);
	virtual bool dropFromExplosion(Explosion *explosion);
	virtual void registerIcons(IconRegister *iconRegister);
};