#include "stdafx.h"
#include "net.minecraft.world.h"
#include "HugeMushroomTile.h"

const wstring HugeMushroomTile::TEXTURE_STEM = L"skin_stem";
const wstring HugeMushroomTile::TEXTURE_INSIDE = L"inside";
const wstring HugeMushroomTile::TEXTURE_TYPE[] = {L"skin_brown", L"skin_red"};

HugeMushroomTile::HugeMushroomTile(int id, Material *material, int type) : Tile(id, material)
{
	this->type = type;
	icons = nullptr;
	iconStem = nullptr;
	iconInside = nullptr;
}

Icon *HugeMushroomTile::getTexture(int face, int data)
{
	// 123
	// 456 10
	// 789
	if (data == 10 && face > 1) return iconStem;
	if (data >= 1 && data <= 9 && face == 1) return icons[type];
	if (data >= 1 && data <= 3 && face == 2) return icons[type];
	if (data >= 7 && data <= 9 && face == 3) return icons[type];

	if ((data == 1 || data == 4 || data == 7) && face == 4) return icons[type];
	if ((data == 3 || data == 6 || data == 9) && face == 5) return icons[type];

	// two special cases requested by rhodox (painterly pack)
	if (data == 14)
	{
		return icons[type];
	}
	if (data == 15)
	{
		return iconStem;
	}

	return iconInside;
}

int HugeMushroomTile::getResourceCount(Random *random)
{
	int count = random->nextInt(10) - 7;
	if (count < 0) count = 0;
	return count;
}

int HugeMushroomTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return Tile::mushroom_brown_Id + type;
}

int HugeMushroomTile::cloneTileId(Level *level, int x, int y, int z)
{
	return Tile::mushroom_brown_Id + type;
}

void HugeMushroomTile::registerIcons(IconRegister *iconRegister)
{
	icons = new Icon*[HUGE_MUSHROOM_TEXTURE_COUNT];

	for (int i = 0; i < HUGE_MUSHROOM_TEXTURE_COUNT; i++)
	{
		icons[i] = iconRegister->registerIcon(getIconName() + L"_" + TEXTURE_TYPE[i]);
	}

	iconInside = iconRegister->registerIcon(getIconName() + L"_" + TEXTURE_INSIDE);
	iconStem = iconRegister->registerIcon(getIconName() + L"_" + TEXTURE_STEM);
}