#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "GrassTile.h"
#include "Bush.h"

void Bush::_init()
{
	setTicking(true);
	updateDefaultShape();
}

Bush::Bush(int id, Material *material) : Tile(id, material, isSolidRender())
{
	_init();
}

Bush::Bush(int id) : Tile(id, Material::plant, isSolidRender())
{
	_init();
}

// 4J Added override
void Bush::updateDefaultShape()
{
	float ss = 0.2f;
	setShape(0.5f - ss, 0, 0.5f - ss, 0.5f + ss, ss * 3, 0.5f + ss);
}

bool Bush::mayPlace(Level *level, int x, int y, int z)
{
	return Tile::mayPlace(level, x, y, z) && mayPlaceOn(level->getTile(x, y - 1, z));
}

bool Bush::mayPlaceOn(int tile)
{
	return tile == Tile::grass_Id || tile == Tile::dirt_Id || tile == Tile::farmland_Id;
}

void Bush::neighborChanged(Level *level, int x, int y, int z, int type)
{
	Tile::neighborChanged(level, x, y, z, type);
	checkAlive(level, x, y, z);
}

void Bush::tick(Level *level, int x, int y, int z, Random *random)
{
	checkAlive(level, x, y, z);
}

void Bush::checkAlive(Level *level, int x, int y, int z)
{
	if (!canSurvive(level, x, y, z))
	{
		this->spawnResources(level, x, y, z, level->getData(x, y, z), 0);
		level->setTileAndData(x, y, z, 0, 0, UPDATE_CLIENTS);
	}
}

bool Bush::canSurvive(Level *level, int x, int y, int z)
{
	return ( level->getDaytimeRawBrightness(x, y, z) >= 8 || (level->canSeeSky(x, y, z))) && mayPlaceOn(level->getTile(x, y - 1, z));
}

AABB *Bush::getAABB(Level *level, int x, int y, int z)
{
	return NULL;
}

bool Bush::blocksLight()
{
	return false;
}

bool Bush::isSolidRender(bool isServerLevel)
{
	return false;
}

bool Bush::isCubeShaped()
{
	return false;
}

int Bush::getRenderShape()
{
	return Tile::SHAPE_CROSS_TEXTURE;
}
