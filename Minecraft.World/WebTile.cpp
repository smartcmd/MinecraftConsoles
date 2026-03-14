#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.item.h"
#include "WebTile.h"

WebTile::WebTile(int id) : Tile(id, Material::web)
{
}


void WebTile::entityInside(Level *level, int x, int y, int z, shared_ptr<Entity> entity)
{
	entity->makeStuckInWeb();
}


bool WebTile::isSolidRender(bool isServerLevel)
{
	return false;
}


AABB *WebTile::getAABB(Level *level, int x, int y, int z)
{
	return nullptr;
}


int WebTile::getRenderShape()
{
	return Tile::SHAPE_CROSS_TEXTURE;
}

bool WebTile::blocksLight()
{
	return false;
}

bool WebTile::isCubeShaped()
{
	return false;
}

int WebTile::getResource(int data, Random *random, int playerBonusLevel)
{
    // @TODO: Explosives currently also give string back. Fix?
    return Item::string->id;
}

bool WebTile::isSilkTouchable()
{
	return true;
}