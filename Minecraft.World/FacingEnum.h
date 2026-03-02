#pragma once

class FacingEnum
{
public:
	static FacingEnum *DOWN;
	static FacingEnum *UP;
	static FacingEnum *NORTH;
	static FacingEnum *SOUTH;
	static FacingEnum *EAST;
	static FacingEnum *WEST;

private:
	const int dataValue;
	const int oppositeIndex;
	const int stepX;
	const int stepY;
	const int stepZ;

	static FacingEnum *BY_DATA[6];

	FacingEnum(int dataValue, int oppositeIndex, int stepX, int stepY, int stepZ);

public:
	int getDataValue();
	FacingEnum *getOpposite();
	int getStepX();
	int getStepY();
	int getStepZ();
	static FacingEnum *fromData(int data);
};