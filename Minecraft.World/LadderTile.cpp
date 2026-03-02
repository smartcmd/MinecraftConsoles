#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "LadderTile.h"


LadderTile::LadderTile(int id) : Tile(id, Material::decoration,isSolidRender())
{
}

AABB *LadderTile::getAABB(Level *level, int x, int y, int z)
{
	updateShape(level, x, y, z);
	return Tile::getAABB(level, x, y, z);
}

AABB *LadderTile::getTileAABB(Level *level, int x, int y, int z)
{
    updateShape(level, x, y, z);
    return Tile::getTileAABB(level, x, y, z);
}

void LadderTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity) // 4J added forceData, forceEntity param
{
	setShape(level->getData(x, y, z));
}

void LadderTile::setShape(int data)
{
	int dir = data;
	float r = 2 / 16.0f;

	if (dir == 2) setShape(0, 0, 1 - r, 1, 1, 1);
	if (dir == 3) setShape(0, 0, 0, 1, 1, r);
	if (dir == 4) setShape(1 - r, 0, 0, 1, 1, 1);
	if (dir == 5) setShape(0, 0, 0, r, 1, 1);
}

bool LadderTile::blocksLight()
{
	return false;
}

bool LadderTile::isSolidRender(bool isServerLevel)
{
	return false;
}

bool LadderTile::isCubeShaped()
{
	return false;
}

int LadderTile::getRenderShape()
{
	return Tile::SHAPE_LADDER;
}

bool LadderTile::mayPlace(Level *level, int x, int y, int z)
{
    if (level->isSolidBlockingTile(x - 1, y, z))
	{
        return true;
    }
	else if (level->isSolidBlockingTile(x + 1, y, z))
	{
        return true;
	}
	else if (level->isSolidBlockingTile(x, y, z - 1))
	{
        return true;
    }
	else if (level->isSolidBlockingTile(x, y, z + 1))
	{
        return true;
    }
    return false;
}

int LadderTile::getPlacedOnFaceDataValue(Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue)
{
    int dir = level->getData(x, y, z);

    if ((dir == 0 || face == 2) && level->isSolidBlockingTile(x, y, z + 1)) dir = 2;
    if ((dir == 0 || face == 3) && level->isSolidBlockingTile(x, y, z - 1)) dir = 3;
    if ((dir == 0 || face == 4) && level->isSolidBlockingTile(x + 1, y, z)) dir = 4;
    if ((dir == 0 || face == 5) && level->isSolidBlockingTile(x - 1, y, z)) dir = 5;

    return dir;
}

void LadderTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
    int face = level->getData(x, y, z);
    bool ok = false;

    if (face == 2 && level->isSolidBlockingTile(x, y, z + 1)) ok = true;
    if (face == 3 && level->isSolidBlockingTile(x, y, z - 1)) ok = true;
    if (face == 4 && level->isSolidBlockingTile(x + 1, y, z)) ok = true;
    if (face == 5 && level->isSolidBlockingTile(x - 1, y, z)) ok = true;
    if (!ok)
	{
        spawnResources(level, x, y, z, face, 0);
        level->setTile(x, y, z, 0);
    }

    Tile::neighborChanged(level, x, y, z, type);
}

int LadderTile::getResourceCount(Random* random)
{
	return 1;
}