#pragma once
#include "MinecartRenderer.h"

class Minecart;

class TntMinecartRenderer : public MinecartRenderer
{
protected:
    void renderMinecartContents(shared_ptr<Minecart> cart, float a, Tile *tile, int tileData);
};