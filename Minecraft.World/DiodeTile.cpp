#include "stdafx.h"
#include "net.minecraft.h"
#include "net.minecraft.world.level.redstone.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.h"
#include "DiodeTile.h"

DiodeTile::DiodeTile(int id, bool on) : DirectionalTile(id, Material::decoration,isSolidRender())
{
	this->on = on;
	updateDefaultShape();
}

// 4J Added override
void DiodeTile::updateDefaultShape()
{
	setShape(0, 0, 0, 1, 2.0f / 16.0f, 1);
}

bool DiodeTile::isCubeShaped()
{
	return false;
}

bool DiodeTile::mayPlace(Level *level, int x, int y, int z)
{
	if (!level->isTopSolidBlocking(x, y - 1, z))
	{
		return false;
	}
	return Tile::mayPlace(level, x, y, z);
}

bool DiodeTile::canSurvive(Level *level, int x, int y, int z)
{
	if (!level->isTopSolidBlocking(x, y - 1, z))
	{
		return false;
	}
	return Tile::canSurvive(level, x, y, z);
}

void DiodeTile::tick(Level *level, int x, int y, int z, Random *random)
{
	int data = level->getData(x, y, z);
	if (!isLocked(level, x, y, z, data))
	{
		bool sourceOn = shouldTurnOn(level, x, y, z, data);
		if (on && !sourceOn)
		{
			level->setTileAndData(x, y, z, getOffTile()->id, data, Tile::UPDATE_CLIENTS);
		}
		else if (!on)
		{
			// when off-diodes are ticked, they always turn on for one tick and
			// then off again if necessary
			level->setTileAndData(x, y, z, getOnTile()->id, data, Tile::UPDATE_CLIENTS);
			if (!sourceOn)
			{
				level->addToTickNextTick(x, y, z, getOnTile()->id, getTurnOffDelay(data), -1);
			}
		}
	}
}

Icon *DiodeTile::getTexture(int face, int data)
{
	// down is used by the torch tesselator
	if (face == Facing::DOWN)
	{
		if (on)
		{
			return Tile::redstoneTorch_on->getTexture(face);
		}
		return Tile::redstoneTorch_off->getTexture(face);
	}
	if (face == Facing::UP)
	{
		return icon;
	}
	// edge of stone half-step
	return Tile::stoneSlab->getTexture(Facing::UP);
}

bool DiodeTile::shouldRenderFace(LevelSource *level, int x, int y, int z, int face)
{
	if (face == Facing::DOWN || face == Facing::UP)
	{
		// up and down is a special case handled by the shape renderer
		return false;
	}
	return true;
}

int DiodeTile::getRenderShape()
{
	return SHAPE_DIODE;
}

bool DiodeTile::isOn(int data)
{
	return on;
}

int DiodeTile::getDirectSignal(LevelSource *level, int x, int y, int z, int dir)
{
	return getSignal(level, x, y, z, dir);
}

int DiodeTile::getSignal(LevelSource *level, int x, int y, int z, int facing)
{
	int data = level->getData(x, y, z);
	if (!isOn(data))
	{
		return Redstone::SIGNAL_NONE;
	}

	int dir = getDirection(data);

	if (dir == Direction::SOUTH && facing == Facing::SOUTH) return getOutputSignal(level, x, y, z, data);
	if (dir == Direction::WEST && facing == Facing::WEST) return getOutputSignal(level, x, y, z, data);
	if (dir == Direction::NORTH && facing == Facing::NORTH) return getOutputSignal(level, x, y, z, data);
	if (dir == Direction::EAST && facing == Facing::EAST) return getOutputSignal(level, x, y, z, data);

	return Redstone::SIGNAL_NONE;
}

void DiodeTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	if (!canSurvive(level, x, y, z))
	{
		this->spawnResources(level, x, y, z, level->getData(x, y, z), 0);
		level->removeTile(x, y, z);
		level->updateNeighborsAt(x + 1, y, z, id);
		level->updateNeighborsAt(x - 1, y, z, id);
		level->updateNeighborsAt(x, y, z + 1, id);
		level->updateNeighborsAt(x, y, z - 1, id);
		level->updateNeighborsAt(x, y - 1, z, id);
		level->updateNeighborsAt(x, y + 1, z, id);
		return;
	}

	checkTickOnNeighbor(level, x, y, z, type);
}

void DiodeTile::checkTickOnNeighbor(Level *level, int x, int y, int z, int type)
{
	int data = level->getData(x, y, z);

	if (!isLocked(level, x, y, z, data))
	{
		bool sourceOn = shouldTurnOn(level, x, y, z, data);
		if ((on && !sourceOn || !on && sourceOn) && !level->isTileToBeTickedAt(x, y, z, id))
		{
			int prio = -1;

			// if the tile in front is a repeater, we prioritize this update
			if (shouldPrioritize(level, x, y, z, data))
			{
				prio = -3;
			}
			else if (on)
			{
				prio = -2;
			}

			level->addToTickNextTick(x, y, z, id, getTurnOnDelay(data), prio);
		}
	}
}

bool DiodeTile::isLocked(LevelSource *level, int x, int y, int z, int data)
{
	return false;
}

bool DiodeTile::shouldTurnOn(Level *level, int x, int y, int z, int data)
{
	return getInputSignal(level, x, y, z, data) > Redstone::SIGNAL_NONE;
}

int DiodeTile::getInputSignal(Level *level, int x, int y, int z, int data)
{
	int dir = getDirection(data);

	int xx = x + Direction::STEP_X[dir];
	int zz = z + Direction::STEP_Z[dir];
	int input = level->getSignal(xx, y, zz, Direction::DIRECTION_FACING[dir]);

	if (input >= Redstone::SIGNAL_MAX) return input;
	return max(input, level->getTile(xx, y, zz) == Tile::redStoneDust_Id ? level->getData(xx, y, zz) : Redstone::SIGNAL_NONE);
}

int DiodeTile::getAlternateSignal(LevelSource *level, int x, int y, int z, int data)
{
	int dir = getDirection(data);

	switch (dir)
	{
	case Direction::SOUTH:
	case Direction::NORTH:
		return max(getAlternateSignalAt(level, x - 1, y, z, Facing::WEST), getAlternateSignalAt(level, x + 1, y, z, Facing::EAST));
	case Direction::EAST:
	case Direction::WEST:
		return max(getAlternateSignalAt(level, x, y, z + 1, Facing::SOUTH), getAlternateSignalAt(level, x, y, z - 1, Facing::NORTH));
	}

	return Redstone::SIGNAL_NONE;
}

int DiodeTile::getAlternateSignalAt(LevelSource *level, int x, int y, int z, int facing)
{
	int tile = level->getTile(x, y, z);

	if (isAlternateInput(tile))
	{
		if (tile == Tile::redStoneDust_Id)
		{
			return level->getData(x, y, z);
		}
		else
		{
			return level->getDirectSignal(x, y, z, facing);
		}
	}

	return Redstone::SIGNAL_NONE;
}

bool DiodeTile::isSignalSource()
{
	return true;
}

void DiodeTile::setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance)
{
	int dir = (((Mth::floor(by->yRot * 4 / (360) + 0.5)) & 3) + 2) % 4;
	level->setData(x, y, z, dir, Tile::UPDATE_ALL);

	bool sourceOn = shouldTurnOn(level, x, y, z, dir);
	if (sourceOn)
	{
		level->addToTickNextTick(x, y, z, id, 1);
	}
}

void DiodeTile::onPlace(Level *level, int x, int y, int z)
{
	updateNeighborsInFront(level, x, y, z);
}

void DiodeTile::updateNeighborsInFront(Level *level, int x, int y, int z)
{
	int dir = getDirection(level->getData(x, y, z));
	if (dir == Direction::WEST)
	{
		level->neighborChanged(x + 1, y, z, id);
		level->updateNeighborsAtExceptFromFacing(x + 1, y, z, id, Facing::WEST);
	}
	if (dir == Direction::EAST)
	{
		level->neighborChanged(x - 1, y, z, id);
		level->updateNeighborsAtExceptFromFacing(x - 1, y, z, id, Facing::EAST);
	}
	if (dir == Direction::NORTH)
	{
		level->neighborChanged(x, y, z + 1, id);
		level->updateNeighborsAtExceptFromFacing(x, y, z + 1, id, Facing::NORTH);
	}
	if (dir == Direction::SOUTH)
	{
		level->neighborChanged(x, y, z - 1, id);
		level->updateNeighborsAtExceptFromFacing(x, y, z - 1, id, Facing::SOUTH);
	}
}

void DiodeTile::destroy(Level *level, int x, int y, int z, int data)
{
	if (on)
	{
		level->updateNeighborsAt(x + 1, y, z, id);
		level->updateNeighborsAt(x - 1, y, z, id);
		level->updateNeighborsAt(x, y, z + 1, id);
		level->updateNeighborsAt(x, y, z - 1, id);
		level->updateNeighborsAt(x, y - 1, z, id);
		level->updateNeighborsAt(x, y + 1, z, id);
	}
	Tile::destroy(level, x, y, z, data);
}

bool DiodeTile::isSolidRender(bool isServerLevel)
{
	return false;
}

bool DiodeTile::isAlternateInput(int tile)
{
	Tile *tt = Tile::tiles[tile];
	return tt != NULL && tt->isSignalSource();
}

int DiodeTile::getOutputSignal(LevelSource *level, int x, int y, int z, int data)
{
	return Redstone::SIGNAL_MAX;
}

bool DiodeTile::isDiode(int id)
{
	return Tile::diode_off->isSameDiode(id) || Tile::comparator_off->isSameDiode(id);
}

bool DiodeTile::isSameDiode(int id)
{
	return id == getOnTile()->id || id == getOffTile()->id;
}

bool DiodeTile::shouldPrioritize(Level *level, int x, int y, int z, int data)
{
	int dir = getDirection(data);
	if (isDiode(level->getTile(x - Direction::STEP_X[dir], y, z - Direction::STEP_Z[dir])))
	{
		int odata = level->getData(x - Direction::STEP_X[dir], y, z - Direction::STEP_Z[dir]);
		int odir = getDirection(odata);
		return odir != dir;
	}
	return false;
}

int DiodeTile::getTurnOffDelay(int data)
{
	return getTurnOnDelay(data);
}

bool DiodeTile::isMatching(int id)
{
	return isSameDiode(id);
}