#pragma once

class Direction
{
public:
	static const int UNDEFINED = -1;
	static const int SOUTH = 0;
	static const int WEST = 1;
	static const int NORTH = 2;
	static const int EAST = 3;

	static const int STEP_X[];
	static const int STEP_Z[];

	static const wstring NAMES[];;

	// for [direction] it gives [tile-face]
	static int DIRECTION_FACING[];

	// for [facing] it gives [direction]
	static int FACING_DIRECTION[];

	// for [direction] it gives [opposite direction]
	static int DIRECTION_OPPOSITE[];

	// for [direction] it gives [90 degrees clockwise direction]
	static int DIRECTION_CLOCKWISE[];

	// for [direction] it gives [90 degrees counter-clockwise direction]
	static int DIRECTION_COUNTER_CLOCKWISE[];

	// for [direction][world-facing] it gives [tile-facing]
	static int RELATIVE_DIRECTION_FACING[4][6];

	static int getDirection(double xd, double zd);
	static int getDirection(int x0, int z0, int x1, int z1);
};