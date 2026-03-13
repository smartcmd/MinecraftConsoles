#include "stdafx.h"
#include "EntityTileRenderer.h"
#include "TileEntityRenderDispatcher.h"
#include "../Minecraft.World/net.minecraft.world.level.tile.h"
#include "../Minecraft.World/net.minecraft.world.level.tile.entity.h"

EntityTileRenderer *EntityTileRenderer::instance = new EntityTileRenderer;

EntityTileRenderer::EntityTileRenderer()
{
	chest = std::make_shared<ChestTileEntity>();
	trappedChest = std::make_shared<ChestTileEntity>(ChestTile::TYPE_TRAP);
	enderChest = std::make_shared<EnderChestTileEntity>();
}

void EntityTileRenderer::render(Tile *tile, int data, float brightness, float alpha, bool setColor, bool useCompiled)
{
	if (tile->id == Tile::enderChest_Id)
	{
		TileEntityRenderDispatcher::instance->render(enderChest, 0, 0, 0, 0, setColor, alpha, useCompiled);
	}
	else if (tile->id == Tile::chest_trap_Id)
	{
		TileEntityRenderDispatcher::instance->render(trappedChest, 0, 0, 0, 0, setColor, alpha, useCompiled);
	}
	else
	{
		TileEntityRenderDispatcher::instance->render(chest, 0, 0, 0, 0, setColor, alpha, useCompiled);
	}
}
