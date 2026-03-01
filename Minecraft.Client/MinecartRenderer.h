#pragma once
#include "EntityRenderer.h"

class Minecart;

class MinecartRenderer : public EntityRenderer
{
private:
	static ResourceLocation MINECART_LOCATION;

protected:
	Model *model;
	TileRenderer *renderer;

public:
	MinecartRenderer();
	virtual void render(shared_ptr<Entity> _cart, double x, double y, double z, float rot, float a);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);

protected:
	virtual void renderMinecartContents(shared_ptr<Minecart> cart, float a, Tile *tile, int tileData);
};