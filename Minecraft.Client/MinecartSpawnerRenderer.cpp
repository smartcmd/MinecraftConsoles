#include "stdafx.h"
#include "MinecartSpawnerRenderer.h"
#include "../Minecraft.World/Tile.h"
#include "../Minecraft.World/net.minecraft.world.entity.item.h"
#include "MobSpawnerRenderer.h"

void MinecartSpawnerRenderer::renderMinecartContents(shared_ptr<MinecartSpawner> cart, float a, Tile *tile, int tileData)
{
    MinecartRenderer::renderMinecartContents(cart, a, tile, tileData);

    if (tile == Tile::mobSpawner)
	{
        MobSpawnerRenderer::render(cart->getSpawner(), cart->x, cart->y, cart->z, a);
    }
}