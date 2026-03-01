#include "stdafx.h"

#include "DirectionalTile.h"

DirectionalTile::DirectionalTile(int id, Material *material, bool isSolidRender) : Tile(id, material, isSolidRender)
{
}

int DirectionalTile::getDirection(int data)
{
	return data & DIRECTION_MASK;
}