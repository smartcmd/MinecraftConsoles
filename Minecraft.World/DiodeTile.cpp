#include "stdafx.h"
#include "net.minecraft.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.h"
#include "DiodeTile.h"

const double DiodeTile::DELAY_RENDER_OFFSETS[4] = { -1.0f / 16.0f, 1.0f / 16.0f, 3.0f / 16.0f, 5.0f / 16.0f };
const int DiodeTile::DELAYS[4] = { 1, 2, 3, 4 };

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
	bool sourceOn = getSourceSignal(level, x, y, z, data);
	if (on && !sourceOn)
	{
		level->setTileAndData(x, y, z, Tile::diode_off_Id, data);
	}
	else if (!on)
	{
		// when off-diodes are ticked, they always turn on for one tick and
		// then off again if necessary
		level->setTileAndData(x, y, z, Tile::diode_on_Id, data);
		if (!sourceOn)
		{
			int delay = (data & DELAY_MASK) >> DELAY_SHIFT;
			level->addToTickNextTick(x, y, z, Tile::diode_on_Id, DELAYS[delay] * 2);
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
			return Tile::notGate_on->getTexture(face);
		}
		return Tile::notGate_off->getTexture(face);
	}
	if (face == Facing::UP)
	{
		return icon;
	}
	// edge of stone half-step
	return Tile::stoneSlab->getTexture(Facing::UP);
}

void DiodeTile::registerIcons(IconRegister *iconRegister)
{
	icon = iconRegister->registerIcon(on ? L"repeater_lit" : L"repeater");
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

bool DiodeTile::getDirectSignal(Level *level, int x, int y, int z, int dir)
{
	return getSignal(level, x, y, z, dir);
}

bool DiodeTile::getSignal(LevelSource *level, int x, int y, int z, int facing)
{
	if (!on)
	{
		return false;
	}

	int dir = getDirection(level->getData(x, y, z));

	if (dir == Direction::SOUTH && facing == Facing::SOUTH) return true;
	if (dir == Direction::WEST && facing == Facing::WEST) return true;
	if (dir == Direction::NORTH && facing == Facing::NORTH) return true;
	if (dir == Direction::EAST && facing == Facing::EAST) return true;

	return false;
}

void DiodeTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	if (!canSurvive(level, x, y, z))
	{
		this->spawnResources(level, x, y, z, level->getData(x, y, z), 0);
		level->setTile(x, y, z, 0);
		level->updateNeighborsAt(x + 1, y, z, id);
		level->updateNeighborsAt(x - 1, y, z, id);
		level->updateNeighborsAt(x, y, z + 1, id);
		level->updateNeighborsAt(x, y, z - 1, id);
		level->updateNeighborsAt(x, y - 1, z, id);
		level->updateNeighborsAt(x, y + 1, z, id);
		return;
	}

	int data = level->getData(x, y, z);

	bool sourceOn = getSourceSignal(level, x, y, z, data);
	int delay = (data & DELAY_MASK) >> DELAY_SHIFT;
	if ( (on && !sourceOn) || (!on && sourceOn))
	{
		level->addToTickNextTick(x, y, z, id, DELAYS[delay] * 2);
	}
}

bool DiodeTile::getSourceSignal(Level *level, int x, int y, int z, int data)
{
	int dir = getDirection(data);
	switch (dir)
	{
		case Direction::SOUTH:
            return level->getSignal(x, y, z + 1, Facing::SOUTH) || (level->getTile(x, y, z + 1) == Tile::redStoneDust_Id && level->getData(x, y, z + 1) > 0);
        case Direction::NORTH:
            return level->getSignal(x, y, z - 1, Facing::NORTH) || (level->getTile(x, y, z - 1) == Tile::redStoneDust_Id && level->getData(x, y, z - 1) > 0);
        case Direction::EAST:
            return level->getSignal(x + 1, y, z, Facing::EAST) || (level->getTile(x + 1, y, z) == Tile::redStoneDust_Id && level->getData(x + 1, y, z) > 0);
        case Direction::WEST:
            return level->getSignal(x - 1, y, z, Facing::WEST) || (level->getTile(x - 1, y, z) == Tile::redStoneDust_Id && level->getData(x - 1, y, z) > 0);   
	}
	return false;
}

// 4J-PB - Adding a TestUse for tooltip display
bool DiodeTile::TestUse()
{
	return true;
}

bool DiodeTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly/*=false*/) // 4J added soundOnly param
{
	if( soundOnly) return false;

	int data = level->getData(x, y, z);
	int delay = (data & DELAY_MASK) >> DELAY_SHIFT;
	delay = ((delay + 1) << DELAY_SHIFT) & DELAY_MASK;

	level->setData(x, y, z, delay | (data & DIRECTION_MASK));
	return true;
}

bool DiodeTile::isSignalSource()
{
	return true;
}

void DiodeTile::setPlacedBy(Level *level, int x, int y, int z, shared_ptr<Mob> by)
{
	int dir = (((Mth::floor(by->yRot * 4 / (360) + 0.5)) & 3) + 2) % 4;
	level->setData(x, y, z, dir);

	bool sourceOn = getSourceSignal(level, x, y, z, dir);
	if (sourceOn)
	{
		level->addToTickNextTick(x, y, z, id, 1);
	}
}

void DiodeTile::onPlace(Level *level, int x, int y, int z)
{
	level->updateNeighborsAt(x + 1, y, z, id);
	level->updateNeighborsAt(x - 1, y, z, id);
	level->updateNeighborsAt(x, y, z + 1, id);
	level->updateNeighborsAt(x, y, z - 1, id);
	level->updateNeighborsAt(x, y - 1, z, id);
	level->updateNeighborsAt(x, y + 1, z, id);
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

int DiodeTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return Item::diode->id;
}

void DiodeTile::animateTick(Level *level, int xt, int yt, int zt, Random *random)
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
			zo = DiodeTile::DELAY_RENDER_OFFSETS[delay];
			break;
		case Direction::NORTH:
			zo = -DiodeTile::DELAY_RENDER_OFFSETS[delay];
			break;
		case Direction::EAST:
			xo = DiodeTile::DELAY_RENDER_OFFSETS[delay];
			break;
		case Direction::WEST:
			xo = -DiodeTile::DELAY_RENDER_OFFSETS[delay];
			break;
		}
	}
	level->addParticle(eParticleType_reddust, x + xo, y, z + zo, 0, 0, 0);

}

int DiodeTile::cloneTileId(Level *level, int x, int y, int z)
{
	return Item::diode_Id;
}
