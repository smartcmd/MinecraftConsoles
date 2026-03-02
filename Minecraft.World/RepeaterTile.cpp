#include "stdafx.h"
#include "net.minecraft.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.redstone.h"
#include "RepeaterTile.h"

const double RepeaterTile::DELAY_RENDER_OFFSETS[4] = {-1.0f / 16.0f, 1.0f / 16.0f, 3.0f / 16.0f, 5.0f / 16.0f};
const int RepeaterTile::DELAYS[4]= {1, 2, 3, 4};

RepeaterTile::RepeaterTile(int id, bool on) : DiodeTile(id, on)
{
}

bool RepeaterTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly)
{
	if (soundOnly)	return false;

	int data = level->getData(x, y, z);
	int delay = (data & DELAY_MASK) >> DELAY_SHIFT;
	delay = ((delay + 1) << DELAY_SHIFT) & DELAY_MASK;

	level->setData(x, y, z, delay | (data & DIRECTION_MASK), Tile::UPDATE_ALL);
	return true;
}

int RepeaterTile::getTurnOnDelay(int data)
{
	return DELAYS[(data & DELAY_MASK) >> DELAY_SHIFT] * 2;
}

DiodeTile *RepeaterTile::getOnTile()
{
	return Tile::diode_on;
}

DiodeTile *RepeaterTile::getOffTile()
{
	return Tile::diode_off;
}

int RepeaterTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return Item::repeater_Id;
}

int RepeaterTile::cloneTileId(Level *level, int x, int y, int z)
{
	return Item::repeater_Id;
}

int RepeaterTile::getRenderShape()
{
	return SHAPE_REPEATER;
}

bool RepeaterTile::isLocked(LevelSource *level, int x, int y, int z, int data)
{
	return getAlternateSignal(level, x, y, z, data) > Redstone::SIGNAL_NONE;
}

bool RepeaterTile::isAlternateInput(int tile)
{
	return isDiode(tile);
}

void RepeaterTile::animateTick(Level *level, int xt, int yt, int zt, Random *random)
{
	if (!on) return;
	int data = level->getData(xt, yt, zt);
	int dir = getDirection(data);

	double x = xt + 0.5f + (random->nextFloat() - 0.5f) * 0.2;
	double y = yt + 0.4f + (random->nextFloat() - 0.5f) * 0.2;
	double z = zt + 0.5f + (random->nextFloat() - 0.5f) * 0.2;

	double xo = 0;
	double zo = 0;

	if (random->nextInt(2) == 0)
	{
		// spawn on receiver
		switch (dir)
		{
		case Direction::SOUTH:
			zo = -5.0f / 16.0f;
			break;
		case Direction::NORTH:
			zo = 5.0f / 16.0f;
			break;
		case Direction::EAST:
			xo = -5.0f / 16.0f;
			break;
		case Direction::WEST:
			xo = 5.0f / 16.0f;
			break;
		}
	}
	else
	{
		// spawn on transmitter
		int delay = (data & DELAY_MASK) >> DELAY_SHIFT;
		switch (dir)
		{
		case Direction::SOUTH:
			zo = DELAY_RENDER_OFFSETS[delay];
			break;
		case Direction::NORTH:
			zo = -DELAY_RENDER_OFFSETS[delay];
			break;
		case Direction::EAST:
			xo = DELAY_RENDER_OFFSETS[delay];
			break;
		case Direction::WEST:
			xo = -DELAY_RENDER_OFFSETS[delay];
			break;
		}
	}

	level->addParticle(eParticleType_reddust, x + xo, y, z + zo, 0, 0, 0);
}

void RepeaterTile::onRemove(Level *level, int x, int y, int z, int id, int data)
{
	DiodeTile::onRemove(level, x, y, z, id, data);
	updateNeighborsInFront(level, x, y, z);
}

bool RepeaterTile::TestUse()
{
	return true;
}