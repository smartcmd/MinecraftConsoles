#include "stdafx.h"
#include "net.minecraft.world.h"
#include "net.minecraft.world.item.h"
#include "CarrotTile.h"

CarrotTile::CarrotTile(int id) : CropTile(id)
{
}

Icon *CarrotTile::getTexture(int face, int data)
{
	if (data < 7)
	{
		if (data == 6)
		{
			data = 5;
		}
		return icons[data >> 1];
	}
	else
	{
		return icons[3];
	}
}

int CarrotTile::getBaseSeedId()
{
	return Item::carrots_Id;
}

int CarrotTile::getBasePlantId()
{
	return Item::carrots_Id;
}

void CarrotTile::registerIcons(IconRegister *iconRegister)
{
	for (int i = 0; i < 4; i++)
	{
		icons[i] = iconRegister->registerIcon(getIconName() + L"_stage_" + std::to_wstring(i));
	}
}