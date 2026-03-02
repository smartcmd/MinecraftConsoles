#include "stdafx.h"
#include "PistonExtensionTile.h"
#include "PistonBaseTile.h"
#include "Facing.h"
#include "net.minecraft.world.level.h"

PistonExtensionTile::PistonExtensionTile(int id) : Tile(id, Material::piston,isSolidRender() )
{
	// 4J added initialiser
	overrideTopTexture = NULL;

	setSoundType(SOUND_STONE);
	setDestroyTime(0.5f);
}

void PistonExtensionTile::setOverrideTopTexture(Icon *overrideTopTexture)
{
	this->overrideTopTexture = overrideTopTexture;
}

void PistonExtensionTile::clearOverrideTopTexture()
{
	this->overrideTopTexture = NULL;
}

void PistonExtensionTile::playerWillDestroy(Level *level, int x, int y, int z, int data, shared_ptr<Player> player)
{
	if (player->abilities.instabuild)
	{
		int facing = getFacing(data);
		int tile = level->getTile(x - Facing::STEP_X[facing], y - Facing::STEP_Y[facing], z - Facing::STEP_Z[facing]);
		if (tile == Tile::pistonBase_Id || tile == Tile::pistonStickyBase_Id)
		{
			level->removeTile(x - Facing::STEP_X[facing], y - Facing::STEP_Y[facing], z - Facing::STEP_Z[facing]);
		}
	}
	Tile::playerWillDestroy(level, x, y, z, data, player);
}

void PistonExtensionTile::onRemove(Level *level, int x, int y, int z, int id, int data)
{
	Tile::onRemove(level, x, y, z, id, data);
	int facing = Facing::OPPOSITE_FACING[getFacing(data)];
	x += Facing::STEP_X[facing];
	y += Facing::STEP_Y[facing];
	z += Facing::STEP_Z[facing];

	int t = level->getTile(x, y, z);

	if (t == Tile::pistonBase_Id || t == Tile::pistonStickyBase_Id)
	{
		data = level->getData(x, y, z);
		if (PistonBaseTile::isExtended(data))
		{
			Tile::tiles[t]->spawnResources(level, x, y, z, data, 0);
			level->removeTile(x, y, z);

		}
	}
}

Icon *PistonExtensionTile::getTexture(int face, int data)
{
	int facing = getFacing(data);

	if (face == facing)
	{
		if (overrideTopTexture != NULL)
		{
			return overrideTopTexture;
		}
		if ((data & STICKY_BIT) != 0)
		{
			return PistonBaseTile::getTexture(PistonBaseTile::PLATFORM_STICKY_TEX);
		}
		return PistonBaseTile::getTexture(PistonBaseTile::PLATFORM_TEX);
	}
	if (facing < 6 && face == Facing::OPPOSITE_FACING[facing])
	{
		return PistonBaseTile::getTexture(PistonBaseTile::PLATFORM_TEX);
	}
	return PistonBaseTile::getTexture(PistonBaseTile::EDGE_TEX); // edge and arms
}

void PistonExtensionTile::registerIcons(IconRegister *iconRegister)
{
	// None
}

int PistonExtensionTile::getRenderShape()
{
	return SHAPE_PISTON_EXTENSION;
}

bool PistonExtensionTile::isSolidRender(bool isServerLevel)
{
	return false;
}

bool PistonExtensionTile::isCubeShaped()
{
	return false;
}

bool PistonExtensionTile::mayPlace(Level *level, int x, int y, int z)
{
	return false;
}

bool PistonExtensionTile::mayPlace(Level *level, int x, int y, int z, int face)
{
	return false;
}

int PistonExtensionTile::getResourceCount(Random *random)
{
	return 0;
}

void PistonExtensionTile::addAABBs(Level *level, int x, int y, int z, AABB *box, AABBList *boxes, shared_ptr<Entity> source)
{
	int data = level->getData(x, y, z);

	const float thickness = PistonBaseTile::PLATFORM_THICKNESS / 16.0f;
	const float smallEdge1 = (8.0f - (PistonBaseTile::PLATFORM_THICKNESS / 2.0f)) / 16.0f;
	const float smallEdge2 = (8.0f + (PistonBaseTile::PLATFORM_THICKNESS / 2.0f)) / 16.0f;
	const float largeEdge1 = (8.0f - PistonBaseTile::PLATFORM_THICKNESS) / 16.0f;
	const float largeEdge2 = (8.0f + PistonBaseTile::PLATFORM_THICKNESS) / 16.0f;

	switch (getFacing(data))
	{
	case Facing::DOWN:
		setShape(0, 0, 0, 1, thickness, 1);
		Tile::addAABBs(level, x, y, z, box, boxes, source);
		setShape(smallEdge1, thickness, smallEdge1, smallEdge2, 1, smallEdge2);
		Tile::addAABBs(level, x, y, z, box, boxes, source);
		break;
	case Facing::UP:
		setShape(0, 1 - thickness, 0, 1, 1, 1);
		Tile::addAABBs(level, x, y, z, box, boxes, source);
		setShape(smallEdge1, 0, smallEdge1, smallEdge2, 1 - thickness, smallEdge2);
		Tile::addAABBs(level, x, y, z, box, boxes, source);
		break;
	case Facing::NORTH:
		setShape(0, 0, 0, 1, 1, thickness);
		Tile::addAABBs(level, x, y, z, box, boxes, source);
		setShape(largeEdge1, smallEdge1, thickness, largeEdge2, smallEdge2, 1);
		Tile::addAABBs(level, x, y, z, box, boxes, source);
		break;
	case Facing::SOUTH:
		setShape(0, 0, 1 - thickness, 1, 1, 1);
		Tile::addAABBs(level, x, y, z, box, boxes, source);
		setShape(largeEdge1, smallEdge1, 0, largeEdge2, smallEdge2, 1 - thickness);
		Tile::addAABBs(level, x, y, z, box, boxes, source);
		break;
	case Facing::WEST:
		setShape(0, 0, 0, thickness, 1, 1);
		Tile::addAABBs(level, x, y, z, box, boxes, source);
		setShape(smallEdge1, largeEdge1, thickness, smallEdge2, largeEdge2, 1);
		Tile::addAABBs(level, x, y, z, box, boxes, source);
		break;
	case Facing::EAST:
		setShape(1 - thickness, 0, 0, 1, 1, 1);
		Tile::addAABBs(level, x, y, z, box, boxes, source);
		setShape(0, smallEdge1, largeEdge1, 1 - thickness, smallEdge2, largeEdge2);
		Tile::addAABBs(level, x, y, z, box, boxes, source);
		break;
	}
	setShape(0, 0, 0, 1, 1, 1);

}

void PistonExtensionTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity) // 4J added forceData, forceEntity param
{
	int data = (forceData == -1 ) ? level->getData(x, y, z) : forceData;

	const float thickness = PistonBaseTile::PLATFORM_THICKNESS / 16.0f;

	switch (getFacing(data))
	{
	case Facing::DOWN:
		setShape(0, 0, 0, 1, thickness, 1);
		break;
	case Facing::UP:
		setShape(0, 1 - thickness, 0, 1, 1, 1);
		break;
	case Facing::NORTH:
		setShape(0, 0, 0, 1, 1, thickness);
		break;
	case Facing::SOUTH:
		setShape(0, 0, 1 - thickness, 1, 1, 1);
		break;
	case Facing::WEST:
		setShape(0, 0, 0, thickness, 1, 1);
		break;
	case Facing::EAST:
		setShape(1 - thickness, 0, 0, 1, 1, 1);
		break;
	}
}

void PistonExtensionTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	int facing = getFacing(level->getData(x, y, z));
	int tile = level->getTile(x - Facing::STEP_X[facing], y - Facing::STEP_Y[facing], z - Facing::STEP_Z[facing]);
	if (tile != Tile::pistonBase_Id && tile != Tile::pistonStickyBase_Id)
	{
		level->removeTile(x, y, z);
	}
	else
	{
		Tile::tiles[tile]->neighborChanged(level, x - Facing::STEP_X[facing], y - Facing::STEP_Y[facing], z - Facing::STEP_Z[facing], type);
	}
}

int PistonExtensionTile::getFacing(int data)
{
	return data & 0x7;
}

int PistonExtensionTile::cloneTileId(Level *level, int x, int y, int z)
{
	int data = level->getData(x, y, z);
	if ((data & STICKY_BIT) != 0)
	{
		return Tile::pistonStickyBase_Id;
	}
	return Tile::pistonBase_Id;
	return 0;
}