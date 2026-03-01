#pragma once

#include "Tile.h"

class DirectionalTile : public Tile
{
public:
	static const int DIRECTION_MASK = 0x3;
	static const int DIRECTION_INV_MASK = 0xC;

protected:
	DirectionalTile(int id, Material *material, bool isSolidRender);

public:
	static int getDirection(int data);
};