#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "TripWireTile.h"

TripWireTile::TripWireTile(int id) : Tile(id, Material::decoration, isSolidRender())
{
	setShape(0, 0, 0, 1, 2.5f / 16.0f, 1);
	this->setTicking(true);
}

int TripWireTile::getTickDelay(Level *level)
{
	// 4J:	Increased (x2); quick update caused problems with shared
	//		data between client and server.
	return 20; // 10;
}

AABB *TripWireTile::getAABB(Level *level, int x, int y, int z)
{
	return NULL;
}

bool TripWireTile::blocksLight()
{
	return false;
}

bool TripWireTile::isSolidRender(bool isServerLevel)
{
	return false;
}

bool TripWireTile::isCubeShaped()
{
	return false;
}

int TripWireTile::getRenderLayer()
{
	return 1;
}

int TripWireTile::getRenderShape()
{
	return Tile::SHAPE_TRIPWIRE;
}

int TripWireTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return Item::string_Id;
}

int TripWireTile::cloneTileId(Level *level, int x, int y, int z)
{
	return Item::string_Id;
}

void TripWireTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	int data = level->getData(x, y, z);
	bool wasSuspended = (data & MASK_SUSPENDED) == MASK_SUSPENDED;
	bool isSuspended = !level->isTopSolidBlocking(x, y - 1, z);
	if (wasSuspended != isSuspended)
	{
		spawnResources(level, x, y, z, data, 0);
		level->removeTile(x, y, z);
	}
}

void TripWireTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity)
{
	int data = level->getData(x, y, z);
	bool attached = (data & MASK_ATTACHED) == MASK_ATTACHED;
	bool suspended = (data & MASK_SUSPENDED) == MASK_SUSPENDED;

	if (!suspended)
	{
		setShape(0, 0, 0, 1, 1.5f / 16.0f, 1);
	}
	else if (!attached)
	{
		setShape(0, 0, 0, 1, 8.0f / 16.0f, 1);
	}
	else
	{
		setShape(0, 1.0f / 16.0f, 0, 1, 2.5f / 16.0f, 1);
	}
}

void TripWireTile::onPlace(Level *level, int x, int y, int z)
{
	int data = level->isTopSolidBlocking(x, y - 1, z) ? 0 : MASK_SUSPENDED;
	level->setData(x, y, z, data, Tile::UPDATE_ALL);
	updateSource(level, x, y, z, data);
}

void TripWireTile::onRemove(Level *level, int x, int y, int z, int id, int data)
{
	updateSource(level, x, y, z, data | MASK_POWERED);
}

void TripWireTile::playerWillDestroy(Level *level, int x, int y, int z, int data, shared_ptr<Player> player)
{
	if (level->isClientSide) return;

	if (player->getSelectedItem() != NULL && player->getSelectedItem()->id == Item::shears_Id)
	{
		level->setData(x, y, z, data | MASK_DISARMED, Tile::UPDATE_NONE);
	}
}

void TripWireTile::updateSource(Level *level, int x, int y, int z, int data)
{
	for (int dir = 0; dir < 2; dir++)
	{
		for (int i = 1; i < TripWireSourceTile::WIRE_DIST_MAX; i++)
		{
			int xx = x + Direction::STEP_X[dir] * i;
			int zz = z + Direction::STEP_Z[dir] * i;
			int tile = level->getTile(xx, y, zz);

			if (tile == Tile::tripWireSource_Id)
			{
				int sourceDir = level->getData(xx, y, zz) & TripWireSourceTile::MASK_DIR;

				if (sourceDir == Direction::DIRECTION_OPPOSITE[dir])
				{
					Tile::tripWireSource->calculateState(level, xx, y, zz, tile, level->getData(xx, y, zz), true, i, data);
				}

				break;
			}
			else if (tile != Tile::tripWire_Id)
			{
				break;
			}
		}
	}
}

void TripWireTile::entityInside(Level *level, int x, int y, int z, shared_ptr<Entity> entity)
{
	if (level->isClientSide) return;

	if ((level->getData(x, y, z) & MASK_POWERED) == MASK_POWERED) return;

	checkPressed(level, x, y, z);
}

void TripWireTile::tick(Level *level, int x, int y, int z, Random *random)
{
	if (level->isClientSide) return;

	if ((level->getData(x, y, z) & MASK_POWERED) != MASK_POWERED) return;

	checkPressed(level, x, y, z);
}

void TripWireTile::checkPressed(Level *level, int x, int y, int z)
{
	int data = level->getData(x, y, z);
	bool wasPressed = (data & MASK_POWERED) == MASK_POWERED;
	bool shouldBePressed = false;

	ThreadStorage *tls = (ThreadStorage *)TlsGetValue(Tile::tlsIdxShape);
	vector<shared_ptr<Entity> > *entities = level->getEntities(nullptr, AABB::newTemp(x + tls->xx0, y + tls->yy0, z + tls->zz0, x + tls->xx1, y + tls->yy1, z + tls->zz1));
	if (!entities->empty())
	{
		for (auto& e : *entities)
		{
			if ( e && !e->isIgnoringTileTriggers())
			{
				shouldBePressed = true;
				break;
			}
		}
	}

	if (shouldBePressed && !wasPressed)
	{
		data |= MASK_POWERED;
	}

	if (!shouldBePressed && wasPressed)
	{
		data &= ~MASK_POWERED;
	}

	if (shouldBePressed != wasPressed)
	{
		level->setData(x, y, z, data, Tile::UPDATE_ALL);
		updateSource(level, x, y, z, data);
	}

	if (shouldBePressed)
	{
		level->addToTickNextTick(x, y, z, id, getTickDelay(level));
	}
}

bool TripWireTile::shouldConnectTo(LevelSource *level, int x, int y, int z, int data, int dir)
{
	int tx = x + Direction::STEP_X[dir];
	int ty = y;
	int tz = z + Direction::STEP_Z[dir];
	int t = level->getTile(tx, ty, tz);
	bool suspended = (data & MASK_SUSPENDED) == MASK_SUSPENDED;

	if (t == Tile::tripWireSource_Id)
	{
		int otherData = level->getData(tx, ty, tz);
		int facing = otherData & TripWireSourceTile::MASK_DIR;

		return facing == Direction::DIRECTION_OPPOSITE[dir];
	}

	if (t == Tile::tripWire_Id)
	{
		int otherData = level->getData(tx, ty, tz);
		bool otherSuspended = (otherData & MASK_SUSPENDED) == MASK_SUSPENDED;
		return suspended == otherSuspended;
	}

	return false;
}
