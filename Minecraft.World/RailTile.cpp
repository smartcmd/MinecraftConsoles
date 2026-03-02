#include "stdafx.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.h"
#include "RailTile.h"

RailTile::Rail::Rail(Level *level, int x, int y, int z)
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
		if (((RailTile *) Tile::tiles[id])->usesDataBit)
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

RailTile::Rail::~Rail()
{
	for( int i = 0; i < connections.size(); i++ )
	{
		delete connections[i];
	}
}

void RailTile::Rail::updateConnections(int direction)
{
	if(m_bValidRail)
	{
		for( int i = 0; i < connections.size(); i++ )
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

void RailTile::Rail::removeSoftConnections()
{
	if(m_bValidRail)
	{
		for (unsigned int i = 0; i < connections.size(); i++)
		{
			Rail *rail = getRail(connections[i]);
			if (rail == NULL || !rail->connectsTo(this))
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

bool RailTile::Rail::hasRail(int x, int y, int z)
{
	if(!m_bValidRail) return false;
	if (isRail(level, x, y, z)) return true;
	if (isRail(level, x, y + 1, z)) return true;
	if (isRail(level, x, y - 1, z)) return true;
	return false;
}

RailTile::Rail *RailTile::Rail::getRail(TilePos *p)
{
	if(!m_bValidRail) return NULL;
	if (isRail(level, p->x, p->y, p->z)) return new Rail(level, p->x, p->y, p->z);
	if (isRail(level, p->x, p->y + 1, p->z)) return new Rail(level, p->x, p->y + 1, p->z);
	if (isRail(level, p->x, p->y - 1, p->z)) return new Rail(level, p->x, p->y - 1, p->z);
	return NULL;
}


bool RailTile::Rail::connectsTo(Rail *rail)
{
	if(m_bValidRail)
	{
		AUTO_VAR(itEnd, connections.end());
		for (AUTO_VAR(it, connections.begin()); it != itEnd; it++)
		{
			TilePos *p = *it; //connections[i];
			if (p->x == rail->x && p->z == rail->z)
			{
				return true;
			}
		}
	}
	return false;
}

bool RailTile::Rail::hasConnection(int x, int y, int z)
{
	if(m_bValidRail)
	{
		AUTO_VAR(itEnd, connections.end());
		for (AUTO_VAR(it, connections.begin()); it != itEnd; it++)
		{
			TilePos *p = *it; //connections[i];
			if (p->x == x && p->z == z)
			{
				return true;
			}
		}
	}
	return false;
}


int RailTile::Rail::countPotentialConnections()
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

bool RailTile::Rail::canConnectTo(Rail *rail)
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

	TilePos *c = connections[0];

	return true;
}

void RailTile::Rail::connectTo(Rail *rail)
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

		level->setData(x, y, z, data);
	}
}

bool RailTile::Rail::hasNeighborRail(int x, int y, int z)
{
	if(!m_bValidRail) return false;
	TilePos tp(x,y,z);
	Rail *neighbor = getRail( &tp );
	if (neighbor == NULL) return false;
	neighbor->removeSoftConnections();
	bool retval = neighbor->canConnectTo(this);
	delete neighbor;
	return retval;
}

void RailTile::Rail::place(bool hasSignal, bool first)
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
			level->setData(x, y, z, data);

			AUTO_VAR(itEnd, connections.end());
			for (AUTO_VAR(it, connections.begin()); it != itEnd; it++)
			{
				Rail *neighbor = getRail(*it);
				if (neighbor == NULL) continue;
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

bool RailTile::isRail(Level *level, int x, int y, int z)
{
	int tile = level->getTile(x, y, z);
	return tile == Tile::rail_Id || tile == Tile::goldenRail_Id || tile == Tile::detectorRail_Id;

}

bool RailTile::isRail(int id)
{
	return id == Tile::rail_Id || id == Tile::goldenRail_Id || id == Tile::detectorRail_Id;
}

RailTile::RailTile(int id, bool usesDataBit) : Tile(id, Material::decoration, isSolidRender())
{
	this->usesDataBit = usesDataBit;
	this->setShape(0, 0, 0, 1, 2 / 16.0f, 1);

	iconTurn = NULL;
}

bool RailTile::isUsesDataBit()
{
	return usesDataBit;
}

AABB *RailTile::getAABB(Level *level, int x, int y, int z)
{
	return NULL;
}

bool RailTile::blocksLight()
{
	return false;
}

bool RailTile::isSolidRender(bool isServerLevel)
{
	return false;
}

HitResult *RailTile::clip(Level *level, int xt, int yt, int zt, Vec3 *a, Vec3 *b)
{
	updateShape(level, xt, yt, zt);
	return Tile::clip(level, xt, yt, zt, a, b);
}

void RailTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity) // 4J added forceData, forceEntity param
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

Icon *RailTile::getTexture(int face, int data)
{
	if (usesDataBit)
	{
		if (id == Tile::goldenRail_Id)
		{
			if ((data & RAIL_DATA_BIT) == 0)
			{
				return icon;
			}
			else
			{
				return iconTurn; // Actually the powered rail on version
			}
		}
	} else if (data >= 6) return iconTurn;
	return icon;
}

bool RailTile::isCubeShaped()
{
	return false;
}

int RailTile::getRenderShape()
{
	return Tile::SHAPE_RAIL;
}

int RailTile::getResourceCount(Random random)
{
	return 1;
}

bool RailTile::mayPlace(Level *level, int x, int y, int z)
{
	if (level->isTopSolidBlocking(x, y - 1, z))
	{
		return true;
	}
	return false;
}

void RailTile::onPlace(Level *level, int x, int y, int z)
{
	if (!level->isClientSide)
	{
		updateDir(level, x, y, z, true);

		if (id == Tile::goldenRail_Id)
		{
			neighborChanged(level, x, y, z, id);
		}
	}
}

void RailTile::neighborChanged(Level *level, int x, int y, int z, int type)
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
		this->spawnResources(level, x, y, z, level->getData(x, y, z), 0);
		level->setTile(x, y, z, 0);
	}
	else
	{
		if (id == Tile::goldenRail_Id)
		{
			bool signal = level->hasNeighborSignal(x, y, z);
			signal = signal || findGoldenRailSignal(level, x, y, z, data, true, 0) || findGoldenRailSignal(level, x, y, z, data, false, 0);

			bool changed = false;
			if (signal && (data & RAIL_DATA_BIT) == 0)
			{
				level->setData(x, y, z, dir | RAIL_DATA_BIT);
				changed = true;
			} else if (!signal && (data & RAIL_DATA_BIT) != 0)
			{
				level->setData(x, y, z, dir);
				changed = true;
			}

			// usually the level only updates neighbors that are in the same
			// y plane as the current tile, but sloped rails may need to
			// update tiles above or below it as well
			if (changed) {
				level->updateNeighborsAt(x, y - 1, z, id);
				if (dir == 2 || dir == 3 || dir == 4 || dir == 5)
				{
					level->updateNeighborsAt(x, y + 1, z, id);
				}
			}
		}
		else if (type > 0 && Tile::tiles[type]->isSignalSource() && !usesDataBit)
		{
			Rail *rail = new Rail(level, x, y, z);
			if (rail->countPotentialConnections() == 3)
			{
				updateDir(level, x, y, z, false);
			}
			delete rail;
		}
	}

}

void RailTile::updateDir(Level *level, int x, int y, int z, bool first)
{
	if (level->isClientSide) return;
	Rail *rail = new Rail(level, x, y, z);
	rail->place(level->hasNeighborSignal(x, y, z), first);
	delete rail;
}

bool RailTile::findGoldenRailSignal(Level *level, int x, int y, int z, int data, bool forward, int searchDepth)
{
	if (searchDepth >= 8)
	{
		return false;
	}

	int dir = data & RAIL_DIRECTION_MASK;

	bool checkBelow = true;
	switch (dir)
	{
	case DIR_FLAT_Z:
		if (forward)
		{
			z++;
		} else {
			z--;
		}
		break;
	case DIR_FLAT_X:
		if (forward)
		{
			x--;
		} else {
			x++;
		}
		break;
	case 2:
		if (forward)
		{
			x--;
		} else
		{
			x++;
			y++;
			checkBelow = false;
		}
		dir = DIR_FLAT_X;
		break;
	case 3:
		if (forward)
		{
			x--;
			y++;
			checkBelow = false;
		} else {
			x++;
		}
		dir = DIR_FLAT_X;
		break;
	case 4:
		if (forward)
		{
			z++;
		} else {
			z--;
			y++;
			checkBelow = false;
		}
		dir = DIR_FLAT_Z;
		break;
	case 5:
		if (forward)
		{
			z++;
			y++;
			checkBelow = false;
		} else
		{
			z--;
		}
		dir = DIR_FLAT_Z;
		break;
	}

	if (isGoldenRailWithPower(level, x, y, z, forward, searchDepth, dir))
	{
		return true;
	}
	if (checkBelow && isGoldenRailWithPower(level, x, y - 1, z, forward, searchDepth, dir))
	{
		return true;
	}
	return false;

}

bool RailTile::isGoldenRailWithPower(Level *level, int x, int y, int z, bool forward, int searchDepth, int dir)
{
	int tile = level->getTile(x, y, z);
	if (tile == Tile::goldenRail_Id)
	{
		int tileData = level->getData(x, y, z);
		int myDir = tileData & RAIL_DIRECTION_MASK;

		if (dir == DIR_FLAT_X && (myDir == DIR_FLAT_Z || myDir == 4 || myDir == 5))
		{
			return false;
		}
		if (dir == DIR_FLAT_Z && (myDir == DIR_FLAT_X || myDir == 2 || myDir == 3))
		{
			return false;
		}

		if ((tileData & RAIL_DATA_BIT) != 0)
		{
			if (level->hasNeighborSignal(x, y, z))
			{
				return true;
			}
			else
			{
				return findGoldenRailSignal(level, x, y, z, tileData, forward, searchDepth + 1);
			}
		}
	}
	return false;
}

int RailTile::getPistonPushReaction()
{
	return Material::PUSH_NORMAL;
}

void RailTile::registerIcons(IconRegister *iconRegister)
{
	Tile::registerIcons(iconRegister);
	if(id == Tile::goldenRail_Id)
	{
		iconTurn = iconRegister->registerIcon(L"goldenRail_powered");
	}
	else
	{
		iconTurn = iconRegister->registerIcon(L"rail_turn");
	}
}