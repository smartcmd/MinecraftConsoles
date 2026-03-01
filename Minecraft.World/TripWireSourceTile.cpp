#include "stdafx.h"
#include "net.minecraft.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.redstone.h"
#include "TripWireSourceTile.h"

TripWireSourceTile::TripWireSourceTile(int id) : Tile(id, Material::decoration, isSolidRender())
{
	this->setTicking(true);
}

AABB *TripWireSourceTile::getAABB(Level *level, int x, int y, int z)
{
	return NULL;
}

bool TripWireSourceTile::blocksLight()
{
	return false;
}

bool TripWireSourceTile::isSolidRender(bool isServerLevel)
{
	return false;
}

bool TripWireSourceTile::isCubeShaped()
{
	return false;
}

int TripWireSourceTile::getRenderShape()
{
	return Tile::SHAPE_TRIPWIRE_SOURCE;
}

int TripWireSourceTile::getTickDelay(Level *level)
{
	return 10;
}

bool TripWireSourceTile::mayPlace(Level *level, int x, int y, int z, int face)
{
	if (face == Facing::NORTH && level->isSolidBlockingTile(x, y, z + 1)) return true;
	if (face == Facing::SOUTH && level->isSolidBlockingTile(x, y, z - 1)) return true;
	if (face == Facing::WEST && level->isSolidBlockingTile(x + 1, y, z)) return true;
	if (face == Facing::EAST && level->isSolidBlockingTile(x - 1, y, z)) return true;
	return false;
}

bool TripWireSourceTile::mayPlace(Level *level, int x, int y, int z)
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

int TripWireSourceTile::getPlacedOnFaceDataValue(Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue)
{
	int dir = 0;

	if (face == Facing::NORTH && level->isSolidBlockingTileInLoadedChunk(x, y, z + 1, true)) dir = Direction::NORTH;
	if (face == Facing::SOUTH && level->isSolidBlockingTileInLoadedChunk(x, y, z - 1, true)) dir = Direction::SOUTH;
	if (face == Facing::WEST && level->isSolidBlockingTileInLoadedChunk(x + 1, y, z, true)) dir = Direction::WEST;
	if (face == Facing::EAST && level->isSolidBlockingTileInLoadedChunk(x - 1, y, z, true)) dir = Direction::EAST;

	return dir;
}

void TripWireSourceTile::finalizePlacement(Level *level, int x, int y, int z, int data)
{
	calculateState(level, x, y, z, id, data, false, -1, 0);
}

void TripWireSourceTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	if (type == this->id) return;
	if (checkCanSurvive(level, x, y, z))
	{
		int data = level->getData(x, y, z);
		int dir = data & MASK_DIR;
		bool replace = false;

		if (!level->isSolidBlockingTile(x - 1, y, z) && dir == Direction::EAST) replace = true;
		if (!level->isSolidBlockingTile(x + 1, y, z) && dir == Direction::WEST) replace = true;
		if (!level->isSolidBlockingTile(x, y, z - 1) && dir == Direction::SOUTH) replace = true;
		if (!level->isSolidBlockingTile(x, y, z + 1) && dir == Direction::NORTH) replace = true;

		if (replace)
		{
			spawnResources(level, x, y, z, data, 0);
			level->removeTile(x, y, z);
		}
	}
}

void TripWireSourceTile::calculateState(Level *level, int x, int y, int z, int id, int data, bool canUpdate, 
										/*4J-Jev, these parameters only used with 'updateSource' -->*/ int wireSource, int wireSourceData)
{
	int dir = data & MASK_DIR;
	bool wasAttached = (data & MASK_ATTACHED) == MASK_ATTACHED;
	bool wasPowered = (data & MASK_POWERED) == MASK_POWERED;
	bool attached = id == Tile::tripWireSource_Id; // id is only != TripwireSource_id when 'onRemove' 
	bool powered = false;
	bool suspended = !level->isTopSolidBlocking(x, y - 1, z);
	int stepX = Direction::STEP_X[dir];
	int stepZ = Direction::STEP_Z[dir];
	int receiverPos = 0;
	int wiresData[WIRE_DIST_MAX];

	// Loop over each tile down the wire, from this tile, to the expected opposing src tile.
	for (int i = 1; i < WIRE_DIST_MAX; i++)
	{
		int xx = x + stepX * i;
		int zz = z + stepZ * i;
		int tile = level->getTile(xx, y, zz);

		if (tile == Tile::tripWireSource_Id)
		{
			int otherData = level->getData(xx, y, zz);

			if ((otherData & MASK_DIR) == Direction::DIRECTION_OPPOSITE[dir])
			{
				receiverPos = i;
			}

			break;
		}
		else if (tile == Tile::tripWire_Id || i == wireSource) // wireSource is the wiretile that caused an 'updateSource'
		{
			int wireData = i == wireSource ? wireSourceData : level->getData(xx, y, zz);
			bool wireArmed = (wireData & TripWireTile::MASK_DISARMED) != TripWireTile::MASK_DISARMED;
			bool wirePowered = (wireData & TripWireTile::MASK_POWERED) == TripWireTile::MASK_POWERED;
			bool wireSuspended = (wireData & TripWireTile::MASK_SUSPENDED) == TripWireTile::MASK_SUSPENDED;
			attached &= wireSuspended == suspended;
			powered |= wireArmed && wirePowered;

			wiresData[i] = wireData;

			if (i == wireSource)
			{
				level->addToTickNextTick(x, y, z, id, getTickDelay(level));
				attached &= wireArmed;
			}
		}
		else // Non-wire or src tile encountered.
		{
			wiresData[i] = -1;
			attached = false;
		}
	}

	attached &= receiverPos > WIRE_DIST_MIN;
	powered &= attached;
	int state = (attached ? MASK_ATTACHED : 0) | (powered ? MASK_POWERED : 0);
	data = dir | state;

	if (receiverPos > 0) // If a receiver is detected update it's state and notify it's neighbours.
	{
		int xx = x + stepX * receiverPos;
		int zz = z + stepZ * receiverPos;
		int opposite = Direction::DIRECTION_OPPOSITE[dir];
		level->setData(xx, y, zz, opposite | state, Tile::UPDATE_ALL);
		notifyNeighbors(level, xx, y, zz, opposite);

		playSound(level, xx, y, zz, attached, powered, wasAttached, wasPowered);
	}

	playSound(level, x, y, z, attached, powered, wasAttached, wasPowered);

	if (id > 0) // ie. it isn't being removed.
	{
		level->setData(x, y, z, data, Tile::UPDATE_ALL);
		if (canUpdate) notifyNeighbors(level, x, y, z, dir);
	}

	if (wasAttached != attached)
	{
		for (int i = 1; i < receiverPos; i++)
		{
			int xx = x + stepX * i;
			int zz = z + stepZ * i;
			int wireData = wiresData[i];
			if (wireData < 0) continue;

			if (attached)
			{
				wireData |= TripWireTile::MASK_ATTACHED;
			}
			else
			{
				wireData &= ~TripWireTile::MASK_ATTACHED;
			}

			
			level->setData(xx, y, zz, wireData, Tile::UPDATE_ALL);
		}
	}
}

void TripWireSourceTile::tick(Level *level, int x, int y, int z, Random *random)
{
	calculateState(level, x, y, z, id, level->getData(x, y, z), true, -1, 0);
}

void TripWireSourceTile::playSound(Level *level, int x, int y, int z, bool attached, bool powered, bool wasAttached, bool wasPowered)
{
	if (powered && !wasPowered)
	{
		level->playSound(x + 0.5, y + 0.1, z + 0.5, eSoundType_RANDOM_CLICK, 0.4f, 0.6f);
	}
	else if (!powered && wasPowered)
	{
		level->playSound(x + 0.5, y + 0.1, z + 0.5, eSoundType_RANDOM_CLICK, 0.4f, 0.5f);
	}
	else if (attached && !wasAttached)
	{
		level->playSound(x + 0.5, y + 0.1, z + 0.5, eSoundType_RANDOM_CLICK, 0.4f, 0.7f);
	}
	else if (!attached && wasAttached)
	{
		level->playSound(x + 0.5, y + 0.1, z + 0.5, eSoundType_RANDOM_BOW_HIT, 0.4f, 1.2f / (level->random->nextFloat() * 0.2f + 0.9f));
	}
}

void TripWireSourceTile::notifyNeighbors(Level *level, int x, int y, int z, int dir)
{
	level->updateNeighborsAt(x, y, z, id);

	if (dir == Direction::EAST)
	{
		level->updateNeighborsAt(x - 1, y, z, id);
	}
	else if (dir == Direction::WEST)
	{
		level->updateNeighborsAt(x + 1, y, z, id);
	}
	else if (dir == Direction::SOUTH)
	{
		level->updateNeighborsAt(x, y, z - 1, id);
	}
	else if (dir == Direction::NORTH)
	{
		level->updateNeighborsAt(x, y, z + 1, id);
	}
}

bool TripWireSourceTile::checkCanSurvive(Level *level, int x, int y, int z)
{
	if (!mayPlace(level, x, y, z))
	{
		this->spawnResources(level, x, y, z, level->getData(x, y, z), 0);
		level->removeTile(x, y, z);
		return false;
	}

	return true;
}

void TripWireSourceTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity)
{
	int dir = level->getData(x, y, z) & MASK_DIR;
	float r = 3 / 16.0f;

	if (dir == Direction::EAST)
	{
		setShape(0, 0.2f, 0.5f - r, r * 2, 0.8f, 0.5f + r);
	}
	else if (dir == Direction::WEST)
	{
		setShape(1 - r * 2, 0.2f, 0.5f - r, 1, 0.8f, 0.5f + r);
	}
	else if (dir == Direction::SOUTH)
	{
		setShape(0.5f - r, 0.2f, 0, 0.5f + r, 0.8f, r * 2);
	}
	else if (dir == Direction::NORTH)
	{
		setShape(0.5f - r, 0.2f, 1 - r * 2, 0.5f + r, 0.8f, 1);
	}
}

void TripWireSourceTile::onRemove(Level *level, int x, int y, int z, int id, int data)
{
	bool attached = (data & MASK_ATTACHED) == MASK_ATTACHED;
	bool powered = (data & MASK_POWERED) == MASK_POWERED;

	if (attached || powered)
	{
		calculateState(level, x, y, z, 0, data, false, -1, 0); // Disconnect
		// the other end.
	}

	if (powered)
	{
		level->updateNeighborsAt(x, y, z, this->id);
		int dir = data & MASK_DIR;

		if (dir == Direction::EAST)
		{
			level->updateNeighborsAt(x - 1, y, z, this->id);
		}
		else if (dir == Direction::WEST)
		{
			level->updateNeighborsAt(x + 1, y, z, this->id);
		}
		else if (dir == Direction::SOUTH)
		{
			level->updateNeighborsAt(x, y, z - 1, this->id);
		}
		else if (dir == Direction::NORTH)
		{
			level->updateNeighborsAt(x, y, z + 1, this->id);
		}
	}

	Tile::onRemove(level, x, y, z, id, data);
}

int TripWireSourceTile::getSignal(LevelSource *level, int x, int y, int z, int dir)
{
	return (level->getData(x, y, z) & MASK_POWERED) == MASK_POWERED ? Redstone::SIGNAL_MAX : Redstone::SIGNAL_NONE;
}

int TripWireSourceTile::getDirectSignal(LevelSource *level, int x, int y, int z, int dir)
{
	int data = level->getData(x, y, z);
	if ((data & MASK_POWERED) != MASK_POWERED) return Redstone::SIGNAL_NONE;
	int myDir = data & MASK_DIR;

	if (myDir == Direction::NORTH && dir == Facing::NORTH) return Redstone::SIGNAL_MAX;
	if (myDir == Direction::SOUTH && dir == Facing::SOUTH) return Redstone::SIGNAL_MAX;
	if (myDir == Direction::WEST && dir == Facing::WEST) return Redstone::SIGNAL_MAX;
	if (myDir == Direction::EAST && dir == Facing::EAST) return Redstone::SIGNAL_MAX;


	return Redstone::SIGNAL_NONE;
}

bool TripWireSourceTile::isSignalSource()
{
	return true;
}
