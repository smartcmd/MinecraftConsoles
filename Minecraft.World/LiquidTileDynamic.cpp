#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "LiquidTileDynamic.h"
#include "net.minecraft.world.level.dimension.h"

LiquidTileDynamic::LiquidTileDynamic(int id, Material *material) : LiquidTile(id, material)
{
	maxCount = 0;
	result = new bool[4];
	dist = new int[4];
	m_iterativeInstatick = false;
}

LiquidTileDynamic::~LiquidTileDynamic()
{
	delete[] result;
	delete[] dist;
}

void LiquidTileDynamic::setStatic(Level *level, int x, int y, int z)
{
	int d = level->getData(x, y, z);
	level->setTileAndData(x, y, z, id + 1, d, Tile::UPDATE_CLIENTS);
}

bool LiquidTileDynamic::isPathfindable(LevelSource *level, int x, int y, int z)
{
	return material != Material::lava;
}

void LiquidTileDynamic::iterativeTick(Level *level, int x, int y, int z, Random *random)
{
	m_tilesToTick.push_back(LiquidTickData(level, x,y,z,random));
	
	int failsafe = 100;
	while((m_tilesToTick.size() > 0) && ( failsafe > 0 ) )
	{
		LiquidTickData tickData = m_tilesToTick.front();
		m_tilesToTick.pop_front();
		mainTick(tickData.level, tickData.x, tickData.y, tickData.z, tickData.random);
		failsafe--;
	}
	m_tilesToTick.clear();
}

void LiquidTileDynamic::tick(Level *level, int x, int y, int z, Random *random)
{
	if(!m_iterativeInstatick && level->getInstaTick() )
	{
		m_iterativeInstatick = true;
		iterativeTick(level, x, y, z, random);
		m_iterativeInstatick = false;
	}
	else if(m_iterativeInstatick && level->getInstaTick())
	{
		m_tilesToTick.push_back(LiquidTickData(level, x,y,z,random));
	}
	else
	{
		mainTick(level, x, y, z, random);
	}
}

// 4J Stu - Split off what was the tick function to be able to change between recursive and iterative ticking
// This is to fix the stack overflow that occurs sometimes when instaticking on level gen.
void LiquidTileDynamic::mainTick(Level *level, int x, int y, int z, Random *random)
{
	int depth = getDepth(level, x, y, z);

	int dropOff = 1;
	if (material == Material::lava && !level->dimension->ultraWarm) dropOff = 2;

	bool becomeStatic = true;
	int tickDelay = getTickDelay(level);
	if (depth > 0)
	{
		int highest = -100;
		maxCount = 0;
		highest = getHighest(level, x - 1, y, z, highest);
		highest = getHighest(level, x + 1, y, z, highest);
		highest = getHighest(level, x, y, z - 1, highest);
		highest = getHighest(level, x, y, z + 1, highest);

		int newDepth = highest + dropOff;
		if (newDepth >= 8 || highest < 0)
		{
			newDepth = -1;
		}
		if (getDepth(level, x, y + 1, z) >= 0)
		{
			int above = getDepth(level, x, y + 1, z);
			if (above >= 8) newDepth = above;
			else newDepth = above + 8;
		}
		if (maxCount >= 2 && material == Material::water)
		{
			// Only spread spring if it's on top of an existing spring, or
			// on top of solid ground.
			if (level->getMaterial(x, y - 1, z)->isSolid())
			{
				newDepth = 0;
			}
			else if (level->getMaterial(x, y - 1, z) == material && level->getData(x, y - 1, z) == 0)
			{
				newDepth = 0;
			}
		}
		if (material == Material::lava)
		{
			if (depth < 8 && newDepth < 8)
			{
				if (newDepth > depth)
				{
					if (random->nextInt(4) != 0)
					{
						tickDelay = tickDelay * 4;
					}
				}
			}
		}
		if (newDepth == depth)
		{
			if (becomeStatic)
			{
				setStatic(level, x, y, z);
			}
		}
		else
		{
			depth = newDepth;
			if (depth < 0)
			{
				level->removeTile(x, y, z);
			}
			else
			{
				level->setData(x, y, z, depth, Tile::UPDATE_CLIENTS);
				level->addToTickNextTick(x, y, z, id, tickDelay);
				level->updateNeighborsAt(x, y, z, id);
			}
		}
	}
	else
	{
		setStatic(level, x, y, z);
	}
	if (canSpreadTo(level, x, y - 1, z))
	{
		if (material == Material::lava)
		{
			if (level->getMaterial(x, y - 1, z) == Material::water)
			{
				level->setTileAndUpdate(x, y - 1, z, Tile::stone_Id);
				fizz(level, x, y - 1, z);
				return;
			}
		}

		if (depth >= 8) trySpreadTo(level, x, y - 1, z, depth);
		else trySpreadTo(level, x, y - 1, z, depth + 8);
	}
	else if (depth >= 0 && (depth == 0 || isWaterBlocking(level, x, y - 1, z)))
	{
		bool *spreads = getSpread(level, x, y, z);
		int neighbor = depth + dropOff;
		if (depth >= 8)
		{
			neighbor = 1;
		}
		if (neighbor >= 8) return;
		if (spreads[0]) trySpreadTo(level, x - 1, y, z, neighbor);
		if (spreads[1]) trySpreadTo(level, x + 1, y, z, neighbor);
		if (spreads[2]) trySpreadTo(level, x, y, z - 1, neighbor);
		if (spreads[3]) trySpreadTo(level, x, y, z + 1, neighbor);
	}
}

void LiquidTileDynamic::trySpreadTo(Level *level, int x, int y, int z, int neighbor)
{
	if (canSpreadTo(level, x, y, z))
	{
		{
			int old = level->getTile(x, y, z);
			if (old > 0)
			{
				if (material == Material::lava)
				{
					fizz(level, x, y, z);
				}
				else
				{
					Tile::tiles[old]->spawnResources(level, x, y, z, level->getData(x, y, z), 0);
				}
			}
		}
		level->setTileAndData(x, y, z, id, neighbor, Tile::UPDATE_ALL);
	}
}

int LiquidTileDynamic::getSlopeDistance(Level *level, int x, int y, int z, int pass, int from)
{
	int lowest = 1000;
	for (int d = 0; d < 4; d++)
	{
		if (d == 0 && from == 1) continue;
		if (d == 1 && from == 0) continue;
		if (d == 2 && from == 3) continue;
		if (d == 3 && from == 2) continue;

		int xx = x;
		int yy = y;
		int zz = z;

		if (d == 0) xx--;
		if (d == 1) xx++;
		if (d == 2) zz--;
		if (d == 3) zz++;

		if (isWaterBlocking(level, xx, yy, zz))
		{
			continue;
		} else if (level->getMaterial(xx, yy, zz) == material && level->getData(xx, yy, zz) == 0)
		{
			continue;
		}
		else
		{
			if (isWaterBlocking(level, xx, yy - 1, zz))
			{
				if (pass < 4)
				{
					int v = getSlopeDistance(level, xx, yy, zz, pass + 1, d);
					if (v < lowest) lowest = v;
				}
			}
			else
			{
				return pass;
			}
		}
	}
	return lowest;

}

bool *LiquidTileDynamic::getSpread(Level *level, int x, int y, int z)
{
	for (int d = 0; d < 4; d++)
	{
		dist[d] = 1000;
		int xx = x;
		int yy = y;
		int zz = z;

		if (d == 0) xx--;
		if (d == 1) xx++;
		if (d == 2) zz--;
		if (d == 3) zz++;
		if (isWaterBlocking(level, xx, yy, zz))
		{
			continue;
		}
		else if (level->getMaterial(xx, yy, zz) == material && level->getData(xx, yy, zz) == 0)
		{
			continue;
		} 

		{
			if (isWaterBlocking(level, xx, yy - 1, zz))
			{
				dist[d] = getSlopeDistance(level, xx, yy, zz, 1, d);
			}
			else
			{
				dist[d] = 0;
			}
		}
	}

	int lowest = dist[0];
	for (int d = 1; d < 4; d++)
	{
		if (dist[d] < lowest) lowest = dist[d];
	}


	for (int d = 0; d < 4; d++)
	{
		result[d] = (dist[d] == lowest);
	}
	return result;

}

bool LiquidTileDynamic::isWaterBlocking(Level *level, int x, int y, int z)
{
	int t = level->getTile(x, y, z);
	if (t == Tile::door_wood_Id || t == Tile::door_iron_Id || t == Tile::sign_Id || t == Tile::ladder_Id || t == Tile::reeds_Id)
	{
		return true;
	}
	if (t == 0) return false;
	Material *m = Tile::tiles[t]->material;
	if (m == Material::portal) return true;
	if (m->blocksMotion()) return true;
	return false;
}

int LiquidTileDynamic::getHighest(Level *level, int x, int y, int z, int current)
{
	int d = getDepth(level, x, y, z);
	if (d < 0) return current;
	if (d == 0) maxCount++;
	if (d >= 8)
	{
		d = 0;
	}
	return current < 0 || d < current ? d : current;
}

bool LiquidTileDynamic::canSpreadTo(Level *level, int x, int y, int z)
{
	// 4J added - don't try and spread out of our restricted map. If we don't do this check then tiles at the edge of the world will try and spread outside as the outside tiles report that they contain
	// only air. The fact that this successfully spreads then updates the neighbours of the tile outside of the map, one of which is the original tile just inside the map, which gets set back to being
	// dynamic, and added to the pending ticks array.
	int xc = x >> 4;
	int zc = z >> 4;
	int ix = xc + (level->chunkSourceXZSize/2);
	int iz = zc + (level->chunkSourceXZSize/2);
	if( ( ix < 0 ) || ( ix >= level->chunkSourceXZSize ) ) return false;
	if( ( iz < 0 ) || ( iz >= level->chunkSourceXZSize ) ) return false;

	Material *target = level->getMaterial(x, y, z);
	if (target == material) return false;
	if (target == Material::lava) return false;
	return !isWaterBlocking(level, x, y, z);
}

void LiquidTileDynamic::onPlace(Level *level, int x, int y, int z)
{
	LiquidTile::onPlace(level, x, y, z);
	if (level->getTile(x, y, z) == id)
	{
		level->addToTickNextTick(x, y, z, id, getTickDelay(level));
	}
}

bool LiquidTileDynamic::canInstantlyTick()
{
	return true;
}