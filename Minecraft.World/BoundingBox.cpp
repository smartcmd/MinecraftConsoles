#include "stdafx.h"
#include "BoundingBox.h"
#include "Direction.h"
#include "JavaMath.h"

BoundingBox::BoundingBox()
{
	// 4J added initialisers
	x0 = 0;
	y0 = 0;
	z0 = 0;
	x1 = 0;
	y1 = 0;
	z1 = 0;
}

BoundingBox::BoundingBox(intArray sourceData)
{
	if (sourceData.length == 6)
	{
		x0 = sourceData[0];
		y0 = sourceData[1];
		z0 = sourceData[2];
		x1 = sourceData[3];
		y1 = sourceData[4];
		z1 = sourceData[5];
	}
}

BoundingBox *BoundingBox::getUnknownBox()
{
	return new BoundingBox(INT_MAX, INT_MAX, INT_MAX, INT_MIN, INT_MIN, INT_MIN );
}

BoundingBox *BoundingBox::orientBox(int footX, int footY, int footZ, int offX, int offY, int offZ, int width, int height, int depth, int orientation)
{
	switch (orientation)
	{
	default:
		return new BoundingBox(footX + offX, footY + offY, footZ + offZ, footX + width - 1 + offX, footY + height - 1 + offY, footZ + depth - 1 + offZ);
	case Direction::NORTH:
		// foot is at x0, y0, z1
		return new BoundingBox(footX + offX, footY + offY, footZ - depth + 1 + offZ, footX + width - 1 + offX, footY + height - 1 + offY, footZ + offZ);
	case Direction::SOUTH:
		// foot is at x0, y0, z0
		return new BoundingBox(footX + offX, footY + offY, footZ + offZ, footX + width - 1 + offX, footY + height - 1 + offY, footZ + depth - 1 + offZ);
	case Direction::WEST:
		// foot is at x1, y0, z0, but width and depth are flipped
		return new BoundingBox(footX - depth + 1 + offZ, footY + offY, footZ + offX, footX + offZ, footY + height - 1 + offY, footZ + width - 1 + offX);
	case Direction::EAST:
		// foot is at x0, y0, z0, but width and depth are flipped
		return new BoundingBox(footX + offZ, footY + offY, footZ + offX, footX + depth - 1 + offZ, footY + height - 1 + offY, footZ + width - 1 + offX);
	}
}

BoundingBox::BoundingBox(BoundingBox *other)
{
	x0 = other->x0;
	y0 = other->y0;
	z0 = other->z0;
	x1 = other->x1;
	y1 = other->y1;
	z1 = other->z1;
}

BoundingBox::BoundingBox(int x0, int y0, int z0, int x1, int y1, int z1)
{
	this->x0 = x0;
	this->y0 = y0;
	this->z0 = z0;
	this->x1 = x1;
	this->y1 = y1;
	this->z1 = z1;
}

BoundingBox::BoundingBox(int x0, int z0, int x1, int z1)
{
	this->x0 = x0;
	this->z0 = z0;
	this->x1 = x1;
	this->z1 = z1;

	// the bounding box for this constructor is limited to world size,
	// excluding bedrock level
	y0 = 1;
	y1 = 512;
}

bool BoundingBox::intersects(BoundingBox *other)
{
	return !(this->x1 < other->x0 || this->x0 > other->x1 || this->z1 < other->z0 || this->z0 > other->z1 || this->y1 < other->y0 || this->y0 > other->y1);
}

bool BoundingBox::intersects(int x0, int y0, int z0, int x1, int y1, int z1)
{
	return !(this->x1 < x0 || this->x0 > x1 || this->z1 < z0 || this->z0 > z1 || this->y1 < y0 || this->y0 > y1);
}

bool BoundingBox::intersects(int x0, int z0, int x1, int z1)
{
	return !(this->x1 < x0 || this->x0 > x1 || this->z1 < z0 || this->z0 > z1);
}

void BoundingBox::expand(BoundingBox *other)
{
	x0 = Math::_min(x0, other->x0);
	y0 = Math::_min(y0, other->y0);
	z0 = Math::_min(z0, other->z0);
	x1 = Math::_max(x1, other->x1);
	y1 = Math::_max(y1, other->y1);
	z1 = Math::_max(z1, other->z1);
}

BoundingBox *BoundingBox::getIntersection(BoundingBox *other)
{
	if (!intersects(other))
	{
		return NULL;
	}
	BoundingBox *result = new BoundingBox();
	result->x0 = Math::_max(x0, other->x0);
	result->y0 = Math::_max(y0, other->y0);
	result->z0 = Math::_max(z0, other->z0);
	result->x1 = Math::_min(x1, other->x1);
	result->y1 = Math::_min(y1, other->y1);
	result->z1 = Math::_min(z1, other->z1);

	return result;
}

void BoundingBox::move(int dx, int dy, int dz)
{
	x0 += dx;
	y0 += dy;
	z0 += dz;
	x1 += dx;
	y1 += dy;
	z1 += dz;
}

bool BoundingBox::isInside(int x, int y, int z)
{
	return (x >= x0 && x <= x1 && z >= z0 && z <= z1 && y >= y0 && y <= y1);
}

int BoundingBox::getXSpan()
{
	return x1 - x0 + 1;
}

int BoundingBox::getYSpan()
{
	return y1 - y0 + 1;
}

int BoundingBox::getZSpan()
{
	return z1 - z0 + 1;
}

int BoundingBox::getXCenter()
{
	return x0 + (x1 - x0 + 1) / 2;
}

int BoundingBox::getYCenter()
{
	return y0 + (y1 - y0 + 1) / 2;
}

int BoundingBox::getZCenter()
{
	return z0 + (z1 - z0 + 1) / 2;
}

wstring BoundingBox::toString()
{
	return L"(" + std::to_wstring(x0) + L", " + std::to_wstring(y0) + L", " + std::to_wstring(z0) + L"; " + std::to_wstring(x1) + L", " + std::to_wstring(y1) + L", " + std::to_wstring(z1) + L")";
}

IntArrayTag *BoundingBox::createTag(const wstring &name)
{
	// 4J-JEV: If somebody knows a better way to do this, please tell me.
	int *data = new int[6]();
	data[0] = x0; data[1] = y0; data[2] = z0;
	data[3] = x1; data[4] = y1; data[5] = z1;

	return new IntArrayTag( name, intArray(data,6) );
}