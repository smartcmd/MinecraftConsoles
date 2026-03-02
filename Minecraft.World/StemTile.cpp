#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.h"
#include "..\Minecraft.Client\Minecraft.h"
#include "..\Minecraft.Client\Common\Colours\ColourTable.h"
#include "StemTile.h"

const wstring StemTile::TEXTURE_ANGLED = L"stem_bent";

StemTile::StemTile(int id, Tile *fruit) : Bush(id)
{
    this->fruit = fruit;

    setTicking(true);
    float ss = 0.125f;
    this->setShape(0.5f - ss, 0, 0.5f - ss, 0.5f + ss, 0.25f, 0.5f + ss);

	iconAngled = NULL;
}

bool StemTile::mayPlaceOn(int tile)
{
	return tile == Tile::farmland_Id;
}

void StemTile::tick(Level *level, int x, int y, int z, Random *random)
{
    Tile::tick(level, x, y, z, random);
    if (level->getRawBrightness(x, y + 1, z) >= Level::MAX_BRIGHTNESS - 6)
	{

        float growthSpeed = getGrowthSpeed(level, x, y, z);

		// 4J Stu - Brought forward change from 1.2.3 to make fruit more likely to grow
        if (random->nextInt((int) (25 / growthSpeed) + 1) == 0)
		{
            int age = level->getData(x, y, z);
            if (age < 7)
			{
                age++;
                level->setData(x, y, z, age);
            }
			else
			{
                if (level->getTile(x - 1, y, z) == fruit->id) return;
                if (level->getTile(x + 1, y, z) == fruit->id) return;
                if (level->getTile(x, y, z - 1) == fruit->id) return;
                if (level->getTile(x, y, z + 1) == fruit->id) return;

                int dir = random->nextInt(4);
                int xx = x;
                int zz = z;
                if (dir == 0) xx--;
                if (dir == 1) xx++;
                if (dir == 2) zz--;
                if (dir == 3) zz++;
				// 4J Stu - Brought forward change from 1.2.3 to not require farmland to grow fruits
				int below = level->getTile(xx, y - 1, zz);
				if (level->getTile(xx, y, zz) == 0 && (below == Tile::farmland_Id || below == Tile::dirt_Id || below == Tile::grass_Id))
				{
                    level->setTile(xx, y, zz, fruit->id);
                }

            }
        }
    }

}

void StemTile::growCropsToMax(Level *level, int x, int y, int z)
{
	level->setData(x, y, z, 7);
}

float StemTile::getGrowthSpeed(Level *level, int x, int y, int z)
{
    float speed = 1;

    int n = level->getTile(x, y, z - 1);
    int s = level->getTile(x, y, z + 1);
    int w = level->getTile(x - 1, y, z);
    int e = level->getTile(x + 1, y, z);

    int d0 = level->getTile(x - 1, y, z - 1);
    int d1 = level->getTile(x + 1, y, z - 1);
    int d2 = level->getTile(x + 1, y, z + 1);
    int d3 = level->getTile(x - 1, y, z + 1);

    bool horizontal = w == this->id || e == this->id;
    bool vertical = n == this->id || s == this->id;
    bool diagonal = d0 == this->id || d1 == this->id || d2 == this->id || d3 == this->id;

    for (int xx = x - 1; xx <= x + 1; xx++)
        for (int zz = z - 1; zz <= z + 1; zz++)
		{
            int t = level->getTile(xx, y - 1, zz);

            float tileSpeed = 0;
            if (t == Tile::farmland_Id)
			{
                tileSpeed = 1;
                if (level->getData(xx, y - 1, zz) > 0) tileSpeed = 3;
            }

            if (xx != x || zz != z) tileSpeed /= 4;

            speed += tileSpeed;
        }

    if (diagonal || (horizontal && vertical)) speed /= 2;

    return speed;
}

int StemTile::getColor(int data)
{
	//int r = data * 32;
	//int g = 255 - data * 8;
	//int b = data * 4;
	//return r << 16 | g << 8 | b;

	int colour = 0;

	unsigned int minColour = Minecraft::GetInstance()->getColourTable()->getColor( eMinecraftColour_Tile_StemMin );
	unsigned int maxColour = Minecraft::GetInstance()->getColourTable()->getColor( eMinecraftColour_Tile_StemMax );

	byte redComponent = ((minColour>>16)&0xFF) + (( (maxColour>>16)&0xFF - (minColour>>16)&0xFF)*( data/7.0f));
	byte greenComponent = ((minColour>>8)&0xFF) + (( (maxColour>>8)&0xFF - (minColour>>8)&0xFF)*( data/7.0f));
	byte blueComponent = ((minColour)&0xFF) + (( (maxColour)&0xFF - (minColour)&0xFF)*( data/7.0f));

	colour = redComponent<<16 | greenComponent<<8 | blueComponent;
	return colour;
}

int StemTile::getColor(LevelSource *level, int x, int y, int z)
{
	return getColor(level->getData(x, y, z));
}

void StemTile::updateDefaultShape()
{
    float ss = 0.125f;
    this->setShape(0.5f - ss, 0, 0.5f - ss, 0.5f + ss, 0.25f, 0.5f + ss);
}

void StemTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity) // 4J added forceData, forceEntity param
{
	ThreadStorage *tls = (ThreadStorage *)TlsGetValue(Tile::tlsIdxShape);
    tls->yy1 = (level->getData(x, y, z) * 2 + 2) / 16.0f;
    float ss = 0.125f;
    this->setShape(0.5f - ss, 0, 0.5f - ss, 0.5f + ss, (float) tls->yy1, 0.5f + ss);
}

int StemTile::getRenderShape()
{
	return Tile::SHAPE_STEM;
}

int StemTile::getConnectDir(LevelSource *level, int x, int y, int z)
	{
	int d = level->getData(x, y, z);
	if (d < 7) return -1;
	if (level->getTile(x - 1, y, z) == fruit->id) return 0;
	if (level->getTile(x + 1, y, z) == fruit->id) return 1;
	if (level->getTile(x, y, z - 1) == fruit->id) return 2;
	if (level->getTile(x, y, z + 1) == fruit->id) return 3;
	return -1;
}

/**
    * Using this method instead of destroy() to determine if seeds should be
    * dropped
    */
void StemTile::spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonus)
{
    Tile::spawnResources(level, x, y, z, data, odds, playerBonus);

    if (level->isClientSide)
	{
        return;
    }

    Item *seed = NULL;
    if (fruit == Tile::pumpkin) seed = Item::seeds_pumpkin;
    if (fruit == Tile::melon) seed = Item::seeds_melon;
    for (int i = 0; i < 3; i++)
	{
        if (level->random->nextInt(5 * 3) > data) continue;
        float s = 0.7f;
        float xo = level->random->nextFloat() * s + (1 - s) * 0.5f;
        float yo = level->random->nextFloat() * s + (1 - s) * 0.5f;
        float zo = level->random->nextFloat() * s + (1 - s) * 0.5f;
        shared_ptr<ItemEntity> item = shared_ptr<ItemEntity>(new ItemEntity(level, x + xo, y + yo, z + zo, shared_ptr<ItemInstance>(new ItemInstance(seed))));
        item->throwTime = 10;
        level->addEntity(item);
    }
}

int StemTile::getResource(int data, Random *random, int playerBonusLevel)
{
    return -1;
}

int StemTile::getResourceCount(Random *random)
{
	return 1;
}

int StemTile::cloneTileId(Level *level, int x, int y, int z)
{
	if (fruit == Tile::pumpkin)
	{
		return Item::seeds_pumpkin_Id;
	}
	else if (fruit == Tile::melon)
	{
		return Item::seeds_melon_Id;
	}

	return 0;
}

void StemTile::registerIcons(IconRegister *iconRegister)
{
	icon = iconRegister->registerIcon(L"stem_straight");
	iconAngled = iconRegister->registerIcon(TEXTURE_ANGLED);
}

Icon *StemTile::getAngledTexture()
{
	return iconAngled;
}
