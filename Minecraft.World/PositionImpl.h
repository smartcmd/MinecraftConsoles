#pragma once

#include "Position.h"

class PositionImpl : public Position
{
protected:
	double x;
	double y;
	double z;

public:
	PositionImpl(double x, double y, double z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	double getX()
	{
		return x;
	}

	double getY()
	{
		return y;
	}

	double getZ()
	{
		return z;
	}
};