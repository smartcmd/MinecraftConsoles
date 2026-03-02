#include "stdafx.h"
#include "HalfSlabTile.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.biome.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.stats.h"
#include "Facing.h"


HalfSlabTile::HalfSlabTile(int id, bool fullSize, Material *material) : Tile(id, material, fullSize)
{
	this->fullSize = fullSize;

	if (fullSize) 
	{
		solid[id] = true;
	} 
	else 
	{
		setShape(0, 0, 0, 1, 0.5f, 1);
	}
	setLightBlock(255);
}

void HalfSlabTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity)	// 4J added forceData, forceEntity param
{
	if (fullSize) 
	{
		setShape(0, 0, 0, 1, 1, 1);
	} 
	else 
	{
		bool upper = (level->getData(x, y, z) & TOP_SLOT_BIT) != 0;
		if (upper) 
		{
			setShape(0, 0.5f, 0, 1, 1, 1);
		} 
		else 
		{
			setShape(0, 0, 0, 1, 0.5f, 1);
		}
	}
}

void HalfSlabTile::updateDefaultShape() 
{
	if (fullSize) 
	{
		setShape(0, 0, 0, 1, 1, 1);
	} 
	else 
	{
		setShape(0, 0, 0, 1, 0.5f, 1);
	}
}

void HalfSlabTile::addAABBs(Level *level, int x, int y, int z, AABB *box, AABBList *boxes, shared_ptr<Entity> source) 
{
	updateShape(level, x, y, z);
	Tile::addAABBs(level, x, y, z, box, boxes, source);
}

bool HalfSlabTile::isSolidRender(bool isServerLevel) 
{
	return fullSize;
}

int HalfSlabTile::getPlacedOnFaceDataValue(Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue)
{
	if (fullSize) return itemValue;

	if (face == Facing::DOWN || (face != Facing::UP && clickY > 0.5))
	{
		return itemValue | TOP_SLOT_BIT;
	}
	return itemValue;
}

int HalfSlabTile::getResourceCount(Random *random) 
{
	if (fullSize) 
	{
		return 2;
	}
	return 1;
}

int HalfSlabTile::getSpawnResourcesAuxValue(int data) 
{
	return data & TYPE_MASK;
}

bool HalfSlabTile::isCubeShaped() 
{
	return fullSize;
}

bool HalfSlabTile::shouldRenderFace(LevelSource *level, int x, int y, int z, int face) 
{
	if (fullSize) return Tile::shouldRenderFace(level, x, y, z, face);

	if (face != Facing::UP && face != Facing::DOWN && !Tile::shouldRenderFace(level, x, y, z, face)) 
	{
		return false;
	}

	int ox = x, oy = y, oz = z;
	ox += Facing::STEP_X[Facing::OPPOSITE_FACING[face]];
	oy += Facing::STEP_Y[Facing::OPPOSITE_FACING[face]];
	oz += Facing::STEP_Z[Facing::OPPOSITE_FACING[face]];

	boolean isUpper = (level->getData(ox, oy, oz) & TOP_SLOT_BIT) != 0;
	if (isUpper) 
	{
		if (face == Facing::DOWN) return true;
		if (face == Facing::UP && Tile::shouldRenderFace(level, x, y, z, face)) return true;
		return !(isHalfSlab(level->getTile(x, y, z)) && (level->getData(x, y, z) & TOP_SLOT_BIT) != 0);
	} 
	else 
	{
		if (face == Facing::UP) return true;
		if (face == Facing::DOWN && Tile::shouldRenderFace(level, x, y, z, face)) return true;
		return !(isHalfSlab(level->getTile(x, y, z)) && (level->getData(x, y, z) & TOP_SLOT_BIT) == 0);
	}
}

bool HalfSlabTile::isHalfSlab(int tileId) 
{
	return tileId == Tile::stoneSlabHalf_Id || tileId == Tile::woodSlabHalf_Id;
}

int HalfSlabTile::cloneTileData(Level *level, int x, int y, int z)
{
	return Tile::cloneTileData(level, x, y, z) & TYPE_MASK;
}

int HalfSlabTile::cloneTileId(Level *level, int x, int y, int z)
{
	if (isHalfSlab(id))
	{
		return id;
	}
	if (id == Tile::stoneSlab_Id)
	{
		return Tile::stoneSlabHalf_Id;
	}
	if (id == Tile::woodSlab_Id)
	{
		return Tile::woodSlabHalf_Id;
	}
	return Tile::stoneSlabHalf_Id;
}