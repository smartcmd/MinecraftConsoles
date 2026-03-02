#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.phys.h"
#include "HellSandTile.h"


HellSandTile::HellSandTile(int id) : Tile(id, Material::sand)
{
}

AABB *HellSandTile::getAABB(Level *level, int x, int y, int z)
{
    float r = 2 / 16.0f;
    return AABB::newTemp(x, y, z, x + 1, y + 1 - r, z + 1);
}

void HellSandTile::entityInside(Level *level, int x, int y, int z, shared_ptr<Entity> entity)
{
    entity->xd*=0.4;
    entity->zd*=0.4;
}