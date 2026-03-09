#include "stdafx.h"
#include "MycelTile.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.h"
#include "net.minecraft.world.h"

MycelTile::MycelTile(int id) : Tile(id, Material::grass)
{
	iconTop = nullptr;
	iconSnowSide = nullptr;
	setTicking(true);
}

Icon *MycelTile::getTexture(int face, int data)
{
	if (face == Facing::UP) return iconTop;
	if (face == Facing::DOWN) return Tile::dirt->getTexture(face);
	return icon;
}

Icon *MycelTile::getTexture(LevelSource *level, int x, int y, int z, int face)
{
	if (face == Facing::UP) return iconTop;
	if (face == Facing::DOWN) return Tile::dirt->getTexture(face);
	Material *above = level->getMaterial(x, y + 1, z);
	if (above == Material::topSnow || above == Material::snow) return iconSnowSide;
	else return icon;
}

void MycelTile::registerIcons(IconRegister *iconRegister)
{
	icon = iconRegister->registerIcon(L"mycel_side");
	iconTop = iconRegister->registerIcon(L"mycel_top");
	iconSnowSide = iconRegister->registerIcon(L"snow_side");
}

void MycelTile::tick(Level *level, int x, int y, int z, Random *random)
{
	if (level->isClientSide) return;

	if (level->getRawBrightness(x, y + 1, z) < MIN_BRIGHTNESS && Tile::lightBlock[level->getTile(x, y + 1, z)] > 2)
	{
		level->setTileAndUpdate(x, y, z, Tile::dirt_Id);
	}
	else
	{
		if (level->getRawBrightness(x, y + 1, z) >= Level::MAX_BRIGHTNESS - 6)
		{
			for (int i=0; i<4; i++)
			{
				int xt = x + random->nextInt(3) - 1;
				int yt = y + random->nextInt(5) - 3;
				int zt = z + random->nextInt(3) - 1;
				int above = level->getTile(xt, yt + 1, zt);
				if (level->getTile(xt, yt, zt) == Tile::dirt_Id && level->getRawBrightness(xt, yt + 1, zt) >= MIN_BRIGHTNESS && Tile::lightBlock[above] <= 2)
				{
					level->setTileAndUpdate(xt, yt, zt, id);
				}
			}
		}
	}
}

void MycelTile::animateTick(Level *level, int x, int y, int z, Random *random)
{
	Tile::animateTick(level, x, y, z, random);
	if (random->nextInt(10) == 0) level->addParticle(eParticleType_townaura, x + random->nextFloat(), y + 1.1f, z + random->nextFloat(), 0, 0, 0);

}

int MycelTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return Tile::dirt->getResource(0, random, playerBonusLevel);
}