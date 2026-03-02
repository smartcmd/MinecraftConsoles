#pragma once

#include "MinecartContainer.h"

class MinecartChest : public MinecartContainer
{
public:
	eINSTANCEOF GetType() { return eTYPE_MINECART_CHEST; };
	static Entity *create(Level *level) { return new MinecartChest(level); }

public:
	MinecartChest(Level *level);
	MinecartChest(Level *level, double x, double y, double z);

	// 4J added
	virtual int getContainerType();

	virtual void destroy(DamageSource *source);
	virtual unsigned int getContainerSize();
	virtual int getType();
	virtual Tile *getDefaultDisplayTile();
	virtual int getDefaultDisplayOffset();
};