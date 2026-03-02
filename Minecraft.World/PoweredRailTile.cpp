#include "stdafx.h"
#include "net.minecraft.world.h"
#include "net.minecraft.world.level.h"
#include "PoweredRailTile.h"

PoweredRailTile::PoweredRailTile(int id) : BaseRailTile(id, true)
{
}

Icon *PoweredRailTile::getTexture(int face, int data)
{
	if ((data & RAIL_DATA_BIT) == 0)
	{
		return icon;
	}
	else
	{
		return iconPowered;
	}
}

void PoweredRailTile::registerIcons(IconRegister *iconRegister)
{
	BaseRailTile::registerIcons(iconRegister);
	iconPowered = iconRegister->registerIcon(getIconName() + L"_powered");
}

bool PoweredRailTile::findPoweredRailSignal(Level *level, int x, int y, int z, int data, bool forward, int searchDepth)
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
		}
		else
		{
			z--;
		}
		break;
	case DIR_FLAT_X:
		if (forward)
		{
			x--;
		}
		else
		{
			x++;
		}
		break;
	case 2:
		if (forward)
		{
			x--;
		}
		else
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
		}
		else
		{
			x++;
		}
		dir = DIR_FLAT_X;
		break;
	case 4:
		if (forward)
		{
			z++;
		}
		else
		{
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
		}
		else
		{
			z--;
		}
		dir = DIR_FLAT_Z;
		break;
	}

	if (isSameRailWithPower(level, x, y, z, forward, searchDepth, dir))
	{
		return true;
	}
	if (checkBelow && isSameRailWithPower(level, x, y - 1, z, forward, searchDepth, dir))
	{
		return true;
	}
	return false;
}

bool PoweredRailTile::isSameRailWithPower(Level *level, int x, int y, int z, bool forward, int searchDepth, int dir)
{
	int tile = level->getTile(x, y, z);

	if (tile == id)
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
				return findPoweredRailSignal(level, x, y, z, tileData, forward, searchDepth + 1);
			}
		}
	}

	return false;
}

void PoweredRailTile::updateState(Level *level, int x, int y, int z, int data, int dir, int type)
{
	bool signal = level->hasNeighborSignal(x, y, z);
	signal = signal || findPoweredRailSignal(level, x, y, z, data, true, 0) || findPoweredRailSignal(level, x, y, z, data, false, 0);

	bool changed = false;
	if (signal && (data & RAIL_DATA_BIT) == 0)
	{
		level->setData(x, y, z, dir | RAIL_DATA_BIT, Tile::UPDATE_ALL);
		changed = true;
	}
	else if (!signal && (data & RAIL_DATA_BIT) != 0)
	{
		level->setData(x, y, z, dir, Tile::UPDATE_ALL);
		changed = true;
	}

	// usually the level only updates neighbors that are in the same
	// y plane as the current tile, but sloped rails may need to
	// update tiles above or below it as well
	if (changed)
	{
		level->updateNeighborsAt(x, y - 1, z, id);
		if (dir == 2 || dir == 3 || dir == 4 || dir == 5)
		{
			level->updateNeighborsAt(x, y + 1, z, id);
		}
	}
}