#include "stdafx.h"

#include "DoorInfo.h"

DoorInfo::DoorInfo(int x, int y, int z, int insideDx, int insideDy, int timeStamp) : x(x), y(y), z(z), insideDx(insideDx), insideDz(insideDy)
{
	removed = false;
	bookings = 0;

	this->timeStamp = timeStamp;
}

int DoorInfo::distanceTo(int x2, int y2, int z2)
{
	return static_cast<int>(sqrt((float)distanceToSqr(x2, y2, z2)));
}

int DoorInfo::distanceToSqr(int x2, int y2, int z2)
{
	int dx = x2 - x;
	int dy = y2 - y;
	int dz = z2 - z;
	return dx * dx + dy * dy + dz * dz;
}

int DoorInfo::distanceToInsideSqr(int x2, int y2, int z2)
{
	int dx = x2 - x - insideDx;
	int dy = y2 - y;
	int dz = z2 - z - insideDz;
	return dx * dx + dy * dy + dz * dz;
}

int DoorInfo::getIndoorX()
{
	return x + insideDx;
}

int DoorInfo::getIndoorY()
{
	return y;
}

int DoorInfo::getIndoorZ()
{
	return z + insideDz;
}

bool DoorInfo::isInsideSide(int testX, int testZ)
{
	int vdx = testX - x;
	int vdz = testZ - z;
	return vdx * insideDx + vdz * insideDz >= 0;
}

void DoorInfo::resetBookingCount()
{
	bookings = 0;
}

void DoorInfo::incBookingCount()
{
	++bookings;
}

int DoorInfo::getBookingsCount()
{
	return bookings;
}