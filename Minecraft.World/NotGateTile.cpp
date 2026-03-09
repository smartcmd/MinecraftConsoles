#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.redstone.h"
#include "NotGateTile.h"
#include "SoundTypes.h"
#include "net.minecraft.world.h"

unordered_map<Level *, deque<NotGateTile::Toggle> *> NotGateTile::recentToggles = unordered_map<Level *, deque<NotGateTile::Toggle> *>();

// 4J - added, to tie in with other changes brought forward from 1.3.2 to associate toggles with a level. In addition to what the java
// version does, we are also removing any references to levels that we are storing when they hit their dtor.
void NotGateTile::removeLevelReferences(Level *level)
{
	if( recentToggles.find(level) != recentToggles.end() )
	{
		delete recentToggles[level];
		recentToggles.erase(level);
	}
}

bool NotGateTile::isToggledTooFrequently(Level *level, int x, int y, int z, bool add)
{
	// 4J - brought forward changes to associate toggles with a level from 1.3.2
	if( recentToggles.find(level) == recentToggles.end() )
	{
		recentToggles[level] = new deque<Toggle>;
	}
	if (add) recentToggles[level]->push_back(Toggle(x, y, z, level->getGameTime()));
	int count = 0;

	for (const auto& it : *recentToggles[level])
	{
		if (it.x == x && it.y == y && it.z == z)
		{
			count++;
			if (count >= MAX_RECENT_TOGGLES)
			{
				return true;
			}
		}
	}
	return false;
}

NotGateTile::NotGateTile(int id, bool on) : TorchTile(id)
{
	this->on = on;
	this->setTicking(true);
}

int NotGateTile::getTickDelay(Level *level)
{
	return 2;
}

void NotGateTile::onPlace(Level *level, int x, int y, int z)
{
	if (level->getData(x, y, z) == 0) TorchTile::onPlace(level, x, y, z);
	if (on)
	{
		level->updateNeighborsAt(x, y - 1, z, id);
		level->updateNeighborsAt(x, y + 1, z, id);
		level->updateNeighborsAt(x - 1, y, z, id);
		level->updateNeighborsAt(x + 1, y, z, id);
		level->updateNeighborsAt(x, y, z - 1, id);
		level->updateNeighborsAt(x, y, z + 1, id);
	}
}

void NotGateTile::onRemove(Level *level, int x, int y, int z, int id, int data)
{
	if (on)
	{
		level->updateNeighborsAt(x, y - 1, z, this->id);
		level->updateNeighborsAt(x, y + 1, z, this->id);
		level->updateNeighborsAt(x - 1, y, z, this->id);
		level->updateNeighborsAt(x + 1, y, z, this->id);
		level->updateNeighborsAt(x, y, z - 1, this->id);
		level->updateNeighborsAt(x, y, z + 1, this->id);
	}
}

int NotGateTile::getSignal(LevelSource *level, int x, int y, int z, int face)
{
	if (!on) return Redstone::SIGNAL_NONE;

	int dir = level->getData(x, y, z);

	if (dir == 5 && face == 1) return Redstone::SIGNAL_NONE;
	if (dir == 3 && face == 3) return Redstone::SIGNAL_NONE;
	if (dir == 4 && face == 2) return Redstone::SIGNAL_NONE;
	if (dir == 1 && face == 5) return Redstone::SIGNAL_NONE;
	if (dir == 2 && face == 4) return Redstone::SIGNAL_NONE;

	return Redstone::SIGNAL_MAX;
}

bool NotGateTile::hasNeighborSignal(Level *level, int x, int y, int z)
{
	int dir = level->getData(x, y, z);

	if (dir == 5 && level->hasSignal(x, y - 1, z, 0)) return true;
	if (dir == 3 && level->hasSignal(x, y, z - 1, 2)) return true;
	if (dir == 4 && level->hasSignal(x, y, z + 1, 3)) return true;
	if (dir == 1 && level->hasSignal(x - 1, y, z, 4)) return true;
	if (dir == 2 && level->hasSignal(x + 1, y, z, 5)) return true;
	return false;
}

void NotGateTile::tick(Level *level, int x, int y, int z, Random *random)
{
	bool neighborSignal = hasNeighborSignal(level, x, y, z);

	// 4J - brought forward changes from 1.3.2 to associate toggles with level
	if( recentToggles.find(level) != recentToggles.end() )
	{
		deque<Toggle> *toggles = recentToggles[level];
		while (!toggles->empty() && level->getGameTime() - toggles->front().when > RECENT_TOGGLE_TIMER)
		{
			toggles->pop_front();
		}
	}

	if (on)
	{
		if (neighborSignal)
		{
			level->setTileAndData(x, y, z, Tile::redstoneTorch_off_Id, level->getData(x, y, z), Tile::UPDATE_ALL);

			if (isToggledTooFrequently(level, x, y, z, true))
			{
				app.DebugPrintf("Torch at (%d,%d,%d) has toggled too many times\n",x,y,z);

				level->playSound(x + 0.5f, y + 0.5f, z + 0.5f, eSoundType_RANDOM_FIZZ, 0.5f, 2.6f + (level->random->nextFloat() - level->random->nextFloat()) * 0.8f);
				for (int i = 0; i < 5; i++)
				{
					double xx = x + random->nextDouble() * 0.6 + 0.2;
					double yy = y + random->nextDouble() * 0.6 + 0.2;
					double zz = z + random->nextDouble() * 0.6 + 0.2;

					level->addParticle(eParticleType_smoke, xx, yy, zz, 0, 0, 0);
				}
			}
		}
	}
	else
	{
		if (!neighborSignal)
		{
			if (!isToggledTooFrequently(level, x, y, z, false))
			{
				level->setTileAndData(x, y, z, Tile::redstoneTorch_on_Id, level->getData(x, y, z), Tile::UPDATE_ALL);
			}
			else
			{
				app.DebugPrintf("Torch at (%d,%d,%d) has toggled too many times\n",x,y,z);
			}
		}
	}
}

void NotGateTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	if (checkDoPop(level, x, y, z, type))
	{
		return;
	}

	bool neighborSignal = hasNeighborSignal(level, x, y, z);
	if ((on && neighborSignal) || (!on && !neighborSignal))
	{
		level->addToTickNextTick(x, y, z, id, getTickDelay(level));
	}
}

int NotGateTile::getDirectSignal(LevelSource *level, int x, int y, int z, int face)
{
	if (face == 0)
	{
		return getSignal(level, x, y, z, face);
	}
	return Redstone::SIGNAL_NONE;
}

int NotGateTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return Tile::redstoneTorch_on_Id;
}

bool NotGateTile::isSignalSource()
{
	return true;
}

void NotGateTile::animateTick(Level *level, int xt, int yt, int zt, Random *random)
{
	if (!on) return;
	int dir = level->getData(xt, yt, zt);
	double x = xt + 0.5f + (random->nextFloat() - 0.5f) * 0.2;
	double y = yt + 0.7f + (random->nextFloat() - 0.5f) * 0.2;
	double z = zt + 0.5f + (random->nextFloat() - 0.5f) * 0.2;
	double h = 0.22f;
	double r = 0.27f;
	if (dir == 1)
	{
		level->addParticle(eParticleType_reddust, x - r, y + h, z, 0, 0, 0);
	}
	else if(dir == 2)
	{
		level->addParticle(eParticleType_reddust, x + r, y + h, z, 0, 0, 0);
	}
	else if (dir == 3)
	{
		level->addParticle(eParticleType_reddust, x, y + h, z - r, 0, 0, 0);
	}
	else if (dir == 4)
	{
		level->addParticle(eParticleType_reddust, x, y + h, z + r, 0, 0, 0);
	}
	else
	{
		level->addParticle(eParticleType_reddust, x, y, z, 0, 0, 0);
	}

}

int NotGateTile::cloneTileId(Level *level, int x, int y, int z)
{
	return Tile::redstoneTorch_on_Id;
}

void NotGateTile::levelTimeChanged(Level *level, int64_t delta, int64_t newTime)
{
	deque<Toggle> *toggles = recentToggles[level];

	if (toggles != nullptr)
	{
		for (auto& toggle : *toggles)
		{
			toggle.when += delta;
		}
	}
}

bool NotGateTile::isMatching(int id)
{
	return id == Tile::redstoneTorch_off_Id || id == Tile::redstoneTorch_on_Id;
}