#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.h"
#include "net.minecraft.h"
#include "WallTile.h"

const float WallTile::WALL_WIDTH = 3.0f / 16.0f;
const float WallTile::WALL_HEIGHT = 13.0f / 16.0f;
const float WallTile::POST_WIDTH = 4.0f / 16.0f;
const float WallTile::POST_HEIGHT = 16.0f / 16.0f;

const unsigned int WallTile::COBBLE_NAMES[2] = { IDS_TILE_COBBLESTONE_WALL,
	IDS_TILE_COBBLESTONE_WALL_MOSSY,
};

WallTile::WallTile(int id, Tile *baseTile) : Tile(id, baseTile->material, isSolidRender())
{
	setDestroyTime(baseTile->destroySpeed);
	setExplodeable(baseTile->explosionResistance / 3);
	setSoundType(baseTile->soundType);
}

Icon *WallTile::getTexture(int face, int data)
{
	if (data == TYPE_MOSSY)
	{
		return Tile::mossyCobblestone->getTexture(face);
	}
	return Tile::cobblestone->getTexture(face);
}

int WallTile::getRenderShape()
{
	return SHAPE_WALL;
}

bool WallTile::isCubeShaped()
{
	return false;
}

bool WallTile::isPathfindable(LevelSource *level, int x, int y, int z)
{
	return false;
}

bool WallTile::isSolidRender(bool isServerLevel)
{
	return false;
}

void WallTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity)
{
	bool n = connectsTo(level, x, y, z - 1);
	bool s = connectsTo(level, x, y, z + 1);
	bool w = connectsTo(level, x - 1, y, z);
	bool e = connectsTo(level, x + 1, y, z);

	float west = .5f - POST_WIDTH;
	float east = .5f + POST_WIDTH;
	float north = .5f - POST_WIDTH;
	float south = .5f + POST_WIDTH;
	float up = POST_HEIGHT;

	if (n)
	{
		north = 0;
	}
	if (s)
	{
		south = 1;
	}
	if (w)
	{
		west = 0;
	}
	if (e)
	{
		east = 1;
	}

	if (n && s && !w && !e)
	{
		up = WALL_HEIGHT;
		west = .5f - WALL_WIDTH;
		east = .5f + WALL_WIDTH;
	}
	else if (!n && !s && w && e)
	{
		up = WALL_HEIGHT;
		north = .5f - WALL_WIDTH;
		south = .5f + WALL_WIDTH;
	}

	setShape(west, 0, north, east, up, south);
}

AABB *WallTile::getAABB(Level *level, int x, int y, int z)
{
	// 4J-JEV: Changed to avoid race conditions associated with calling update shape.

	bool n = connectsTo(level, x, y, z - 1);
	bool s = connectsTo(level, x, y, z + 1);
	bool w = connectsTo(level, x - 1, y, z);
	bool e = connectsTo(level, x + 1, y, z);

	float west = .5f - POST_WIDTH;
	float east = .5f + POST_WIDTH;
	float north = .5f - POST_WIDTH;
	float south = .5f + POST_WIDTH;
	float up = POST_HEIGHT;

	if (n)
	{
		north = 0;
	}
	if (s)
	{
		south = 1;
	}
	if (w)
	{
		west = 0;
	}
	if (e)
	{
		east = 1;
	}

	/*	4J-JEV:
		Stopping the width changing here, it's causing cows/mobs/passers-by to 'jump' up when they are pressed against the
		wall and then the wall section is upgraded to a wall post expanding the bounding box. 
		It's only a 1/16 of a block difference, it shouldn't matter if we leave it a little larger.
	*/
	if (n && s && !w && !e)
	{
		up = WALL_HEIGHT;
		//west = .5f - WALL_WIDTH;
		//east = .5f + WALL_WIDTH;
	}
	else if (!n && !s && w && e)
	{
		up = WALL_HEIGHT;
		//north = .5f - WALL_WIDTH;
		//south = .5f + WALL_WIDTH;
	}

	return AABB::newTemp(x+west, y, z+north, x+east, y+1.5f, z+south);
}


bool WallTile::connectsTo(LevelSource *level, int x, int y, int z)
{
	int tile = level->getTile(x, y, z);
	if (tile == id || tile == Tile::fenceGate_Id)
	{
		return true;
	}
	Tile *tileInstance = Tile::tiles[tile];
	if (tileInstance != NULL)
	{
		if (tileInstance->material->isSolidBlocking() && tileInstance->isCubeShaped())
		{
			return tileInstance->material != Material::vegetable;
		}
	}
	return false;
}

int WallTile::getSpawnResourcesAuxValue(int data)
{
	return data;
}

bool WallTile::shouldRenderFace(LevelSource *level, int x, int y, int z, int face)
{
	if (face == Facing::DOWN)
	{
		return Tile::shouldRenderFace(level, x, y, z, face);
	}
	return true;
}

void WallTile::registerIcons(IconRegister *iconRegister)
{
	// None
}
