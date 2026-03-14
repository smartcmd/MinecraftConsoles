#include "stdafx.h"
#include "../Minecraft.Client/Minecraft.h"
#include "GrassTile.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.biome.h"
#include "net.minecraft.h"
#include "net.minecraft.world.h"

// AP - included for PSVita Alpha cut out optimisation
#include "IntBuffer.h"
#include "../Minecraft.Client/Tesselator.h"

GrassTile::GrassTile(int id) : Tile(id, Material::grass)
{
	iconTop = nullptr;
	iconSnowSide = nullptr;
	iconSideOverlay = nullptr;

	setTicking(true);
}

Icon *GrassTile::getTexture(int face, int data)
{
	if (face == Facing::UP) return iconTop;
	if (face == Facing::DOWN) return Tile::dirt->getTexture(face);
	return icon;
}

Icon *GrassTile::getTexture(LevelSource *level, int x, int y, int z, int face)
{
	if (face == Facing::UP) return iconTop;
	if (face == Facing::DOWN) return Tile::dirt->getTexture(face);
	Material *above = level->getMaterial(x, y + 1, z);
	if (above == Material::topSnow || above == Material::snow) return iconSnowSide;
	else return icon;
}

void GrassTile::registerIcons(IconRegister *iconRegister)
{
	icon = iconRegister->registerIcon(L"grass_side");
	iconTop = iconRegister->registerIcon(L"grass_top");
	iconSnowSide = iconRegister->registerIcon(L"snow_side");
	iconSideOverlay = iconRegister->registerIcon(L"grass_side_overlay");
}

int GrassTile::getColor() const
{
	// 4J Replaced
	//double temp = 0.5;
	//double rain = 1.0;

	//return GrassColor::get(temp, rain);

	return Minecraft::GetInstance()->getColourTable()->getColor( eMinecraftColour_Grass_Common );
}

int GrassTile::getColor(int auxData)
{
	return getColor();
}

int GrassTile::getColor(LevelSource *level, int x, int y, int z)
{
	return getColor( level, x, y, z, level->getData( x, y, z ) );
}

// 4J - changed interface to have data passed in, and put existing interface as wrapper above
int GrassTile::getColor(LevelSource *level, int x, int y, int z, int data)
{
	//return level->getBiomeSource()->getBiome(x, z)->getGrassColor(level, x, y, z);

	int totalRed = 0;
	int totalGreen = 0;
	int totalBlue = 0;

	for (int oz = -1; oz <= 1; oz++)
	{
		for (int ox = -1; ox <= 1; ox++)
		{
			int grassColor = level->getBiome(x + ox, z + oz)->getGrassColor();

			totalRed += (grassColor & 0xff0000) >> 16;
			totalGreen += (grassColor & 0xff00) >> 8;
			totalBlue += (grassColor & 0xff);
		}
	}

	return (((totalRed / 9) & 0xFF) << 16) | (((totalGreen / 9) & 0xFF) << 8) | (((totalBlue / 9) & 0xFF));
}

void GrassTile::tick(Level *level, int x, int y, int z, Random *random)
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
			for (int i = 0; i < 4; i++)
			{
				int xt = x + random->nextInt(3) - 1;
				int yt = y + random->nextInt(5) - 3;
				int zt = z + random->nextInt(3) - 1;
				int above = level->getTile(xt, yt + 1, zt);
				if (level->getTile(xt, yt, zt) == Tile::dirt_Id && level->getRawBrightness(xt, yt + 1, zt) >= MIN_BRIGHTNESS && Tile::lightBlock[above] <= 2)
				{
					level->setTileAndUpdate(xt, yt, zt, Tile::grass_Id);
				}
			}
		}
	}
}

int GrassTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return Tile::dirt->getResource(0, random, playerBonusLevel);
}

Icon *GrassTile::getSideTextureOverlay()
{
#ifdef __PSVITA__
	// AP - alpha cut out is expensive on vita. Because of the way grass sides are treated as special case we need to set the alpha flag here
	// this would normally happen in TileRenderer::getTextureOrMissing
	Tesselator* t = Tesselator::getInstance();
	t->setAlphaCutOut( true );
#endif

	return Tile::grass->iconSideOverlay;
}

bool GrassTile::shouldTileTick(Level *level, int x,int y,int z)
{
	bool should = false;

	if( (level->getRawBrightness(x, y + 1, z) < MIN_BRIGHTNESS && Tile::lightBlock[level->getTile(x, y + 1, z)] > 2) ||
		(level->getRawBrightness(x, y + 1, z) >= Level::MAX_BRIGHTNESS - 6) )
	{
		should = true;
	}

	return should;
}
