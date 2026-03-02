#pragma once

#include "Minecart.h"

class MinecartTNT : public Minecart
{
public:
	eINSTANCEOF GetType() { return eTYPE_MINECART_TNT; };
	static Entity *create(Level *level) { return new MinecartTNT(level); }

private:
	static const byte EVENT_PRIME = 10;

	int fuse;

	void _init();

public:
	MinecartTNT(Level *level);
	MinecartTNT(Level *level, double x, double y, double z);

	virtual int getType();
	virtual Tile *getDefaultDisplayTile();
	virtual void tick();
	virtual void destroy(DamageSource *source);

protected:
	virtual void explode(double speedSqr);
	virtual void causeFallDamage(float distance);

public:
	virtual void activateMinecart(int xt, int yt, int zt, bool state);
	virtual void handleEntityEvent(byte eventId);
	virtual void primeFuse();
	virtual int getFuse();
	virtual bool isPrimed();
	virtual float getTileExplosionResistance(Explosion *explosion, Level *level, int x, int y, int z, Tile *tile);
	virtual bool shouldTileExplode(Explosion *explosion, Level *level, int x, int y, int z, int id, float power);

protected:
	virtual void readAdditionalSaveData(CompoundTag *tag);
	virtual void addAdditonalSaveData(CompoundTag *tag);
};