#include "stdafx.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.h"
#include "BaseRailTile.h"

BaseRailTile::Rail::Rail(Level *level, int x, int y, int z)
{
	this->level = level;
	this->x = x;
	this->y = y;
	this->z = z;

	int id = level->getTile(x, y, z);

	// 4J Stu - We saw a random crash near the end of development on XboxOne orignal version where the id here isn't a tile any more
	// Adding this check in to avoid that crash
	m_bValidRail = isRail(id);
	if(m_bValidRail)
	{
		int direction = level->getData(x, y, z);
		if (static_cast<BaseRailTile *>(Tile::tiles[id])->usesDataBit)
		{
			usesDataBit = true;
			direction = direction & ~RAIL_DATA_BIT;
		}
		else
		{
			usesDataBit = false;
		}
		updateConnections(direction);
	}
}

BaseRailTile::Rail::~Rail()
{
	for( size_t i = 0; i < connections.size(); i++ )
	{
		delete connections[i];
	}
}

void BaseRailTile::Rail::updateConnections(int direction)
{
	if(m_bValidRail)
	{
		for( size_t i = 0; i < connections.size(); i++ )
		{
			delete connections[i];
		}
		connections.clear();
		MemSect(50);
		if (direction == DIR_FLAT_Z)
		{
			connections.push_back(new TilePos(x, y, z - 1));
			connections.push_back(new TilePos(x, y, z + 1));
		} else if (direction == DIR_FLAT_X)
		{
			connections.push_back(new TilePos(x - 1, y, z));
			connections.push_back(new TilePos(x + 1, y, z));
		} else if (direction == 2)
		{
			connections.push_back(new TilePos(x - 1, y, z));
			connections.push_back(new TilePos(x + 1, y + 1, z));
		} else if (direction == 3)
		{
			connections.push_back(new TilePos(x - 1, y + 1, z));
			connections.push_back(new TilePos(x + 1, y, z));
		} else if (direction == 4)
		{
			connections.push_back(new TilePos(x, y + 1, z - 1));
			connections.push_back(new TilePos(x, y, z + 1));
		} else if (direction == 5)
		{
			connections.push_back(new TilePos(x, y, z - 1));
			connections.push_back(new TilePos(x, y + 1, z + 1));
		} else if (direction == 6)
		{
			connections.push_back(new TilePos(x + 1, y, z));
			connections.push_back(new TilePos(x, y, z + 1));
		} else if (direction == 7)
		{
			connections.push_back(new TilePos(x - 1, y, z));
			connections.push_back(new TilePos(x, y, z + 1));
		} else if (direction == 8)
		{
			connections.push_back(new TilePos(x - 1, y, z));
			connections.push_back(new TilePos(x, y, z - 1));
		} else if (direction == 9)
		{
			connections.push_back(new TilePos(x + 1, y, z));
			connections.push_back(new TilePos(x, y, z - 1));
		}
		MemSect(0);
	}
}

void BaseRailTile::Rail::removeSoftConnections()
{
	if(m_bValidRail)
	{
		for (unsigned int i = 0; i < connections.size(); i++)
		{
			Rail *rail = getRail(connections[i]);
			if (rail == nullptr || !rail->connectsTo(this))
			{
				delete connections[i];
				connections.erase(connections.begin()+i);
				i--;
			} else
			{
				delete connections[i];
				MemSect(50);
				connections[i] =new TilePos(rail->x, rail->y, rail->z);
				MemSect(0);
			}
			delete rail;
		}
	}
}

bool BaseRailTile::Rail::hasRail(int x, int y, int z)
{
	if(!m_bValidRail) return false;
	if (isRail(level, x, y, z)) return true;
	if (isRail(level, x, y + 1, z)) return true;
	if (isRail(level, x, y - 1, z)) return true;
	return false;
}

BaseRailTile::Rail *BaseRailTile::Rail::getRail(TilePos *p)
{
	if(!m_bValidRail) return nullptr;
	if (isRail(level, p->x, p->y, p->z)) return new Rail(level, p->x, p->y, p->z);
	if (isRail(level, p->x, p->y + 1, p->z)) return new Rail(level, p->x, p->y + 1, p->z);
	if (isRail(level, p->x, p->y - 1, p->z)) return new Rail(level, p->x, p->y - 1, p->z);
	return nullptr;
}


bool BaseRailTile::Rail::connectsTo(Rail *rail)
{
	if(m_bValidRail)
	{
		for ( const auto& p : connections )
		{
			if (p->x == rail->x && p->z == rail->z)
			{
				return true;
			}
		}
	}
	return false;
}

bool BaseRailTile::Rail::hasConnection(int x, int y, int z)
{
	if(m_bValidRail)
	{
		for ( const auto& p : connections )
		{
			if (p->x == x && p->z == z)
			{
				return true;
			}
		}
	}
	return false;
}


int BaseRailTile::Rail::countPotentialConnections()
{
	int count = 0;

	if(m_bValidRail)
	{
		if (hasRail(x, y, z - 1)) count++;
		if (hasRail(x, y, z + 1)) count++;
		if (hasRail(x - 1, y, z)) count++;
		if (hasRail(x + 1, y, z)) count++;
	}

	return count;
}

bool BaseRailTile::Rail::canConnectTo(Rail *rail)
{
	if(!m_bValidRail) return false;
	if (connectsTo(rail)) return true;
	if (connections.size() == 2)
	{
		return false;
	}
	if (connections.empty())
	{
		return true;
	}

	return true;
}

void BaseRailTile::Rail::connectTo(Rail *rail)
{
	if(m_bValidRail)
	{
		MemSect(50);
		connections.push_back(new TilePos(rail->x, rail->y, rail->z));
		MemSect(0);

		bool n = hasConnection(x, y, z - 1);
		bool s = hasConnection(x, y, z + 1);
		bool w = hasConnection(x - 1, y, z);
		bool e = hasConnection(x + 1, y, z);

		int dir = -1;

		if (n || s) dir = DIR_FLAT_Z;
		if (w || e) dir = DIR_FLAT_X;

		if (!usesDataBit)
		{
			if (s && e && !n && !w) dir = 6;
			if (s && w && !n && !e) dir = 7;
			if (n && w && !s && !e) dir = 8;
			if (n && e && !s && !w) dir = 9;
		}
		if (dir == DIR_FLAT_Z)
		{
			if (isRail(level, x, y + 1, z - 1)) dir = 4;
			if (isRail(level, x, y + 1, z + 1)) dir = 5;
		}
		if (dir == DIR_FLAT_X)
		{
			if (isRail(level, x + 1, y + 1, z)) dir = 2;
			if (isRail(level, x - 1, y + 1, z)) dir = 3;
		}

		if (dir < 0) dir = DIR_FLAT_Z;

		int data = dir;
		if (usesDataBit)
		{
			data = (level->getData(x, y, z) & RAIL_DATA_BIT) | dir;
		}

		level->setData(x, y, z, data, Tile::UPDATE_ALL);
	}
}

bool BaseRailTile::Rail::hasNeighborRail(int x, int y, int z)
{
	if(!m_bValidRail) return false;
	TilePos tp(x,y,z);
	Rail *neighbor = getRail( &tp );
	if (neighbor == nullptr) return false;
	neighbor->removeSoftConnections();
	bool retval = neighbor->canConnectTo(this);
	delete neighbor;
	return retval;
}

void BaseRailTile::Rail::place(bool hasSignal, bool first)
{
	if(m_bValidRail)
	{
		bool n = hasNeighborRail(x, y, z - 1);
		bool s = hasNeighborRail(x, y, z + 1);
		bool w = hasNeighborRail(x - 1, y, z);
		bool e = hasNeighborRail(x + 1, y, z);

		int dir = -1;

		if ((n || s) && !w && !e) dir = DIR_FLAT_Z;
		if ((w || e) && !n && !s) dir = DIR_FLAT_X;

		if (!usesDataBit)
		{
			if (s && e && !n && !w) dir = 6;
			if (s && w && !n && !e) dir = 7;
			if (n && w && !s && !e) dir = 8;
			if (n && e && !s && !w) dir = 9;
		}
		if (dir == -1)
		{
			if (n || s) dir = DIR_FLAT_Z;
			if (w || e) dir = DIR_FLAT_X;

			if (!usesDataBit)
			{
				if (hasSignal)
				{
					if (s && e) dir = 6;
					if (w && s) dir = 7;
					if (e && n) dir = 9;
					if (n && w) dir = 8;
				} else {
					if (n && w) dir = 8;
					if (e && n) dir = 9;
					if (w && s) dir = 7;
					if (s && e) dir = 6;
				}
			}
		}

		if (dir == DIR_FLAT_Z)
		{
			if (isRail(level, x, y + 1, z - 1)) dir = 4;
			if (isRail(level, x, y + 1, z + 1)) dir = 5;
		}
		if (dir == DIR_FLAT_X)
		{
			if (isRail(level, x + 1, y + 1, z)) dir = 2;
			if (isRail(level, x - 1, y + 1, z)) dir = 3;
		}

		if (dir < 0) dir = DIR_FLAT_Z;

		updateConnections(dir);

		int data = dir;
		if (usesDataBit)
		{
			data = (level->getData(x, y, z) & RAIL_DATA_BIT) | dir;
		}

		if (first || level->getData(x, y, z) != data)
		{
			level->setData(x, y, z, data, Tile::UPDATE_ALL);

			for ( auto& it : connections )
			{
				Rail *neighbor = getRail(it);
				if (neighbor == nullptr) continue;
				neighbor->removeSoftConnections();

				if (neighbor->canConnectTo(this))
				{
					neighbor->connectTo(this);
				}
				delete neighbor;
			}
		}
	}
}

bool BaseRailTile::isRail(Level *level, int x, int y, int z)
{
	return isRail(level->getTile(x, y, z));
}

bool BaseRailTile::isRail(int id)
{
	return id == Tile::rail_Id || id == Tile::goldenRail_Id || id == Tile::detectorRail_Id || id == Tile::activatorRail_Id;
}

BaseRailTile::BaseRailTile(int id, bool usesDataBit) : Tile(id, Material::decoration, isSolidRender())
{
	this->usesDataBit = usesDataBit;
	setShape(0, 0, 0, 1, 2 / 16.0f, 1);

	iconTurn = nullptr;
}

bool BaseRailTile::isUsesDataBit()
{
	return usesDataBit;
}

AABB *BaseRailTile::getAABB(Level *level, int x, int y, int z)
{
	return nullptr;
}

bool BaseRailTile::blocksLight()
{
	return false;
}

bool BaseRailTile::isSolidRender(bool isServerLevel)
{
	return false;
}

HitResult *BaseRailTile::clip(Level *level, int xt, int yt, int zt, Vec3 *a, Vec3 *b)
{
	updateShape(level, xt, yt, zt);
	return Tile::clip(level, xt, yt, zt, a, b);
}

void BaseRailTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity) // 4J added forceData, forceEntity param
{
	int data = level->getData(x, y, z);
	if (data >= 2 && data <= 5)
	{
		setShape(0, 0, 0, 1, 2 / 16.0f + 0.5f, 1);
	} else
	{
		setShape(0, 0, 0, 1, 2 / 16.0f, 1);
	}
}

bool BaseRailTile::isCubeShaped()
{
	return false;
}

int BaseRailTile::getRenderShape()
{
	return Tile::SHAPE_RAIL;
}

int BaseRailTile::getResourceCount(Random random)
{
	return 1;
}

bool BaseRailTile::mayPlace(Level *level, int x, int y, int z)
{
	if (level->isTopSolidBlocking(x, y - 1, z))
	{
		return true;
	}
	return false;
}

void BaseRailTile::onPlace(Level *level, int x, int y, int z)
{
	if (!level->isClientSide)
	{
		updateDir(level, x, y, z, true);

		if (usesDataBit)
		{
			neighborChanged(level, x, y, z, id);
		}
	}
}

void BaseRailTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	if (level->isClientSide) return;

	int data = level->getData(x, y, z);
	int dir = data;
	if (usesDataBit) {
		dir = dir & RAIL_DIRECTION_MASK;
	}
	bool remove = false;

	if (!level->isTopSolidBlocking(x, y - 1, z)) remove = true;
	if (dir == 2 && !level->isTopSolidBlocking(x + 1, y, z)) remove = true;
	if (dir == 3 && !level->isTopSolidBlocking(x - 1, y, z)) remove = true;
	if (dir == 4 && !level->isTopSolidBlocking(x, y, z - 1)) remove = true;
	if (dir == 5 && !level->isTopSolidBlocking(x, y, z + 1)) remove = true;

	if (remove)
	{
		spawnResources(level, x, y, z, level->getData(x, y, z), 0);
		level->removeTile(x, y, z);
	}
	else
	{
		updateState(level, x, y, z, data, dir, type);
	}

}

void BaseRailTile::updateState(Level *level, int x, int y, int z, int data, int dir, int type)
{
}

void BaseRailTile::updateDir(Level *level, int x, int y, int z, bool first)
{
	if (level->isClientSide) return;
	Rail *rail = new Rail(level, x, y, z);
	rail->place(level->hasNeighborSignal(x, y, z), first);
	delete rail;
}

int BaseRailTile::getPistonPushReaction()
{
	// override the decoration material's reaction
	return Material::PUSH_NORMAL;
}

void BaseRailTile::onRemove(Level *level, int x, int y, int z, int id, int data)
{
	int dir = data;
	if (usesDataBit)
	{
		dir &= RAIL_DIRECTION_MASK;
	}

	Tile::onRemove(level, x, y, z, id, data);

	if (dir == 2 || dir == 3 || dir == 4 || dir == 5)
	{
		level->updateNeighborsAt(x, y + 1, z, id);
	}
	if (usesDataBit)
	{
		level->updateNeighborsAt(x, y, z, id);
		level->updateNeighborsAt(x, y - 1, z, id);
	}
}