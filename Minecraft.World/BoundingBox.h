#pragma once

#include "ArrayWithLength.h"

class BoundingBox
{
public:
	int x0, y0, z0, x1, y1, z1;

	BoundingBox();
	BoundingBox(intArray sourceData);
	static BoundingBox *getUnknownBox();
	static BoundingBox *orientBox(int footX, int footY, int footZ, int offX, int offY, int offZ, int width, int height, int depth, int orientation);
	BoundingBox(BoundingBox *other);
	BoundingBox(int x0, int y0, int z0, int x1, int y1, int z1);
	BoundingBox(int x0, int z0, int x1, int z1);
	bool intersects(BoundingBox *other);
	bool intersects(int x0, int y0, int z0, int x1, int y1, int z1);

	bool intersects(int x0, int z0, int x1, int z1);
	void expand(BoundingBox *other);
	BoundingBox *getIntersection(BoundingBox *other);
	void move(int dx, int dy, int dz);
	bool isInside(int x, int y, int z);

	int getXSpan();
	int getYSpan();
	int getZSpan();
	int getXCenter();
	int getYCenter();
	int getZCenter();

	wstring toString();
	IntArrayTag *createTag(const wstring &name);
};