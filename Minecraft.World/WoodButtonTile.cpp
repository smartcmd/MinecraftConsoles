#include "stdafx.h"
#include "net.minecraft.h"
#include "WoodButtonTile.h"

WoodButtonTile::WoodButtonTile(int id) : ButtonTile(id, true)
{
}

Icon *WoodButtonTile::getTexture(int face, int data)
{
	return Tile::wood->getTexture(Facing::UP);
}