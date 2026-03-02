#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.h"
#include "DetectorRailTile.h"
#include "net.minecraft.h"

DetectorRailTile::DetectorRailTile(int id) : RailTile(id, true)
{
	setTicking(true);
	icons = NULL;
}

int DetectorRailTile::getTickDelay()
{
	return 20;
}

bool DetectorRailTile::isSignalSource()
{
	return true;
}

void DetectorRailTile::entityInside(Level *level, int x, int y, int z, shared_ptr<Entity> entity)
{
    if (level->isClientSide)
	{
        return;
    }

    int data = level->getData(x, y, z);
    if ((data & RAIL_DATA_BIT) != 0)
	{
        return;
    }

    checkPressed(level, x, y, z, data);
}

void DetectorRailTile::tick(Level *level, int x, int y, int z, Random *random)
{
    if (level->isClientSide) return;

    int data = level->getData(x, y, z);
    if ((data & RAIL_DATA_BIT) == 0)
	{
        return;
    }

    checkPressed(level, x, y, z, data);
}

bool DetectorRailTile::getSignal(LevelSource *level, int x, int y, int z, int dir)
{
	return (level->getData(x, y, z) & RAIL_DATA_BIT) != 0;
}

bool DetectorRailTile::getDirectSignal(Level *level, int x, int y, int z, int facing)
{
    if ((level->getData(x, y, z) & RAIL_DATA_BIT) == 0) return false;
    return (facing == Facing::UP);
}

void DetectorRailTile::checkPressed(Level *level, int x, int y, int z, int currentData)
{
    bool wasPressed = (currentData & RAIL_DATA_BIT) != 0;
    bool shouldBePressed = false;

    float b = 2 / 16.0f;
	vector<shared_ptr<Entity> > *entities = level->getEntitiesOfClass(typeid(Minecart), AABB::newTemp(x + b, y, z + b, x + 1 - b, y + 1 - b, z + 1 - b));
    if (!entities->empty())
	{
        shouldBePressed = true;
    }

    if (shouldBePressed && !wasPressed)
	{
        level->setData(x, y, z, currentData | RAIL_DATA_BIT);
        level->updateNeighborsAt(x, y, z, id);
        level->updateNeighborsAt(x, y - 1, z, id);
        level->setTilesDirty(x, y, z, x, y, z);
    }
    if (!shouldBePressed && wasPressed)
	{
        level->setData(x, y, z, currentData & RAIL_DIRECTION_MASK);
        level->updateNeighborsAt(x, y, z, id);
        level->updateNeighborsAt(x, y - 1, z, id);
        level->setTilesDirty(x, y, z, x, y, z);
    }

    if (shouldBePressed)
	{
        level->addToTickNextTick(x, y, z, id, getTickDelay());
    }

	delete entities;
}

void DetectorRailTile::registerIcons(IconRegister *iconRegister)
{
	icons = new Icon*[2];
	icons[0] = iconRegister->registerIcon(L"detectorRail");
	icons[1] = iconRegister->registerIcon(L"detectorRail_on");
}

Icon *DetectorRailTile::getTexture(int face, int data)
{
	if ((data & RAIL_DATA_BIT) != 0)
	{
		return icons[1];
	}
	return icons[0];
}