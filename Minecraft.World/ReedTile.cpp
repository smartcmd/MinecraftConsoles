#include "stdafx.h"
#include "GrassTile.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.material.h"
#include "net.minecraft.world.phys.h"
#include "ReedTile.h"

ReedTile::ReedTile(int id) : Tile( id, Material::plant,isSolidRender() )
{
	this->updateDefaultShape();
	this->setTicking(true);
}

// 4J Added override
void ReedTile::updateDefaultShape()
{
	float ss = 6 / 16.0f;
	this->setShape(0.5f - ss, 0, 0.5f - ss, 0.5f + ss, 1, 0.5f + ss);
}

void ReedTile::tick(Level *level, int x, int y, int z, Random* random)
{
	if (level->isEmptyTile(x, y + 1, z))
	{
		int height = 1;
		while (level->getTile(x, y - height, z) == id)
		{
			height++;
		}
		if (height < 3)
		{
			int age = level->getData(x, y, z);
			if (age == 15) 
			{
				level->setTileAndUpdate(x, y + 1, z, id);
				level->setData(x, y, z, 0, Tile::UPDATE_NONE);
			} 
			else 
			{
				level->setData(x, y, z, age + 1, Tile::UPDATE_NONE);
			}
		}
	}
}

bool ReedTile::mayPlace(Level *level, int x, int y, int z) 
{
	int below = level->getTile(x, y - 1, z);
	if (below == id) return true;
	if (below != Tile::grass_Id && below != Tile::dirt_Id && below != Tile::sand_Id) return false;
	if (level->getMaterial(x - 1, y - 1, z) == Material::water) return true;
	if (level->getMaterial(x + 1, y - 1, z) == Material::water) return true;
	if (level->getMaterial(x, y - 1, z - 1) == Material::water) return true;
	if (level->getMaterial(x, y - 1, z + 1) == Material::water) return true;
	//printf("no water\n");
	return false;
}

void ReedTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	checkAlive(level, x, y, z);
}

const void ReedTile::checkAlive(Level *level, int x, int y, int z)
{
	if (!canSurvive(level, x, y, z)) 
	{
		spawnResources(level, x, y, z, level->getData(x, y, z), 0);
		level->removeTile(x, y, z);
	}
}

bool ReedTile::canSurvive(Level *level, int x, int y, int z)
{
	return mayPlace(level, x, y, z);
}

AABB *ReedTile::getAABB(Level *level, int x, int y, int z)
{
	return nullptr;
}

int ReedTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return Item::reeds->id;
}

bool ReedTile::blocksLight()
{
	return false;
}

bool ReedTile::isSolidRender(bool isServerLevel)
{
	return false;
}

bool ReedTile::isCubeShaped() 
{
	return false;
}

int ReedTile::getRenderShape()
{
	return Tile::SHAPE_CROSS_TEXTURE;
}

int ReedTile::cloneTileId(Level *level, int x, int y, int z)
{
	return Item::reeds_Id;
}

bool ReedTile::shouldTileTick(Level *level, int x,int y,int z)
{
	return level->isEmptyTile(x, y + 1, z);
}
