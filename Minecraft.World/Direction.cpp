#include "stdafx.h"
#include "Direction.h"
#include "Facing.h"

const int Direction::STEP_X[] =
{
	0, -1, 0, 1
};

const int Direction::STEP_Z[] =
{
	1, 0, -1, 0
};

const wstring Direction::NAMES[] = {L"SOUTH", L"WEST", L"NORTH", L"EAST" };

// for [direction] it gives [tile-face]
int Direction::DIRECTION_FACING[4] =
{
	Facing::SOUTH, Facing::WEST, Facing::NORTH, Facing::EAST
};

// for [facing] it gives [direction]
int  Direction::FACING_DIRECTION[] =
{
	UNDEFINED, UNDEFINED, NORTH, SOUTH, WEST, EAST
};

int Direction::DIRECTION_OPPOSITE[4] =
{
	NORTH, EAST, SOUTH, WEST
};

// for [direction] it gives [90 degrees clockwise direction]
int Direction::DIRECTION_CLOCKWISE[] =
{
	WEST, NORTH, EAST, SOUTH
};

// for [direction] it gives [90 degrees counter clockwise direction]
int Direction::DIRECTION_COUNTER_CLOCKWISE[] =
{
	EAST, SOUTH, WEST, NORTH
};

int Direction::RELATIVE_DIRECTION_FACING[4][6] =
{
	// south
	{
		Facing::UP, Facing::DOWN, Facing::SOUTH, Facing::NORTH, Facing::EAST, Facing::WEST
	},
	// west
	{
		Facing::UP, Facing::DOWN, Facing::EAST, Facing::WEST, Facing::NORTH, Facing::SOUTH
		},
		// north
		{
			Facing::UP, Facing::DOWN, Facing::NORTH, Facing::SOUTH, Facing::WEST, Facing::EAST
		},
		// east
		{
			Facing::UP, Facing::DOWN, Facing::WEST, Facing::EAST, Facing::SOUTH, Facing::NORTH
			}
};

int Direction::getDirection(double xd, double zd)
{
	if (Mth::abs(static_cast<float>(xd)) > Mth::abs(static_cast<float>(zd)))
	{
		if (xd > 0)
		{
			return WEST;
		}
		else
		{
			return EAST;
		}
	}
	else
	{
		if (zd > 0)
		{
			return NORTH;
		}
		else
		{
			return SOUTH;
		}
	}
}

int Direction::getDirection(int x0, int z0, int x1, int z1)
{
	int xd = x0 - x1;
	int zd = z0 - z1;

	return getDirection(xd, zd);
}