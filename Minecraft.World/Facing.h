#pragma once

class Facing
{
public:
	static const int DOWN = 0;
	static const int UP = 1;
	static const int NORTH = 2;
	static const int SOUTH = 3;
	static const int WEST = 4;
	static const int EAST = 5;

	static const int OPPOSITE_FACING[6];
	static const int STEP_X[6];
	static const int STEP_Y[6];
	static const int STEP_Z[6];

	static const wstring NAMES[];
};