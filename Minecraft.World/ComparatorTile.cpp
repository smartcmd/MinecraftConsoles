#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.redstone.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "net.minecraft.h"
#include "ComparatorTile.h"

ComparatorTile::ComparatorTile(int id, bool on) : DiodeTile(id, on)
{
	_isEntityTile = true;
}

int ComparatorTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return Item::comparator_Id;
}

int ComparatorTile::cloneTileId(Level *level, int x, int y, int z)
{
	return Item::comparator_Id;
}

int ComparatorTile::getTurnOnDelay(int data)
{
	return 2;
}

DiodeTile *ComparatorTile::getOnTile()
{
	return Tile::comparator_on;
}

DiodeTile *ComparatorTile::getOffTile()
{
	return Tile::comparator_off;
}

int ComparatorTile::getRenderShape()
{
	return SHAPE_COMPARATOR;
}

Icon *ComparatorTile::getTexture(int face, int data)
{
	bool isOn = on || (data & BIT_IS_LIT) != 0;
	// down is used by the torch tesselator
	if (face == Facing::DOWN)
	{
		if (isOn)
		{
			return Tile::redstoneTorch_on->getTexture(face);
		}
		return Tile::redstoneTorch_off->getTexture(face);
	}
	if (face == Facing::UP)
	{
		if (isOn)
		{
			return Tile::comparator_on->icon;
		}
		return icon;
	}
	// edge of stone half-step
	return Tile::stoneSlab->getTexture(Facing::UP);
}

bool ComparatorTile::isOn(int data)
{
	return on || (data & BIT_IS_LIT) != 0;
}

int ComparatorTile::getOutputSignal(LevelSource *levelSource, int x, int y, int z, int data)
{
	return getComparator(levelSource, x, y, z)->getOutputSignal();
}

int ComparatorTile::calculateOutputSignal(Level *level, int x, int y, int z, int data)
{
	if (!isReversedOutputSignal(data))
	{
		return getInputSignal(level, x, y, z, data);
	}
	else
	{
		return max(getInputSignal(level, x, y, z, data) - getAlternateSignal(level, x, y, z, data), Redstone::SIGNAL_NONE);
	}
}

bool ComparatorTile::isReversedOutputSignal(int data)
{
	return (data & BIT_OUTPUT_SUBTRACT) == BIT_OUTPUT_SUBTRACT;
}

bool ComparatorTile::shouldTurnOn(Level *level, int x, int y, int z, int data)
{
	int input = getInputSignal(level, x, y, z, data);
	if (input >= Redstone::SIGNAL_MAX) return true;
	if (input == Redstone::SIGNAL_NONE) return false;

	int alt = getAlternateSignal(level, x, y, z, data);
	if (alt == Redstone::SIGNAL_NONE) return true;

	return input >= alt;
}

int ComparatorTile::getInputSignal(Level *level, int x, int y, int z, int data)
{
	int result = DiodeTile::getInputSignal(level, x, y, z, data);

	int dir = getDirection(data);
	int xx = x + Direction::STEP_X[dir];
	int zz = z + Direction::STEP_Z[dir];
	int tile = level->getTile(xx, y, zz);

	if (tile > 0)
	{
		if (Tile::tiles[tile]->hasAnalogOutputSignal())
		{
			result = Tile::tiles[tile]->getAnalogOutputSignal(level, xx, y, zz, Direction::DIRECTION_OPPOSITE[dir]);
		}
		else if (result < Redstone::SIGNAL_MAX && Tile::isSolidBlockingTile(tile))
		{
			xx += Direction::STEP_X[dir];
			zz += Direction::STEP_Z[dir];
			tile = level->getTile(xx, y, zz);

			if (tile > 0 && Tile::tiles[tile]->hasAnalogOutputSignal())
			{
				result = Tile::tiles[tile]->getAnalogOutputSignal(level, xx, y, zz, Direction::DIRECTION_OPPOSITE[dir]);
			}
		}
	}

	return result;
}

shared_ptr<ComparatorTileEntity> ComparatorTile::getComparator(LevelSource *level, int x, int y, int z)
{
	return dynamic_pointer_cast<ComparatorTileEntity>( level->getTileEntity(x, y, z) );
}

bool ComparatorTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly)
{
	int data = level->getData(x, y, z);
	bool isOn = on || ( (data & BIT_IS_LIT) != 0 );
	bool subtract = !isReversedOutputSignal(data);
	int outputBit = subtract ? BIT_OUTPUT_SUBTRACT : 0;
	outputBit |= isOn ? BIT_IS_LIT : 0;

	level->playSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_RANDOM_CLICK, 0.3f, subtract ? 0.55f : 0.5f);

	if (!soundOnly)
	{
		level->setData(x, y, z, outputBit | (data & DIRECTION_MASK), Tile::UPDATE_CLIENTS);
		refreshOutputState(level, x, y, z, level->random);
	}

	return true;
}

void ComparatorTile::checkTickOnNeighbor(Level *level, int x, int y, int z, int type)
{
	if (!level->isTileToBeTickedAt(x, y, z, id))
	{
		int data = level->getData(x, y, z);
		int outputValue = calculateOutputSignal(level, x, y, z, data);
		int oldValue = getComparator(level, x, y, z)->getOutputSignal();

		if (outputValue != oldValue || (isOn(data) != shouldTurnOn(level, x, y, z, data)))
		{
			// prioritize locking comparators
			if (shouldPrioritize(level, x, y, z, data))
			{
				level->addToTickNextTick(x, y, z, id, getTurnOnDelay(0), -1);
			}
			else
			{
				level->addToTickNextTick(x, y, z, id, getTurnOnDelay(0), 0);
			}
		}
	}
}

void ComparatorTile::refreshOutputState(Level *level, int x, int y, int z, Random *random)
{
	int data = level->getData(x, y, z);
	int outputValue = calculateOutputSignal(level, x, y, z, data);
	int oldValue = getComparator(level, x, y, z)->getOutputSignal();
	getComparator(level, x, y, z)->setOutputSignal(outputValue);

	if (oldValue != outputValue || !isReversedOutputSignal(data))
	{
		bool sourceOn = shouldTurnOn(level, x, y, z, data);
		bool isOn = on || (data & BIT_IS_LIT) != 0;
		if (isOn && !sourceOn)
		{
			level->setData(x, y, z, data & ~BIT_IS_LIT, Tile::UPDATE_CLIENTS);
		}
		else if (!isOn && sourceOn)
		{
			level->setData(x, y, z, data | BIT_IS_LIT, Tile::UPDATE_CLIENTS);
		}
		updateNeighborsInFront(level, x, y, z);
	}
}

void ComparatorTile::tick(Level *level, int x, int y, int z, Random *random)
{
	if (on)
	{
		// clean-up old tiles with the 'on' id
		int data = level->getData(x, y, z);
		level->setTileAndData(x, y, z, getOffTile()->id, data | BIT_IS_LIT, Tile::UPDATE_NONE);
	}
	refreshOutputState(level, x, y, z, random);
}

void ComparatorTile::onPlace(Level *level, int x, int y, int z)
{
	DiodeTile::onPlace(level, x, y, z);
	level->setTileEntity(x, y, z, newTileEntity(level));
}

void ComparatorTile::onRemove(Level *level, int x, int y, int z, int id, int data)
{
	DiodeTile::onRemove(level, x, y, z, id, data);
	level->removeTileEntity(x, y, z);

	updateNeighborsInFront(level, x, y, z);
}

bool ComparatorTile::triggerEvent(Level *level, int x, int y, int z, int b0, int b1)
{
	DiodeTile::triggerEvent(level, x, y, z, b0, b1);
	shared_ptr<TileEntity> te = level->getTileEntity(x, y, z);
	if (te != nullptr)
	{
		return te->triggerEvent(b0, b1);
	}
	return false;
}

shared_ptr<TileEntity> ComparatorTile::newTileEntity(Level *level)
{
	return std::make_shared<ComparatorTileEntity>();
}

bool ComparatorTile::TestUse()
{
	return true;
}