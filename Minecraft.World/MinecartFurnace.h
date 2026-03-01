#pragma once

#include "Minecart.h"

class MinecartFurnace : public Minecart
{
public:
	eINSTANCEOF GetType() { return eTYPE_MINECART_FURNACE; };
	static Entity *create(Level *level) { return new MinecartFurnace(level); }

private:
	static const int DATA_ID_FUEL = 16;

private:
	int fuel;

public:
	double xPush, zPush;

	MinecartFurnace(Level *level);
	MinecartFurnace(Level *level, double x, double y, double z);

	// 4J added
	virtual int getContainerType();

	int getType();

protected:
	void defineSynchedData();

public:
	void tick();
	void destroy(DamageSource *source);

protected:
	void moveAlongTrack(int xt, int yt, int zt, double maxSpeed, double slideSpeed, int tile, int data);
	void applyNaturalSlowdown();

public:
	bool interact(shared_ptr<Player> player);

protected:
	void addAdditonalSaveData(CompoundTag *base);
	void readAdditionalSaveData(CompoundTag *base);
	bool hasFuel();
	void setHasFuel(bool fuel);

public:
	Tile *getDefaultDisplayTile();
	int getDefaultDisplayData();
};