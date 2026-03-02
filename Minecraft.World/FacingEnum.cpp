#include "stdafx.h"

#include "FacingEnum.h"

FacingEnum *FacingEnum::DOWN = new FacingEnum(0, 1, 0, -1, 0);
FacingEnum *FacingEnum::UP = new FacingEnum(1, 0, 0, 1, 0);
FacingEnum *FacingEnum::NORTH = new FacingEnum(2, 3, 0, 0, -1);
FacingEnum *FacingEnum::SOUTH = new FacingEnum(3, 2, 0, 0, 1);
FacingEnum *FacingEnum::EAST = new FacingEnum(4, 5, -1, 0, 0);
FacingEnum *FacingEnum::WEST = new FacingEnum(5, 4, 1, 0, 0);

FacingEnum *FacingEnum::BY_DATA[6] = {FacingEnum::DOWN,FacingEnum::UP,FacingEnum::NORTH,FacingEnum::SOUTH,FacingEnum::EAST,FacingEnum::WEST};

FacingEnum::FacingEnum(int dataValue, int oppositeIndex, int stepX, int stepY, int stepZ)
	: dataValue(dataValue), oppositeIndex(oppositeIndex), stepX(stepX), stepY(stepY), stepZ(stepZ)
{
}

int FacingEnum::getDataValue()
{
	return dataValue;
}

FacingEnum *FacingEnum::getOpposite()
{
	return BY_DATA[oppositeIndex];
}

int FacingEnum::getStepX()
{
	return stepX;
}

int FacingEnum::getStepY()
{
	return stepY;
}

int FacingEnum::getStepZ()
{
	return stepZ;
}

FacingEnum *FacingEnum::fromData(int data)
{
	return BY_DATA[data % 6];
}