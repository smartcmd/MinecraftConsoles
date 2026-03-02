#include "stdafx.h"
#include "net.minecraft.world.inventory.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.redstone.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.h"
#include "DetectorRailTile.h"
#include "net.minecraft.h"

DetectorRailTile::DetectorRailTile(int id) : BaseRailTile(id, true)
{
	setTicking(true);
	icons = NULL;
}

int DetectorRailTile::getTickDelay(Level *level)
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

int DetectorRailTile::getSignal(LevelSource *level, int x, int y, int z, int dir)
{
	return (level->getData(x, y, z) & RAIL_DATA_BIT) != 0 ? Redstone::SIGNAL_MAX : Redstone::SIGNAL_NONE;
}

int DetectorRailTile::getDirectSignal(LevelSource *level, int x, int y, int z, int facing)
{
	if ((level->getData(x, y, z) & RAIL_DATA_BIT) == 0) return Redstone::SIGNAL_NONE;
	return (facing == Facing::UP) ? Redstone::SIGNAL_MAX : Redstone::SIGNAL_NONE;
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
		level->setData(x, y, z, currentData | RAIL_DATA_BIT, Tile::UPDATE_ALL);
		level->updateNeighborsAt(x, y, z, id);
		level->updateNeighborsAt(x, y - 1, z, id);
		level->setTilesDirty(x, y, z, x, y, z);
	}
	if (!shouldBePressed && wasPressed)
	{
		level->setData(x, y, z, currentData & RAIL_DIRECTION_MASK, Tile::UPDATE_ALL);
		level->updateNeighborsAt(x, y, z, id);
		level->updateNeighborsAt(x, y - 1, z, id);
		level->setTilesDirty(x, y, z, x, y, z);
	}

	if (shouldBePressed)
	{
		level->addToTickNextTick(x, y, z, id, getTickDelay(level));
	}

	level->updateNeighbourForOutputSignal(x, y, z, id);

	delete entities;
}

void DetectorRailTile::onPlace(Level *level, int x, int y, int z)
{
	BaseRailTile::onPlace(level, x, y, z);
	checkPressed(level, x, y, z, level->getData(x, y, z));
}

bool DetectorRailTile::hasAnalogOutputSignal()
{
	return true;
}

int DetectorRailTile::getAnalogOutputSignal(Level *level, int x, int y, int z, int dir)
{
	if ((level->getData(x, y, z) & RAIL_DATA_BIT) > 0)
	{
		float b = 2 / 16.0f;
		vector<shared_ptr<Entity> > *entities = level->getEntitiesOfClass(typeid(Minecart), AABB::newTemp(x + b, y, z + b, x + 1 - b, y + 1 - b, z + 1 - b), EntitySelector::CONTAINER_ENTITY_SELECTOR);

		if (entities->size() > 0)
		{
			shared_ptr<Entity> out = entities->at(0);
			delete entities;
			return AbstractContainerMenu::getRedstoneSignalFromContainer(dynamic_pointer_cast<Container>(out));
		}
	}

	return Redstone::SIGNAL_NONE;
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