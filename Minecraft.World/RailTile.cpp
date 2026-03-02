#include "stdafx.h"
#include "net.minecraft.world.h"
#include "RailTile.h"

RailTile::RailTile(int id) : BaseRailTile(id, false)
{
}

Icon *RailTile::getTexture(int face, int data)
{
	if (data >= 6)
	{
		return iconTurn;
	}
	else
	{
		return icon;
	}
}

void RailTile::registerIcons(IconRegister *iconRegister)
{
	BaseRailTile::registerIcons(iconRegister);
	iconTurn = iconRegister->registerIcon(getIconName() + L"_turned");
}

void RailTile::updateState(Level *level, int x, int y, int z, int data, int dir, int type)
{
	if (type > 0 && Tile::tiles[type]->isSignalSource())
	{
		if (Rail(level, x, y, z).countPotentialConnections() == 3)
		{
			updateDir(level, x, y, z, false);
		}
	}
}